namespace IpkEpsilon.Server.Sftp.Abstractions
{
    /// <summary>
    /// Provides a storage of user accounts that are represented by pairs of username and password credentials.
    /// Provides interfaces for checking whether a user exists and whether a certain username/password combination is valid.
    /// </summary>
    public interface IAccountProvider
    {
        /// <summary>
        /// Checks whether a user with the specified username exists.
        /// </summary>
        /// <remarks>
        /// Usernames are case-sensitive.
        /// </remarks>
        /// <param name="username">The username to check.</param>
        /// <returns>True if a user with the specified username exists.</returns>
        public bool HasUser(string username);

        /// <summary>
        /// Checks whether a user with the specified username exists and if their password matches the specified value.
        /// </summary>
        /// <remarks>
        /// Usernames and passwords are case-sensitive.
        /// </remarks>
        /// <param name="username">The username to check.</param>
        /// <param name="password">The password to check.</param>
        /// <returns>True if the provided pair of username and password is valid.</returns>
        public bool AuthenticateUser(string username, string password);
    }
}