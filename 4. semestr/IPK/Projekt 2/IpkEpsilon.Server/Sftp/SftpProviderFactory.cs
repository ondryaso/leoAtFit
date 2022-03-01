using IpkEpsilon.Server.Sftp.Abstractions;

namespace IpkEpsilon.Server.Sftp
{
    /// <summary>
    /// An implementation of <see cref="ISftpProviderFactory"/> that creates 
    /// </summary>
    public class SftpProviderFactory : ISftpProviderFactory
    {
        private readonly IAccountProvider _accountProvider;

        public SftpProviderFactory(IAccountProvider accountProvider)
        {
            _accountProvider = accountProvider;
        }

        public ISftpProvider CreateProvider()
        {
            return new SftpProvider(_accountProvider);
        }
    }
}