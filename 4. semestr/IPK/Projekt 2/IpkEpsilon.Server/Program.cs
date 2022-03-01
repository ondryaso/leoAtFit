using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.NetworkInformation;
using System.Net.Sockets;
using System.Threading;
using System.Threading.Tasks;
using IpkEpsilon.Common;
using IpkEpsilon.Server.Network;
using IpkEpsilon.Server.Sftp;

namespace IpkEpsilon.Server
{
    public class Program
    {
        public static void Main(string[] args)
        {
            Log.AppendTime = true;

            var (endpointsEnumerable, users) = ParseArgs(args);
            var endpoints = endpointsEnumerable?.ToList();
            //endpoints = new List<IPEndPoint>() {endpoints[0]};
                                            
            if (endpoints == null || endpoints.Count == 0)
            {
                Log.Error("Interface has no usable addresses");
                Environment.Exit(1);
                return;
            }

            var acp = new FileAccountProvider();
            try
            {
                acp.LoadUserDatabase(users);
            }
            catch (Exception e)
            {
                Log.Error("Cannot read user database: " + e.Message);
                Environment.Exit(2);
                return;
            }

            var factory = new SftpProviderFactory(acp);

            var cancelled = false;
            var cts = new CancellationTokenSource();
            var tasks = new Task[endpoints.Count];
            var tasksCounter = 0;

            foreach (var endpoint in endpoints)
            {
                var eTask = Task.Run(() =>
                {
                    var server = new SocketServer(endpoint, factory);
                    server.Run(cts.Token);
                }, cts.Token);

                tasks[tasksCounter++] = eTask;
            }

            // Graceful handling of SIGINT, SIGTERM
            // https://medium.com/@rainer_8955/gracefully-shutdown-c-apps-2e9711215f6d
            Console.CancelKeyPress += (_, ea) =>
            {
                Log.Info("Exiting server");
                ea.Cancel = true;
                cancelled = true;
                cts.Cancel();
            };

            AppDomain.CurrentDomain.ProcessExit += (_, _) =>
            {
                if (!cancelled)
                {
                    Log.Info("Exiting server");
                    cts.Cancel();
                }
            };

            Task.WaitAll(tasks);
        }

        /// <summary>
        /// Parses command line arguments. Calls <see cref="GetInterfaceAddresses"/> to resolve the interface
        /// and create <see cref="IPEndPoint"/> objects that describe the IP (both v4 and v6) addresses to bind server to,
        /// together with a port. Returns these objects together with a path to a user database file.
        /// If the arguments are not valid, the program is terminated with exit code 1.
        /// </summary>
        /// <param name="args">An array of command line arguments.</param>
        /// <returns>A tuple of a list of target endpoints and a path to a user database file.</returns>
        private static (IEnumerable<IPEndPoint> Endpoints, string UserDatabaseFile) ParseArgs(string[] args)
        {
            static void InvalidArgs()
            {
                Log.Error(
                    $"Expected usage: {Path.GetFileName(Environment.GetCommandLineArgs()[0])} -u {{user database file}} [-i {{interface}}] [-p {{port}}]");
                Environment.Exit(1);
            }

            var len = args.Length;
            if (len == 0 || len % 2 != 0)
                InvalidArgs();

            string users = null;
            int? port = null;
            var interfaces = new List<string>();

            for (var i = 0; i < args.Length; i++)
            {
                switch (args[i])
                {
                    case "-u":
                        if (users != null)
                            InvalidArgs();

                        users = args[++i];
                        break;
                    case "-i":
                        interfaces.Add(args[++i]);
                        break;
                    case "-p":
                    {
                        if (!int.TryParse(args[++i], out var p))
                        {
                            InvalidArgs();
                        }

                        if (port != null)
                            InvalidArgs();

                        port = p;
                        if (p < 1 || p > 65535)
                        {
                            InvalidArgs();
                        }

                        break;
                    }
                    default:
                        InvalidArgs();
                        break;
                }
            }

            if (users == null)
                InvalidArgs();

            port ??= 115;

            var endpoints = new List<IPEndPoint>();
            foreach (var i in interfaces.Distinct(StringComparer.InvariantCultureIgnoreCase))
            {
                var e = GetInterfaceAddresses(i, port.Value);
                if (e != null)
                {
                    endpoints.AddRange(e);
                }
            }

            if (endpoints.Count == 0)
            {
                if (interfaces.Count > 0)
                {
                    Log.Error($"No usable IP addresses found on the specified interface(s)");
                    Environment.Exit(1);
                    return (null, null);
                }

                endpoints.Add(new IPEndPoint(IPAddress.Any, port.Value));
                endpoints.Add(new IPEndPoint(IPAddress.IPv6Any, port.Value));
            }

            return (endpoints, users);
        }

        /// <summary>
        /// Resolves the interface and creates <see cref="IPEndPoint"/> objects that describe the IPv4 and IPv6
        /// addresses to bind server to. All IPv4 and IPv6 unicast address assigned to the interface are used.
        /// If <paramref name="nInterface"/> is null or empty, null is returned.
        /// If no interface with the specified name exists, terminates the program with exit code 1. 
        /// </summary>
        /// <param name="nInterface">The name of the interface to use.</param>
        /// <param name="port">The port to listen on.</param>
        /// <returns>An enumerable of <see cref="IPEndPoint"/> objects describing addresses to bind to.</returns>
        private static IEnumerable<IPEndPoint> GetInterfaceAddresses(string nInterface, int port)
        {
            if (string.IsNullOrWhiteSpace(nInterface))
            {
                return null;
            }

            var iface = NetworkInterface.GetAllNetworkInterfaces().FirstOrDefault(i =>
                i.Name.Equals(nInterface, StringComparison.InvariantCultureIgnoreCase) ||
                i.Id.Equals(nInterface, StringComparison.InvariantCultureIgnoreCase));

            if (iface == null)
            {
                Log.Error($"Interface not found: {nInterface}");
                Environment.Exit(1);
                return null;
            }

            return iface.GetIPProperties().UnicastAddresses
                .Where(i => i.Address.AddressFamily == AddressFamily.InterNetwork ||
                            i.Address.AddressFamily == AddressFamily.InterNetworkV6)
                .Select(i => new IPEndPoint(i.Address, port));
        }
    }
}