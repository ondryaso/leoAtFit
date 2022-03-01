using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using IpkEpsilon.Common;
using IpkEpsilon.Server.Sftp;
using IpkEpsilon.Server.Sftp.Abstractions;

namespace IpkEpsilon.Server.Network
{
    /// <summary>
    /// Represents a worker that asynchronously handles communication with a single client.
    /// Instances of this class are created when a client is accepted.
    /// </summary>
    public class ClientHandler : IDisposable
    {
        private Socket _clientSocket;
        private readonly ISftpProviderFactory _sftpProviderFactory;
        private readonly string _id;

        private ISftpProvider _sftp;
        private bool _ended, _started;

        private bool _receivingStream;
        private Stream _currentStream;
        private long _bytesToFetch;
        private ManualResetEventSlim _canExit = new ManualResetEventSlim(false);

        /// <summary>
        /// Creates a new ClientHandler instance that communicates with a client using
        /// the provided <see cref="Socket"/>.
        /// </summary>
        /// <param name="clientSocket">A client socket.</param>
        /// <param name="sftpProviderFactory">A factory object for creating a SFTP protocol handler.</param>
        public ClientHandler(Socket clientSocket, ISftpProviderFactory sftpProviderFactory)
        {
            _clientSocket = clientSocket;
            _sftpProviderFactory = sftpProviderFactory;

            // Create a unique ID and saves it as a base32 string (6 characters shorter than saving the guid in hex)
            // This is a bit of an overkill but why not
            var bytes = Guid.NewGuid().ToByteArray();
            _id = Base32Utils.ToBase32String(bytes);
        }

        /// <summary>
        /// Executes the main client loop. Creates a SFTP protocol handler, asynchronously waits for incoming data,
        /// lets the handler process the incoming message and acts based on the handler's result.
        /// This runs until cancellation is requested on the passed <paramref name="cancellationToken"/>.
        /// </summary>
        /// <param name="cancellationToken">The token to monitor for cancellation requests.</param>
        /// <returns>A Task that represents the asynchronous processing loop.</returns>
        /// <exception cref="InvalidOperationException">Thrown when attempting to execute this method on
        /// an instance that has already started or ended.</exception>
        public async Task Run(CancellationToken cancellationToken)
        {
            if (_ended || _started) throw new InvalidOperationException();
            _started = true;

            if (_clientSocket.RemoteEndPoint is IPEndPoint ipEndpoint)
            {
                Log.Info($"{_id}: Client connected (remote IP: {ipEndpoint.Address})");
            }
            else
            {
                Log.Info($"{_id}: Client connected");
            }

            // Receive buffer
            var buffer = new byte[SocketServer.BufferSize];
            var bufferMem = new Memory<byte>(buffer);
            // SFTP provider instance that handles the protocol
            _sftp = _sftpProviderFactory.CreateProvider();

            if (!await SendHello(cancellationToken)) return;

            // Used for incoming string messages longer than the receive buffer
            // (which are very improbable, but in theory possible)
            var sb = new StringBuilder();
            var readingLongMessage = false;

            while (!cancellationToken.IsCancellationRequested)
            {
                if (_ended) return;

                _canExit.Reset();
                var token = cancellationToken;

                // If we're in the middle of receiving a file, create a new CancellationToken that cancels
                // waiting for new data after 3 seconds. Typically, ReceiveAsync would fail if client disconnected,
                // but just in case.
                if (_receivingStream)
                {
                    var cts = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
                    cts.CancelAfter(SocketServer.ReceiveTimeout);
                    token = cts.Token;
                }

                int recv;

                try
                {
                    recv = await _clientSocket.ReceiveAsync(bufferMem, SocketFlags.None,
                        token);
                }
                catch (SocketException e)
                {
                    Log.Error($"{_id}: Client error, disconnecting ({e.Message})");
                    Exit();
                    return;
                }
                catch (OperationCanceledException)
                {
                    // If the operation was aborted 
                    if (!cancellationToken.IsCancellationRequested)
                    {
                        // This CanceledException was thrown by the timeout token expiring
                        Log.Error($"{_id}: Client transmission timeout, disconnecting");
                    }

                    Exit();
                    return;
                }

                // Receive returns zero when the socket has been gracefully ended
                if (recv == 0)
                {
                    Exit();
                    return;
                }

                try
                {
                    if (_receivingStream)
                    {
                        // File transfers are handled separately
                        await ReceiveStream(buffer, bufferMem, recv, cancellationToken);
                    }
                    else
                    {
                        // Check whether the received data ends with a <NULL> byte
                        if (buffer[recv - 1] == 0)
                        {
                            var msgPart = Encoding.UTF8.GetString(bufferMem.Slice(0, recv - 1).Span);

                            if (readingLongMessage)
                            {
                                sb.Append(msgPart);
                                msgPart = sb.ToString();
                                sb.Clear();
                                readingLongMessage = false;
                            }


                            await ProcessMessage(msgPart, cancellationToken);
                        }
                        else
                        {
                            // If it does not, it means that the message didn't fit into our buffer
                            // Set the long message flag and append what we got to the string builder
                            readingLongMessage = true;
                            var msgPart = Encoding.UTF8.GetString(bufferMem.Slice(0, recv).Span);
                            sb.Append(msgPart);
                        }
                    }
                }
                catch (SocketException e)
                {
                    // Client might disconnect unexpectedly when processing the message
                    Log.Error($"{_id}: Client error, disconnecting ({e.Message})");
                    Exit();
                    return;
                }
                catch (OperationCanceledException)
                {
                    Exit();
                    return;
                }

                _canExit.Set();
            }
        }

