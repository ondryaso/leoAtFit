namespace IpkEpsilon.Server.Sftp.Abstractions
{
    public interface ISftpProviderFactory
    {
        /// <summary>
        /// Creates a new <see cref="SftpProvider"/> instance.
        /// </summary>
        /// <returns>An SftpProvider instance.</returns>
        public ISftpProvider CreateProvider();
    }
}