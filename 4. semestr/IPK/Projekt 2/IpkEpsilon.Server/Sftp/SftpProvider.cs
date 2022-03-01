using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Threading.Tasks;
using IpkEpsilon.Common;
using IpkEpsilon.Server.Sftp.Abstractions;

namespace IpkEpsilon.Server.Sftp
{
    internal class SftpProvider : ISftpProvider
    {
        private enum State
        {
            WaitingForLogin, // Immediately after connection begins
            WaitingForPassword, // USER has been sent, waiting for PASS
            WaitingForFileSendConfirmation, // RETR has been used and we responded, now we're waiting for SEND or STOP
            WaitingForStoreSize, // STOR has been used and we responded, now we're waiting for SIZE
            WaitingForFile, // We're accepting a file, waiting for a NotifyFileReceived() call
            WaitingForNewFileName, // NAME has been used, we're waiting for TOBE
            Ready // We're waiting for any command
        }

        private readonly IAccountProvider _accountProvider;

        // Static list of valid protocol commands. The boolean value says whether it expects arguments.
        private readonly Dictionary<string, bool> _commandDefinitions = new()
        {
            {"user", true},
            {"acct", true},
            {"pass", true},
            {"type", true},
            {"list", true},
            {"cdir", true},
            {"kill", true},
            {"name", true},
            {"done", false},
            {"retr", true},
            {"send", false},
            {"stop", false},
            {"stor", true},
            {"size", true},
            {"tobe", true}
        };

        private bool _useAsciiMapping;
        private string _username;
        private string _currentDirectory;
        private string _currentFile;
        private State _state;
        private FileMode _storeMode;
        private Stream _currentStream;

        internal SftpProvider(IAccountProvider accountProvider)
        {
            _accountProvider = accountProvider;
            _currentDirectory = Environment.CurrentDirectory;
        }

        public CommandExecutionResult MakeGreeting(string identifier)
        {
            return string.IsNullOrEmpty(identifier)
                ? new CommandExecutionResult("+Hello")
                : new CommandExecutionResult($"+Hello, your client ID is {identifier}");
        }

        /// <inheritdoc />
        public void NotifyFileReceived()
        {
            if (_state != State.WaitingForFile)
            {
                throw new InvalidOperationException();
            }

            _state = State.Ready;
        }

        /// <inheritdoc />
        /// <exception cref="ArgumentNullException">Thrown when <paramref name="command"/> is null.</exception>
        public CommandExecutionResult HandleCommand(string command)
        {
            if (command == null) throw new ArgumentNullException(nameof(command));
            command = command.Trim();

            // Empty command will not throw an exception, it might've come from the client
            if (command == string.Empty) return CommandExecutionResult.InvalidCommand;

            var spaceIndex = command.IndexOf(' ');
            var verb = (spaceIndex == -1 ? command : command.Substring(0, spaceIndex))
                .ToLowerInvariant();
            var argLine = spaceIndex == -1 ? null : command.Substring(spaceIndex + 1);

            if (!_commandDefinitions.ContainsKey(verb))
            {
                if (_state == State.Ready)
                {
                    return CommandExecutionResult.InvalidCommand;
                }

                if (_state == State.WaitingForLogin || _state == State.WaitingForPassword)
                {
                    // When waiting for login or password, we want to stay in that state
                    return new CommandExecutionResult("-Invalid command; please login first");
                }

                // When waiting for a specific command and not getting it, abort the action
                _state = State.Ready;
                return new CommandExecutionResult("-Invalid command; aborting current operation");
            }

            var needsArgs = _commandDefinitions[verb];
            if (needsArgs && argLine == null)
            {
                return new CommandExecutionResult($"-Expected an argument");
            }

            return _state switch
            {
                State.Ready => HandleGeneralCommand(verb, argLine),
                State.WaitingForLogin => HandleWaitingForLogin(verb, argLine),
                State.WaitingForPassword => HandleWaitingForPassword(verb, argLine),
                State.WaitingForFileSendConfirmation => HandleWaitingForFileSendConfirmation(verb),
                State.WaitingForStoreSize => HandleWaitingForStoreSize(verb, argLine),
                State.WaitingForFile =>
                    // This should not happen (when accepting a file, the ClientHandler is instructed to read
                    // from the socket and write to a stream). We're waiting for the ClientHandler to call our
                    // NotifyFileReceived().
                    throw new InvalidOperationException("Cannot handle commands while receiving a file"),
                State.WaitingForNewFileName => HandleWaitingForNewFileName(verb, argLine),
                _ => throw new InvalidOperationException("Invalid state")
            };
        }

