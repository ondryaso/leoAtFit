using System;
using System.Collections.Concurrent;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Threading.Tasks;
using IpkEpsilon.Common;
using IpkEpsilon.Server.Sftp.Abstractions;

namespace IpkEpsilon.Server.Network
{
    /// <summary>
    /// Represents a basic TCP asynchronous server that listens on an endpoint, accepts clients
    /// and creates <see cref="ClientHandler"/> instances for them.
    /// </summary>
    /// <remarks>
    /// Inspired by a MSDN example: https://docs.microsoft.com/en-gb/dotnet/framework/network-programming/asynchronous-server-socket-example 
    /// </remarks>
    public class SocketServer
    {
        // The size of receive buffers allocated by ClientHandler instances
        public const int BufferSize = 1400;

        // The maximum number of milliseconds to wait between accepting blocks of data when transferring a file. 
        public const int ReceiveTimeout = 3000;

        // The maximum number of milliseconds to wait for a socket to exit.
        public const int ExitTimeout = 5000;
        
        // The maximum length of the pending connections queue
        private const int BacklogSize = 8;

        private readonly IPEndPoint _endpoint;
        private readonly ISftpProviderFactory _sftpProviderFactory;
        private readonly ManualResetEventSlim _clientDone = new(false);

        private bool _running;
        private Socket _socket;

        private ConcurrentDictionary<int, ClientHandler> _clients;

        public SocketServer(IPEndPoint endpoint, ISftpProviderFactory sftpProviderFactory)
        {
            _endpoint = endpoint;
            _sftpProviderFactory = sftpProviderFactory;
        }

        /// <summary>
        /// Binds to this SocketServer's endpoint and begins listening for connections until cancellation is requested.
        /// </summary>
        /// <param name="cancellationToken">The token to monitor for cancellation requests.</param>
        /// <exception cref="InvalidOperationException">Thrown when attempting to execute this method on
        /// an instance that is currently running and accepting clients.</exception>
        public void Run(CancellationToken cancellationToken)
        {
            if (_running)
            {
                throw new InvalidOperationException("Server has already started.");
            }

            _socket = new Socket(_endpoint.AddressFamily, SocketType.Stream, ProtocolType.Tcp);
            _clients = new ConcurrentDictionary<int, ClientHandler>();
            _running = true;

            try
            {
                Log.Info($"Starting server on {_endpoint.Address}, port {_endpoint.Port}");
                _socket.Bind(_endpoint);
                _socket.Listen(BacklogSize);

                while (!cancellationToken.IsCancellationRequested)
                {
                    _clientDone.Reset();

                    _socket.BeginAccept(AcceptConnectionCallback, cancellationToken);

                    try
                    {
                        // Wait for a client to be accepted and then wait for another one
                        _clientDone.Wait(cancellationToken);
                    }
                    catch (OperationCanceledException)
                    {
                        break;
                    }
                }
            }
            catch (Exception e)
            {
                Log.Error("Server socket error: " + e.Message);
            }
            finally
            {
                Log.Info($"Terminating server {_endpoint.Address}");

                foreach (var client in _clients)
                {
                    client.Value.WaitForExit();
                }

                var s = _socket;
                _socket = null;
                s.Close(ExitTimeout);
                s.Dispose();

                _clients = null;
                _running = false;
            }
        }

        private void AcceptConnectionCallback(IAsyncResult r)
        {
            // This could get invoked after the accepting loop has been cancelled.
            if (_socket == null)
            {
                return;
            }

            _clientDone.Set();
            ClientHandler handler;

            try
            {
                var clientSocket = _socket.EndAccept(r);
                handler = new ClientHandler(clientSocket, _sftpProviderFactory);
                _clients.TryAdd(handler.GetHashCode(), handler);
            }
            catch (OperationCanceledException)
            {
                return;
            }
            catch
            {
                Log.Error("An exception has occured while accepting a client.");
                return;
            }

            // ReSharper disable once PossibleNullReferenceException
            var ct = (CancellationToken) r.AsyncState;

            try
            {
                // ReSharper disable once MethodSupportsCancellation
                handler.Run(ct).Wait();
                Log.Info("Client disconnected");
                _clients?.TryRemove(handler.GetHashCode(), out _);
            }
            catch (SocketException e)
            {
                Log.Warning("Client disconnected: " + e.Message);
            }
            catch (OperationCanceledException)
            {
            }
            catch (Exception e)
            {
                handler.WaitForExit();
                Log.Error("Unexpected error occured, client disconnected: " + e.Message);
            }
        }
    }
}