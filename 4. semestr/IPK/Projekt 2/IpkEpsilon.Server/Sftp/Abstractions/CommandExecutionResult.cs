using System.IO;

namespace IpkEpsilon.Server.Sftp.Abstractions
{
    /// <summary>
    /// Represents a response to an executed SFTP command. Every command leads to a string message being sent
    /// back to the client or to a byte stream operation. 
    /// </summary>
    public class CommandExecutionResult
    {
        internal static readonly CommandExecutionResult InvalidCommand =
            new("-Invalid or unexpected command");

        /// <summary>
        /// The message to send to the client. This is only relevant when <see cref="ResponseMode"/>
        /// is set to <see cref="Abstractions.ResponseMode.SendString"/>.
        /// </summary>
        public string ResponseMessage { get; }

        /// <summary>
        /// The stream to read outgoing file data from, or to save incoming file data to.
        /// This is only relevant when <see cref="ResponseMode"/> is set to
        /// <see cref="Abstractions.ResponseMode.SendStream"/> or when <see cref="NextMode"/> is set to
        /// <see cref="Abstractions.NextMode.ReceiveBytes"/>.
        /// </summary>
        public Stream Stream { get; }

        /// <summary>
        /// Specifies whether a string message or a file data stream should be sent to the client. 
        /// </summary>
        public ResponseMode ResponseMode { get; }

        /// <summary>
        /// Specifies what kind of data from the client should be expected after sending a response to the last command. 
        /// </summary>
        public NextMode NextMode { get; }

        /// <summary>
        /// Specifies the amount of bytes to expect from the client when receiving a file.
        /// This is only relevant when <see cref="NextMode"/> is set to <see cref="Abstractions.NextMode.ReceiveBytes"/>.
        /// </summary>
        public long BytesToFetch { get; }

        public CommandExecutionResult(string responseMessage, Stream responseStream, ResponseMode responseMode,
            NextMode nextMode, long bytesToFetch = 0)
        {
            ResponseMessage = responseMessage;
            Stream = responseStream;
            ResponseMode = responseMode;
            NextMode = nextMode;
            BytesToFetch = bytesToFetch;
        }

        public CommandExecutionResult(string responseMessage)
            : this(responseMessage, null, ResponseMode.SendString, NextMode.ReadCommand)
        {
        }
    }
}