        private async Task<bool> SendHello(CancellationToken cancellationToken)
        {
            try
            {
                var msg = _sftp.MakeGreeting(_id);
                await _clientSocket.SendAsync(Encoding.ASCII.GetBytes(msg.ResponseMessage + "\0"),
                    SocketFlags.None, cancellationToken);
            }
            catch (SocketException e)
            {
                Log.Error($"{_id}: Error sending initial message, terminating ({e.Message})");
                Exit();
                return false;
            }
            catch (OperationCanceledException)
            {
                Exit();
                return false;
            }

            return true;
        }

        private async Task ReceiveStream(byte[] buffer, Memory<byte> bufferMem, int receivedTotal,
            CancellationToken cancellationToken)
        {
            await _currentStream.WriteAsync(bufferMem.Slice(0, (int) Math.Min(_bytesToFetch, receivedTotal)),
                cancellationToken);
            _bytesToFetch -= receivedTotal;

            if (_bytesToFetch <= 0)
            {
                _receivingStream = false;
                _sftp.NotifyFileReceived();

                buffer[0] = (byte) '+';
                buffer[1] = 0;
                await _clientSocket.SendAsync(bufferMem.Slice(0, 2), SocketFlags.None, cancellationToken);

                await _currentStream.FlushAsync(cancellationToken);
                await _currentStream.DisposeAsync();
                _currentStream = null;

                if (_bytesToFetch < 0)
                {
                    Log.Warning($"{_id}: Unexpected number of bytes received");
                }
            }
        }

