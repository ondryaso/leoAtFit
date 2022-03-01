using System;
using System.IO;
using System.Net;
using System.Threading;
using System.Threading.Tasks;
using IpkEpsilon.Common;

namespace IpkEpsilon.Client
{
    public class Program
    {
        private const int ConnectTimeout = 5000;

        public static async Task Main(string[] args)
        {
            var endpoint = ParseArgs(args);
            var socketClient = new SocketClient(endpoint);
            var sftpClient = new SftpClient(socketClient);

            var cts = new CancellationTokenSource();
            var cancelled = false;

            // Graceful handling of SIGINT, SIGTERM
            // https://medium.com/@rainer_8955/gracefully-shutdown-c-apps-2e9711215f6d
            Console.CancelKeyPress += (_, ea) =>
            {
                Log.Info("Exiting client");
                ea.Cancel = true;
                cancelled = true;
                cts.Cancel();
            };

            AppDomain.CurrentDomain.ProcessExit += (o, ea) =>
            {
                if (!cancelled)
                {
                    cts.Cancel();
                }
            };

            try
            {
                await sftpClient.Run(ConnectTimeout, cts.Token);
            }
            catch (OperationCanceledException)
            {
            }
            catch (Exception e)
            {
                Log.Error("An unexpected error occured: " + e.Message);
            }
            finally
            {
                socketClient.Exit();
            }

            cancelled = true;
            Log.Info("Finished");
        }

        /// <summary>
        /// Parses arguments and returns an IPEndPoint object based on the IP address and port specified in the arguments.
        /// If the IP address passed using the -h argument cannot be parsed, a DNS resolve is performed.
        /// If the arguments aren't valid or the DNS resolve fails, the program is terminated with exit code 1.
        /// If the -p port argument is not present, 115 is used.
        /// </summary>
        /// <param name="args">An array of command line arguments.</param>
        /// <returns>An IPEndPoint describing the server IP address and port.</returns>
        private static IPEndPoint ParseArgs(string[] args)
        {
            static void InvalidArgs()
            {
                Log.Error(
                    $"Expected usage: {Path.GetFileName(Environment.GetCommandLineArgs()[0])} -h {{server IP}} [-p {{port}}]");
                Environment.Exit(1);
            }

            if (args.Length != 2 && args.Length != 4)
                InvalidArgs();

            var ip = string.Empty;
            var port = 0;

            for (var i = 0; i < args.Length; i++)
            {
                if (args[i] == "-h")
                {
                    ip = args[++i];
                }
                else if (args[i] == "-p")
                {
                    if (!int.TryParse(args[++i], out port))
                    {
                        InvalidArgs();
                    }

                    if (port < 1 || port > 65535)
                    {
                        InvalidArgs();
                    }
                }
                else
                {
                    InvalidArgs();
                }
            }

            if (!IPAddress.TryParse(ip, out var addr))
            {
                try
                {
                    var dnsEntry = Dns.GetHostEntry(ip);
                    if (dnsEntry.AddressList.Length == 0)
                    {
                        Log.Error("Invalid IP address or hostname.");
                        Environment.Exit(1);
                    }
                    else
                    {
                        addr = dnsEntry.AddressList[0];
                    }
                }
                catch
                {
                    Log.Error("Invalid IP address or hostname.");
                    Environment.Exit(1);
                }
            }

            try
            {
                var endpoint = new IPEndPoint(addr, port == 0 ? 115 : port);
                return endpoint;
            }
            catch (ArgumentOutOfRangeException)
            {
                Log.Error("Invalid port.");
                Environment.Exit(1);
            }

            return null;
        }
    }
}