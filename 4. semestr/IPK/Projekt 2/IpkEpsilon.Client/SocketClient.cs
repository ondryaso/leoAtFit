using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using IpkEpsilon.Common;

namespace IpkEpsilon.Client
{
    /// <summary>
    /// Represents an asynchronous TCP client. It is used as a wrapper around a Socket object,
    /// providing asynchronous methods for connecting and sending or receiving text messages, as
    /// well as byte streams. 
    /// </summary>
    public class SocketClient : IDisposable
    {
        public const int BufferSize = 1400;

        private readonly IPEndPoint _endpoint;

        private Socket _socket;
        private bool _ended;
        private readonly byte[] _recvBuffer;
        private readonly Memory<byte> _recvBufferMem;

        public bool Ended => _ended;

        /// <summary>
        /// Creates a <see cref="SocketClient"/> instance that will connect to the specified endpoint.
        /// </summary>
        /// <param name="endpoint">An IP endpoint to connect to.</param>
        public SocketClient(IPEndPoint endpoint)
        {
            _endpoint = endpoint;

            _recvBuffer = new byte[BufferSize];
            _recvBufferMem = _recvBuffer;
        }

        /// <summary>
        /// Creates a client socket and connects to this instance's endpoint.
        /// </summary>
        /// <param name="timeout">Maximum connection timeout.</param>
        /// <param name="cancellationToken">The token to monitor for cancellation requests.</param>
        /// <returns>True if connection was successful.</returns>
        public async Task<bool> Connect(int timeout, CancellationToken cancellationToken)
        {
            _socket?.Dispose();
            _socket = new Socket(_endpoint.AddressFamily, SocketType.Stream, ProtocolType.Tcp);

            try
            {
                var r = _socket.ConnectAsync(_endpoint);

                // ConnectAsync doesn't offer any means of setting a custom timeout or passing a CancellationToken
                // This spins up a delay task that handles both and waits for the first of the two to finish
                // Inspired by https://makolyte.com/how-to-set-a-timeout-for-tcpclient-connectasync/
                var delayTask = Task.Delay(timeout, cancellationToken);
                var connectionTask = r;

                await await Task.WhenAny(delayTask, connectionTask);

                if (delayTask.IsCompleted)
                {
                    Log.Error("Cannot connect: Connection timed out.");
                    _ended = true;
                    _socket.Dispose();

                    return false;
                }
            }
            catch (SocketException e)
            {
                Log.Error("Cannot connect: " + e.Message);
                _ended = true;
                _socket.Dispose();
                return false;
            }

            _ended = false;
            return true;
        }

        /// <summary>
        /// Sends the specified string, terminated by a zero byte, to the server.
        /// </summary>
        /// <param name="message">The string to send.</param>
        /// <param name="cancellationToken">The token to monitor for cancellation requests.</param>
        /// <exception cref="InvalidOperationException">Thrown when the connection isn't open.</exception>
        /// <returns>A Task representing the asynchronous send operation.</returns>
        public async Task SendMessage(string message, CancellationToken cancellationToken)
        {
            if (_socket == null || _ended || !_socket.Connected) throw new InvalidOperationException();

            var buffer = new byte[Encoding.UTF8.GetByteCount(message) + 1];
            Encoding.UTF8.GetBytes(message, buffer.AsSpan());
            var memory = buffer.AsMemory();
            var sent = 0;

            while (sent < buffer.Length)
            {
                sent += await _socket.SendAsync(memory.Slice(sent), SocketFlags.None, cancellationToken);
                if (cancellationToken.IsCancellationRequested)
                {
                    return;
                }
            }
        }

        /// <summary>
        /// Reads data from the specified stream and sends them over to the server.
        /// Signalises progress in the console output.
        /// </summary>
        /// <param name="stream">An input stream to read and send data from.</param>
        /// <param name="cancellationToken">The token to monitor for cancellation requests.</param>
        /// <exception cref="InvalidOperationException">Thrown when the connection isn't open.</exception>
        /// <exception cref="ArgumentException">Thrown when <paramref name="stream"/> is null or doesn't support reading.</exception>
        /// <returns>A Task representing the asynchronous send operation.</returns>
        public async Task SendFromStream(Stream stream, CancellationToken cancellationToken)
        {
            if (_socket == null || _ended || !_socket.Connected) throw new InvalidOperationException();

            if (stream == null || !stream.CanRead)
            {
                throw new ArgumentException("Cannot read from the specified stream.");
            }

            var buffer = new byte[BufferSize];
            var mem = new ReadOnlyMemory<byte>(buffer);

            var sent = 0;

            while (!cancellationToken.IsCancellationRequested)
            {
                var read = await stream.ReadAsync(buffer, 0, buffer.Length, cancellationToken);
                if (read == 0) break;

                sent += await _socket.SendAsync(mem.Slice(0, read), SocketFlags.None, cancellationToken);

                Console.SetCursorPosition(0, Console.CursorTop);
                Console.Write($"Sent {sent} / {stream.Length}");
            }

            Console.WriteLine();
        }

