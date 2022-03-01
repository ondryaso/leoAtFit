using System;
using System.IO;
using System.Net.Sockets;
using System.Threading;
using System.Threading.Tasks;
using IpkEpsilon.Common;

namespace IpkEpsilon.Client
{
    /// <summary>
    /// Represents a SFTP protocol client. Handles accepting commands from the user and communicating with
    /// a server (using a <see cref="SocketClient"/>).
    /// </summary>
    /// <remarks>
    /// This client implementation does not perform a whole lot of checks on the messages sent to the server.
    /// Only the commands that modify the client state (e.g. RETR, STOR or TYPE) are managed; most inputs from
    /// the user are, however, sent to the server without checking whether they're correct. 
    /// </remarks>
    public class SftpClient
    {
        // The maximum number of milliseconds to wait for a STOR confirmation message from the server.
        private const int StorTimeout = 3000;

        private enum State
        {
            WaitingRetr, // RETR has been sent to the server, wait for a number of bytes
            WaitingStor, // STOR has been sent to the server, wait for confirmation
            Other // We're not waiting for any specific response
        }

        private readonly SocketClient _client;

        private bool _done;
        private bool _usingAscii;
        private bool? _toUseAscii;
        private State _fState = State.Other;

        private string _targetFile;

        /// <summary>
        /// Creates a <see cref="SftpClient"/> instance that uses the specified <see cref="SocketClient"/> to communicate
        /// with a server.
        /// </summary>
        /// <param name="client">The <see cref="SocketClient"/> to use for communication.</param>
        public SftpClient(SocketClient client)
        {
            _client = client;
        }

        /// <summary>
        /// Executes the main client loop. Connects to the server, then waits for a command from the user,
        /// sends it to the server, waits for a response and processes it.
        /// This runs until cancellation is requested on the passed <paramref name="cancellationToken"/>.
        /// </summary>
        /// <param name="connectTimeout">The maximum number of milliseconds to wait for the connection to be established.</param>
        /// <param name="cancellationToken">The token to monitor for cancellation requests.</param>
        /// <returns>A Task that represents the asynchronous processing loop.</returns>
        public async Task Run(int connectTimeout, CancellationToken cancellationToken)
        {
            if (!await _client.Connect(connectTimeout, cancellationToken))
            {
                return;
            }

            var initialMessage = await _client.ReceiveMessage(cancellationToken);
            if (!initialMessage.StartsWith('+'))
            {
                Log.Warning($"Server unavailable – negative welcome message received:\n{initialMessage}");
                _client.Exit();
                return;
            }

            Console.WriteLine(initialMessage);
            Log.Info("Server welcomed us");

            while (!cancellationToken.IsCancellationRequested)
            {
                Console.Write("> ");
                var line = ConsoleUtils.ReadLine(cancellationToken);

                // If our ReadLine returns null, cancellation has been requested
                if (line == null) break;

                // If ProcessLocalCommand() returns false, the command is invalid and another one should be acquired
                // from the user
                if (ProcessLocalCommand(line))
                {
                    try
                    {
                        await _client.SendMessage(line, cancellationToken);
                        await GetAndProcessAnswer(cancellationToken);
                    }
                    catch (OperationCanceledException)
                    {
                        _done = true;
                    }
                    catch (Exception e)
                    {
                        Log.Warning("An unexpected error occured: " + e.Message);
                        _done = true;
                    }
                }

                if (_done)
                {
                    break;
                }

                // This could happen if a handled exception occurs when processing a message in the SocketClient.
                if (_client.Ended)
                {
                    return;
                }
            }

            _client.Exit();
        }

        /// <summary>
        /// Processes a command executed by the user and decides whether it should be sent to the server.
        /// If the command is RETR and the argument part of the command is not empty,
        /// changes this client's state to <see cref="State.WaitingRetr"/>.
        /// </summary>
        /// <param name="line">The command line acquired from the user.</param>
        /// <returns>True if the command is valid; otherwise false.</returns>
        private bool ProcessLocalCommand(string line)
        {
            line = line.Trim();
            if (line == string.Empty) return false;

            var spaceIndex = line.IndexOf(' ');
            var verb = (spaceIndex == -1 ? line : line.Substring(0, spaceIndex))
                .ToLowerInvariant();
            var argLine = spaceIndex == -1 ? null : line.Substring(spaceIndex + 1);

            switch (verb)
            {
                case "type":
                    return ProcessTypeCommand(argLine);
                case "done":
                    _done = true;
                    break;
                case "retr":
                    if (argLine == null)
                    {
                        Log.Info("Missing RETR argument");
                        return false;
                    }

                    _fState = State.WaitingRetr;
                    _targetFile = argLine;
                    break;
                case "stor":
                    return ProcessStorCommand(argLine);
            }

            return true;
        }