        private CommandExecutionResult HandleGeneralCommand(string verb, string argLine)
        {
            switch (verb)
            {
                case "type":
                    argLine = argLine.ToLowerInvariant();
                    if (argLine == "a")
                    {
                        _useAsciiMapping = true;
                        return new CommandExecutionResult("+Set ASCII transfer mode");
                    }
                    else if (argLine == "b" || argLine == "c")
                    {
                        _useAsciiMapping = false;
                        return new CommandExecutionResult("+Set binary/continuous transfer mode");
                    }
                    else
                    {
                        return new CommandExecutionResult("-Wrong type");
                    }
                case "list":
                    // Either there's only a V/F argument or there is the specified and a directory path
                    if (argLine.Length == 1 || (argLine.Length > 2 && argLine[1] == ' '))
                    {
                        bool verbose;

                        var mode = char.ToLowerInvariant(argLine[0]);
                        if (mode == 'f')
                            verbose = false;
                        else if (mode == 'v')
                            verbose = true;
                        else
                            return new CommandExecutionResult("-Wrong listing mode (use V or F)");

                        try
                        {
                            return MakeDirectoryListing(verbose, argLine.Length == 1 ? null : argLine.Substring(2));
                        }
                        catch (SystemException e)
                        {
                            return new CommandExecutionResult("-Cannot list directory: " + e.Message);
                        }
                    }
                    else
                    {
                        return new CommandExecutionResult("-Wrong listing mode (use V or F)");
                    }
                case "cdir":
                    var newDir = LookupPath(argLine, false);
                    if (newDir != null)
                    {
                        _currentDirectory = newDir;
                        return new CommandExecutionResult("!Directory changed");
                    }
                    else
                    {
                        return new CommandExecutionResult("-Directory does not exist");
                    }
                case "kill":
                    var file = LookupPath(argLine, true);
                    if (file == null)
                    {
                        return new CommandExecutionResult("-Cannot delete: File does not exist");
                    }

                    try
                    {
                        File.Delete(file);
                        return new CommandExecutionResult("+File deleted");
                    }
                    catch (SystemException e)
                    {
                        return new CommandExecutionResult("-Cannot delete: " + e.Message);
                    }
                case "name":
                    file = LookupPath(argLine, true);
                    if (file == null)
                    {
                        return new CommandExecutionResult("-Cannot rename: File does not exist");
                    }

                    _currentFile = file;
                    _state = State.WaitingForNewFileName;
                    return new CommandExecutionResult("+Send new file name");
                case "done":
                    return new CommandExecutionResult("+Bye", null, ResponseMode.SendString, NextMode.Exit);
                case "retr":
                    file = LookupPath(argLine, true);
                    if (file == null)
                    {
                        return new CommandExecutionResult("-File does not exist");
                    }
                    else
                    {
                        var fi = new FileInfo(file);
                        _state = State.WaitingForFileSendConfirmation;
                        _currentFile = file;

                        if (_useAsciiMapping)
                        {
                            // If an error occurs, it will be handled in SocketServer.AcceptConnectionCallback
                            // Client will be closed
                            var (stream, len) =
                                AsciiStreamUtils.LoadAsciiFile(fi.Open(FileMode.Open, FileAccess.Read, FileShare.Read));
                            _currentStream = stream;
                            return new CommandExecutionResult(len.ToString());
                        }
                        else
                        {
                            _currentStream = fi.Open(FileMode.Open, FileAccess.Read, FileShare.Read);
                            return new CommandExecutionResult(fi.Length.ToString());
                        }
                    }
                case "stor":
                    return HandleStor(argLine);
            }

            return CommandExecutionResult.InvalidCommand;
        }

        private CommandExecutionResult HandleWaitingForLogin(string verb, string argLine)
        {
            // Only the USER command is accepted
            if (verb != "user")
                return CommandExecutionResult.InvalidCommand;

            if (!_accountProvider.HasUser(argLine))
                return new CommandExecutionResult("-Invalid user-id");

            _username = argLine;
            _state = State.WaitingForPassword;

            return new CommandExecutionResult("+Send password");
        }