        /// <summary>
        /// Receives a string message, terminated by a zero byte, from the server.
        /// If a socket error occurs during this operation, terminates the connection and
        /// sets this <see cref="SocketClient"/> instance to the Ended state.
        /// </summary>
        /// <param name="cancellationToken">The token to monitor for cancellation requests.</param>
        /// <returns>A Task that represents the asynchronous message receiving operation.
        /// Its result evaluates to the received message or null when an error occurs or when the operation is cancelled.</returns>
        /// <exception cref="InvalidOperationException">Thrown when the connection isn't open.</exception>
        public async Task<string> ReceiveMessage(CancellationToken cancellationToken)
        {
            if (_socket == null || _ended || !_socket.Connected) throw new InvalidOperationException();

            var readingLongMessage = false;
            StringBuilder sb = null;

            while (!cancellationToken.IsCancellationRequested)
            {
                try
                {
                    var recv = await _socket.ReceiveAsync(_recvBufferMem, SocketFlags.None,
                        cancellationToken);

                    if (recv == 0)
                    {
                        Exit();
                        return null;
                    }

                    // Check whether the received data ends with a <NULL> byte
                    if (_recvBuffer[recv - 1] == 0)
                    {
                        var msgPart = Encoding.UTF8.GetString(_recvBufferMem.Slice(0, recv - 1).Span);

                        if (!readingLongMessage) return msgPart;
                        sb.Append(msgPart);
                        return sb.ToString();
                    }
                    else
                    {
                        readingLongMessage = true;
                        var msgPart = Encoding.UTF8.GetString(_recvBufferMem.Slice(0, recv).Span);
                        sb = new StringBuilder();
                        sb.Append(msgPart);
                    }
                }
                catch (OperationCanceledException)
                {
                    return null;
                }
                catch (SocketException e)
                {
                    Log.Error($"Connection error: {e.Message}");
                    Exit();
                    return null;
                }
            }

            return null;
        }

        /// <summary>
        /// Receives the specified number of bytes from the server and writes them to a stream.
        /// Signalises progress in the console output.
        /// </summary>
        /// <param name="stream">An output stream to write the received data to.</param>
        /// <param name="bytesToFetch">The number of bytes to receive from the server.</param>
        /// <param name="cancellationToken">The token to monitor for cancellation requests.</param>
        /// <returns>A Task that represents the asynchronous data retrieving operation.</returns>
        /// <exception cref="InvalidOperationException">Thrown when the connection isn't open.</exception>
        public async Task ReceiveToStream(Stream stream, long bytesToFetch, CancellationToken cancellationToken)
        {
            if (_socket == null || _ended || !_socket.Connected) throw new InvalidOperationException();

            long receivedTotal = 0;
            var total = bytesToFetch;

            while (!cancellationToken.IsCancellationRequested)
            {
                try
                {
                    var recv = await _socket.ReceiveAsync(_recvBufferMem, SocketFlags.None,
                        cancellationToken);

                    if (recv <= 0)
                    {
                        Exit();
                        return;
                    }

                    receivedTotal += recv;
                    Console.SetCursorPosition(0, Console.CursorTop);
                    Console.Write($"{receivedTotal} / {total}");

                    await stream.WriteAsync(_recvBufferMem.Slice(0, (int) Math.Min(bytesToFetch, recv)),
                        cancellationToken);
                    bytesToFetch -= recv;

                    if (bytesToFetch > 0) continue;
                    if (bytesToFetch < 0)
                    {
                        Log.Warning("Suspicious number of bytes received.");
                    }

                    break;
                }
                catch (OperationCanceledException)
                {
                    return;
                }
            }

            Console.WriteLine();
        }

        /// <summary>
        /// Synchronously disconnects.
        /// </summary>
        public void Exit()
        {
            if (_socket?.Connected ?? false)
            {
                Log.Info("Disconnecting");
                _socket.Disconnect(false);
            }

            _socket?.Dispose();
            _ended = true;
        }

        public void Dispose()
        {
            _socket?.Dispose();
        }
    }
}