        /// <summary>
        /// Processes a TYPE command. Checks whether a valid mode follows and sets this client's mode flag accordingly.
        /// </summary>
        /// <param name="argLine">The argument part of the executed command.</param>
        /// <returns>True if the command is valid; otherwise false.</returns>
        private bool ProcessTypeCommand(string argLine)
        {
            argLine = argLine?.ToLowerInvariant();
            switch (argLine)
            {
                case "a":
                    _toUseAscii = true;
                    break;
                case "b":
                case "c":
                    _toUseAscii = false;
                    break;
                default:
                    _toUseAscii = null;
                    Log.Info("Invalid transfer type");
                    return false;
            }

            return true;
        }

        /// <summary>
        /// Processes a STOR command. Checks whether a valid mode follows, asks for a source file path,
        /// checks whether it exists and changes this client's state to <see cref="State.WaitingStor"/>.
        /// </summary>
        /// <param name="argLine">The argument part of the executed command.</param>
        /// <returns>True if the command is valid and the source file exists; otherwise false.</returns>
        private bool ProcessStorCommand(string argLine)
        {
            if (argLine == null)
            {
                Log.Info("Missing STOR argument");
                return false;
            }

            var argLineLower = argLine.ToLowerInvariant();
            if (!argLineLower.StartsWith("new ") && !argLineLower.StartsWith("old ") &&
                !argLineLower.StartsWith("app "))
            {
                Log.Info("Invalid STOR mode");
                return false;
            }

            Console.Write("What file should be sent?\n> ");
            var targetPath = Console.ReadLine();

            try
            {
                var fi = new FileInfo(targetPath);
                // Test if we can open the file for reading
                fi.OpenRead().Close();
                _targetFile = fi.FullName;

                if (!fi.Exists)
                {
                    Console.WriteLine("File doesn't exist; aborting");
                    return false;
                }
            }
            catch
            {
                Console.WriteLine("File doesn't exist; aborting");
                return false;
            }

            _fState = State.WaitingStor;
            return true;
        }

        /// <summary>
        /// Receives a string message from the server, writes it to the standard output stream and, if the client is
        /// in a state waiting for a particular answer, processes it and performs the corresponding action: sets
        /// transfer mode or performs file sending/retrieval.
        /// </summary>
        /// <param name="cancellationToken">The token to monitor for cancellation requests.</param>
        /// <returns>A Task that represents the asynchronous message processing operation.</returns>
        private async Task GetAndProcessAnswer(CancellationToken cancellationToken)
        {
            var msg = await _client.ReceiveMessage(cancellationToken);
            Console.WriteLine(msg);

            if (_toUseAscii != null)
            {
                // Handling response to TYPE
                if (msg.StartsWith("+"))
                {
                    _usingAscii = _toUseAscii.Value;
                    Log.Info($"Changed mode to " + (_usingAscii ? "ASCII" : "binary"));
                }
                else
                {
                    Log.Warning("Server refused to change mode; keeping " + (_usingAscii ? "ASCII" : "binary"));
                }

                _toUseAscii = null;
                return;
            }

            if (_fState == State.WaitingRetr)
            {
                // Handling response to RETR - either a minus, or a number (length of file)
                if (msg.StartsWith("-"))
                {
                    Log.Info("Negative server response; aborting RETR");
                    _fState = State.Other;
                    return;
                }

                if (!long.TryParse(msg, out var bytesToTransfer))
                {
                    Log.Warning("Server responded unexpectedly; aborting RETR");
                    _fState = State.Other;
                    return;
                }

                if (bytesToTransfer <= 0)
                {
                    Log.Info("Refusing to transfer an empty file");
                    _fState = State.Other;
                    Console.WriteLine("> STOP");
                    await _client.SendMessage("STOP", cancellationToken);
                    await _client.ReceiveMessage(cancellationToken);
                    return;
                }

                await ProcessRetr(bytesToTransfer, cancellationToken);
                return;
            }

            if (_fState == State.WaitingStor)
            {
                // Handling response to STOR
                if (!msg.StartsWith("+"))
                {
                    Log.Info("Negative server response; aborting STOR");
                    _fState = State.Other;
                    return;
                }

                await ProcessStor(cancellationToken);
            }
        }

