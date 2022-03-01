using System;

namespace IpkEpsilon.Server.Sftp.Abstractions
{
    /// <summary>
    /// Provides a stateful mechanism for handling SFTP commands.  
    /// </summary>
    public interface ISftpProvider : IDisposable
    {
        /// <summary>
        /// Parses and executes a SFTP protocol command and returns a <see cref="CommandExecutionResult"/> which specifies
        /// the response action for this command. 
        /// </summary>
        /// <param name="command">The command to execute.</param>
        /// <returns>An object describing a response action.</returns>
        public CommandExecutionResult HandleCommand(string command);

        /// <summary>
        /// Generates a message that should be sent as a response to the client establishing connection.
        /// </summary>
        /// <param name="identifier">A string that contains an optional client identifier that the client should be informed of.</param>
        /// <returns>An object describing a response action.</returns>
        public CommandExecutionResult MakeGreeting(string identifier = null);

        /// <summary>
        /// Notifies this SFTP handler that an incoming file transfer has been finished.
        /// </summary>
        public void NotifyFileReceived();
    }
}