        /// <summary>
        /// Passes a received protocol command to the protocol handler and performs the required response action.
        /// </summary>
        /// <seealso cref="SftpProvider.HandleCommand"/>
        /// <param name="message">The received command.</param>
        /// <param name="cancellationToken">The token to monitor for cancellation requests.</param>
        /// <returns>A Task object representing the asynchronous command processing operation.</returns>
        private async Task ProcessMessage(string message, CancellationToken cancellationToken)
        {
            // Pass command to the protocol handler and get a CommandExecutionResult
            var result = _sftp.HandleCommand(message);

            // The result has two important flags: response mode and next mode.
            // Response mode decides whether a string response or a byte stream should be sent back.

            switch (result.ResponseMode)
            {
                case ResponseMode.SendString:
                    // We could potentially save some allocations by creating and reusing a single big buffer
                    var buffer = new byte[Encoding.UTF8.GetByteCount(result.ResponseMessage) + 1];
                    Encoding.UTF8.GetBytes(result.ResponseMessage, buffer.AsSpan());
                    var memory = buffer.AsMemory();
                    var sent = 0;

                    // Send bytes until we send them all
                    while (sent < buffer.Length && !cancellationToken.IsCancellationRequested)
                    {
                        sent += await _clientSocket.SendAsync(memory.Slice(sent), SocketFlags.None, cancellationToken);
                    }

                    break;
                case ResponseMode.SendStream:
                    _currentStream = result.Stream;
                    try
                    {
                        await SendFromStream(result.Stream, cancellationToken);
                    }
                    finally
                    {
                        await _currentStream.DisposeAsync();
                        _currentStream = null;
                    }

                    break;
                default:
                    throw new ArgumentOutOfRangeException();
            }

            // After this command, we can either exit or wait for another command or save the specified amount
            // of received bytes to a stream – receive and store a file.
            switch (result.NextMode)
            {
                case NextMode.ReceiveBytes:
                    _receivingStream = true;
                    _currentStream = result.Stream;
                    _bytesToFetch = result.BytesToFetch;
                    break;
                case NextMode.ReadCommand:
                    _receivingStream = false;
                    break;
                case NextMode.Exit:
                    Exit();
                    break;
                default:
                    throw new ArgumentOutOfRangeException();
            }
        }

        /// <summary>
        /// Sends the entire content of the specified stream through the client socket.
        /// </summary>
        /// <param name="stream">The stream to read data from.</param>
        /// <param name="cancellationToken">The token to monitor for cancellation requests.</param>
        /// <returns>A Task representing the asynchronous data sending operation.</returns>
        /// <exception cref="InvalidOperationException">Thrown if <paramref name="stream"/> doesn't support reading.</exception>
        private async Task SendFromStream(Stream stream, CancellationToken cancellationToken)
        {
            if (!stream.CanRead)
            {
                throw new InvalidOperationException("Cannot read from the specified stream.");
            }

            var buffer = new byte[SocketServer.BufferSize];
            var mem = new ReadOnlyMemory<byte>(buffer);

            while (!cancellationToken.IsCancellationRequested)
            {
                var read = await stream.ReadAsync(buffer, 0, buffer.Length, cancellationToken);
                if (read == 0) break; // Everything has been sent

                await _clientSocket.SendAsync(mem.Slice(0, read), SocketFlags.None, cancellationToken);
            }
        }


        /// <summary>
        /// Attempts to exit this client and dispose this object in a thread-safe way.
        /// </summary>
        public void WaitForExit()
        {
            if (_ended) return;

            _canExit.Wait(SocketServer.ExitTimeout);
            Exit();

            _canExit.Dispose();
            _canExit = null;
        }

        /// <summary>
        /// Synchronously disconnects.
        /// </summary>
        private void Exit()
        {
            if (_ended) return;

            _canExit.Reset();

            if (_clientSocket.Connected)
            {
                Log.Info($"{_id}: Disconnecting client");
                _clientSocket.Disconnect(false);
            }

            _clientSocket.Dispose();
            _sftp.Dispose();
            _currentStream?.Dispose();
            _currentStream = null;
            _ended = true;
            _canExit.Set();
        }

        public void Dispose()
        {
            var exit = _canExit;
            _canExit = null;
            exit?.Dispose();

            if (_ended) return;

            _clientSocket?.Dispose();
            _clientSocket = null;

            _sftp?.Dispose();
            _sftp = null;

            _currentStream?.Dispose();
            _currentStream = null;

            _ended = true;
        }
    }
}