        /// <summary>
        /// Continues a pending RETR operation. Asks the user for a target file path, asks the server to SEND
        /// the file and saves the received data.
        /// </summary>
        /// <param name="bytes">Number of bytes expected from the server.</param>
        /// <param name="cancellationToken">The token to monitor for cancellation requests.</param>
        /// <returns>A Task that represents the asynchronous file transfer operation.</returns>
        private async Task ProcessRetr(long bytes, CancellationToken cancellationToken)
        {
            Console.Write("Where should be the file stored? Press Enter to save to current directory.\n> ");
            var targetPath = ConsoleUtils.ReadLine(cancellationToken);
            if (targetPath.Length == 0)
            {
                if (_targetFile.EndsWith('/') || _targetFile.EndsWith('\\'))
                    _targetFile = _targetFile[..^1];
                if (_targetFile.Contains('/'))
                    targetPath = _targetFile.Substring(_targetFile.LastIndexOf('/') + 1);
                else if (_targetFile.Contains('\\'))
                    targetPath = _targetFile.Substring(_targetFile.LastIndexOf('\\') + 1);
                else
                    targetPath = _targetFile;
            }

            try
            {
                var fi = new FileInfo(targetPath);
                targetPath = fi.FullName;
                if (fi.Exists)
                {
                    var resp = ConsoleUtils.ReadYesNo("Target file exists. Should I overwrite it?");
                    if (!resp)
                    {
                        _fState = State.Other;
                        Console.WriteLine("> STOP");
                        await _client.SendMessage("STOP", cancellationToken);
                        await _client.ReceiveMessage(cancellationToken);
                        return;
                    }
                }
            }
            catch
            {
                _fState = State.Other;
                Console.WriteLine("Invalid target path; aborting");
                Console.WriteLine("> STOP");
                await _client.SendMessage("STOP", cancellationToken);
                await _client.ReceiveMessage(cancellationToken);
                return;
            }

            Stream stream;
            try
            {
                stream = new FileStream(targetPath, FileMode.Create, FileAccess.Write);
            }
            catch (SystemException e)
            {
                _fState = State.Other;
                Log.Error("Cannot open target file for writing: " + e.Message);
                Console.WriteLine("> STOP");
                await _client.SendMessage("STOP", cancellationToken);
                await _client.ReceiveMessage(cancellationToken);
                return;
            }

            Console.WriteLine("> SEND");

            try
            {
                await _client.SendMessage("SEND", cancellationToken);
                await _client.ReceiveToStream(stream, bytes, cancellationToken);
            }
            catch (SocketException e)
            {
                Log.Error("Unexpected error retrieving file: " + e.Message);
                _client.Exit();
                _done = true;
            }
            finally
            {
                await stream.FlushAsync(cancellationToken);
                await stream.DisposeAsync();
            }

            _fState = State.Other;
        }

        /// <summary>
        /// Continues a pending STOR operation. Sends a SIZE command to the server, gets its response and if it's
        /// positive, transfers the file.
        /// </summary>
        /// <param name="cancellationToken">The token to monitor for cancellation requests.</param>
        /// <returns>A Task that represents the asynchronous file transfer operation.</returns>
        private async Task ProcessStor(CancellationToken cancellationToken)
        {
            _fState = State.Other;

            var fi = new FileInfo(_targetFile);
            if (!fi.Exists)
            {
                Log.Error("Source file not found");
                _done = true;
                _client.Exit();
            }

            Stream stream = null;
            var len = fi.Length;

            try
            {
                if (_usingAscii)
                {
                    var fs = new FileStream(_targetFile, FileMode.Open, FileAccess.Read, FileShare.Read);
                    (stream, len) = AsciiStreamUtils.LoadAsciiFile(fs); // LoadAsciiFile closes the original stream.
                }
                else
                {
                    stream = new FileStream(_targetFile, FileMode.Open, FileAccess.Read, FileShare.Read);
                }
            }
            catch (SystemException e)
            {
                Log.Error("Cannot open source file for reading: " + e.Message);
                _done = true;
                _client.Exit();
                return;
            }

            Console.WriteLine("> SIZE " + len);
            await _client.SendMessage("SIZE " + len, cancellationToken);

            var msg = await _client.ReceiveMessage(cancellationToken);
            if (msg == null)
            {
                return;
            }

            Console.WriteLine(msg);

            if (!msg.StartsWith("+"))
            {
                Log.Warning("Server refused STOR");
                return;
            }

            try
            {
                await _client.SendFromStream(stream, cancellationToken);

                var cts = new CancellationTokenSource(StorTimeout);
                msg = await _client.ReceiveMessage(cts.Token);
                if (msg == null)
                {
                    Log.Error("Didn't receive STOR confirmation");
                    _done = true;
                    _client.Exit();
                    return;
                }

                if (!msg.StartsWith("+"))
                {
                    Log.Warning("Negative STOR confirmation – an error may have occured while storing the file");
                }
            }
            catch (SocketException e)
            {
                Log.Error("Unexpected error sending file: " + e.Message);
                _done = true;
                _client.Exit();
            }
            finally
            {
                await stream.DisposeAsync();
            }
        }
    }
}