        private CommandExecutionResult HandleWaitingForPassword(string verb, string argLine)
        {
            // ACCT commands are accepted but silently ignored
            if (verb == "acct")
                return new CommandExecutionResult("+Send password");

            // If it's not the ACCT command, it has to be the PASS command
            if (verb != "pass")
                return CommandExecutionResult.InvalidCommand;

            if (!_accountProvider.AuthenticateUser(_username, argLine))
                return new CommandExecutionResult("-Wrong password, try again");

            _state = State.Ready;
            return new CommandExecutionResult("!Logged in");
        }

        private CommandExecutionResult HandleWaitingForFileSendConfirmation(string verb)
        {
            // Whatever happens, the next command will not be a special one
            _state = State.Ready;

            if (verb == "send")
            {
                // ASCII stream is prepared when the RETR command is processed
                // (without it, we wouldn't know the number of bytes)
                if (_useAsciiMapping)
                {
                    var aStream = _currentStream;
                    _currentStream = null;
                    // Mind the SendStream response mode
                    // NextMode is ReadCommand – after the whole stream is sent, we're moving on and reading the next command 
                    return new CommandExecutionResult(null, aStream, ResponseMode.SendStream,
                        NextMode.ReadCommand);
                }

                // If an error occurs, it will be handled in SocketServer.AcceptConnectionCallback
                // Client will be closed
                // Mind the SendStream response mode
                return new CommandExecutionResult(null, _currentStream, ResponseMode.SendStream,
                    NextMode.ReadCommand);
            }

            if (verb == "stop")
            {
                _currentFile = null;
                _currentStream?.Dispose();
                return new CommandExecutionResult("+This is so sad, aborted");
            }

            return new CommandExecutionResult("-Invalid command, RETR aborted");
        }

        private CommandExecutionResult HandleWaitingForStoreSize(string verb, string argLine)
        {
            if (verb != "size")
            {
                _state = State.Ready;
                return new CommandExecutionResult("-Invalid command, STOR aborted");
            }

            if (!long.TryParse(argLine, out var bytes))
            {
                _state = State.Ready;
                return new CommandExecutionResult("-Invalid file length, STOR aborted");
            }

            try
            {
                _state = State.WaitingForFile;
                var stream = new FileStream(_currentFile, _storeMode, FileAccess.Write, FileShare.None);
                // Mind the SendString response mode and the ReceiveBytes next mode
                return new CommandExecutionResult("+Send file", stream, ResponseMode.SendString,
                    NextMode.ReceiveBytes, bytes);
            }
            catch (Exception e)
            {
                _state = State.Ready;
                Log.Error("Cannot perform STOR: " + e.Message);
                return new CommandExecutionResult("-Cannot open file for writing, STOR aborted");
            }
        }

        private CommandExecutionResult MakeDirectoryListing(bool verbose, string dir)
        {
            if (string.IsNullOrWhiteSpace(dir)) dir = _currentDirectory;
            var path = LookupPath(dir, false);
            if (path == null)
            {
                return new CommandExecutionResult("-Directory not found");
            }

            var sb = new StringBuilder("+", path.Length + 64);
            sb.Append(path);
            sb.Append("\r\n");

            if (verbose)
            {
                // Verbose mode lists directories as well as files
                foreach (var innerDir in Directory.EnumerateDirectories(path))
                {
                    var di = new DirectoryInfo(innerDir);

                    sb.Append($"D\t{di.Attributes:X}\t0\t{di.CreationTime:s}\t{di.LastWriteTime:s}\t{di.Name}");
                    sb.Append("\r\n");
                }

                foreach (var file in Directory.EnumerateFiles(path))
                {
                    var fi = new FileInfo(file);

                    sb.Append(
                        $"F\t{fi.Attributes:X}\t{fi.Length}\t{fi.CreationTime:s}\t{fi.LastWriteTime:s}\t{fi.Name}");
                    sb.Append("\r\n");
                }
            }
            else
            {
                // The _F_ mode only lists files, per spec
                foreach (var file in Directory.EnumerateFiles(path))
                {
                    sb.Append(Path.GetFileName(file));
                    sb.Append("\r\n");
                }
            }

            return new CommandExecutionResult(sb.ToString());
        }

        private CommandExecutionResult HandleStor(string argLine)
        {
            if (argLine.Length <= 4)
                return new CommandExecutionResult("-Invalid STOR mode");
            var mode = argLine.Substring(0, 3).ToLowerInvariant();
            argLine = argLine.Substring(4);

            var (file, fileExists) = MakePath(argLine);
            if (file == null)
            {
                return new CommandExecutionResult("-Invalid file name");
            }

            _currentFile = file;

            switch (mode)
            {
                case "new":
                    if (fileExists)
                    {
                        _currentFile = null;
                        return new CommandExecutionResult("-System doesn't support generations");
                    }

                    _state = State.WaitingForStoreSize;
                    _storeMode = FileMode.Create;
                    return new CommandExecutionResult("+File does not exist, will create new file");
                case "old":
                    _state = State.WaitingForStoreSize;
                    _storeMode = FileMode.Create;
                    return new CommandExecutionResult(fileExists
                        ? "+File exists and will be overwritten"
                        : "+Send new file size");
                case "app":
                    _state = State.WaitingForStoreSize;
                    _storeMode = FileMode.Append;
                    return new CommandExecutionResult(fileExists
                        ? "+File exists and will be appended to"
                        : "+Send new file size");
            }

            return new CommandExecutionResult("-Invalid STOR mode");
        }

        private CommandExecutionResult HandleWaitingForNewFileName(string verb, string argLine)
        {
            _state = State.Ready;

            if (verb != "tobe")
            {
                return new CommandExecutionResult("-Expected TOBE; NAME aborted");
            }

            var (file, fileExists) = MakePath(argLine);
            if (file == null)
            {
                return new CommandExecutionResult("-Invalid file name");
            }

            if (fileExists)
            {
                return new CommandExecutionResult("-File already exists");
            }

            try
            {
                File.Move(_currentFile, file);
            }
            catch (IOException e)
            {
                return new CommandExecutionResult("-Cannot rename: " + e.Message);
            }

            return new CommandExecutionResult($"+Renamed to '{file}'");
        }

        /// <summary>
        /// Finds a file or directory at the specified path and returns its absolute path.
        /// The path may be either absolute (fully qualified) or relative, it is then resolved relatively to
        /// the current working directory.
        /// If the file or directory does not exist, null is returned.
        /// </summary>
        /// <remarks>
        /// This is used when looking for an existing file or directory (i.e. when handling RETR, CDIR or others).
        /// In contrast to <see cref="MakePath"/>, this method only supports both file and directory paths and it
        /// returns null to signalise that the specified path is not valid.
        /// Both these methods are, however, used to provide client with a 'virtual' current working directory,
        /// while still allowing them to use absolute paths as well.
        /// </remarks>
        /// <param name="path">The path to the file or directory that is being looked up.</param>
        /// <param name="isFile">When true, only files are considered. Otherwise, only directories are considered.</param>
        /// <returns>The absolute path of the file or directory, or null if it doesn't exist.</returns>
        private string LookupPath(string path, bool isFile)
        {
            if (Path.IsPathFullyQualified(path))
            {
                if (isFile)
                {
                    if (File.Exists(path)) return Path.GetFullPath(path);
                }
                else
                {
                    if (Directory.Exists(path)) return Path.GetFullPath(path);
                }

                return null;
            }

            var cwdPath = Path.Combine(_currentDirectory, path);
            if (isFile)
            {
                if (File.Exists(cwdPath)) return Path.GetFullPath(cwdPath);
            }
            else
            {
                if (Directory.Exists(cwdPath)) return Path.GetFullPath(cwdPath);
            }

            return null;
        }

        /// <summary>
        /// Checks whether the specified file exists and returns this information, as well as the absolute path
        /// of the file. The path may be either absolute (fully qualified) or relative, it is then resolved relatively
        /// to the current working directory.
        /// </summary>
        /// <remarks>
        /// This is used when creating a new file (when handling STOR). In contrast to <see cref="LookupPath"/>,
        /// this method only supports file paths and it always returns a path, even if the file does not exist.
        /// Both these methods are, however, used to provide client with a 'virtual' current working directory,
        /// while still allowing them to use absolute paths as well.
        /// </remarks>
        /// <param name="path">The path to the file that is being looked up.</param>
        /// <returns>A tuple of the absolute path of the file and a bool signalising whether it exists.</returns>
        private (string Path, bool Exists) MakePath(string path)
        {
            var file = Path.IsPathFullyQualified(path) ? path : Path.Combine(_currentDirectory, path);

            try
            {
                var fi = new FileInfo(file);
                return (file, fi.Exists);
            }
            catch
            {
                return (null, false);
            }
        }

        public void Dispose()
        {
            _currentStream?.Dispose();
            _currentStream = null;
        }
    }
}