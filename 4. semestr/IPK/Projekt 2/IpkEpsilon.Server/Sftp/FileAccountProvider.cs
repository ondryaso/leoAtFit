using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using IpkEpsilon.Common;
using IpkEpsilon.Server.Sftp.Abstractions;

namespace IpkEpsilon.Server.Sftp
{
    public class FileAccountProvider : IAccountProvider
    {
        private Dictionary<string, string> _users = null;

        /// <summary>
        /// Loads a user database from the specified text file.
        /// </summary>
        /// <remarks>
        /// The database text file consists of records in form of "username:password", followed by a newline character.
        /// Whitespaces at the beginning and at the end of a line are omitted.
        /// Each line of the file contains exactly one user/password combination.
        /// Each line must contain exactly one occurrence of the colon separator.
        /// The file must be encoded in ASCII.
        /// </remarks>
        /// <param name="file">The path to the file with the user database to load.</param>
        /// <exception cref="FileNotFoundException">The specified file does not exist.</exception>
        public void LoadUserDatabase(string file)
        {
            if (!File.Exists(file))
            {
                throw new FileNotFoundException("The specified user database file does not exist.", file);
            }

            using var sr = new StreamReader(file, Encoding.ASCII);
            _users = new Dictionary<string, string>();

            var lc = 0;
            while (!sr.EndOfStream)
            {
                lc++;
                var line = sr.ReadLine();
                if (string.IsNullOrWhiteSpace(line)) continue;
                line = line.Trim();

                if (!line.Contains(':') || line.IndexOf(':') != line.LastIndexOf(':'))
                {
                    throw new FormatException($"Invalid user record on line {lc}.");
                }

                var parts = line.Split(':');
                if (string.IsNullOrWhiteSpace(parts[0]) || string.IsNullOrWhiteSpace(parts[1]))
                {
                    throw new FormatException($"Invalid user record on line {lc}.");
                }

                _users.Add(parts[0], parts[1]);
            }

            Log.Info(_users.Count == 1 ? "Loaded 1 user" : $"Loaded {_users.Count} users");
        }

        /// <inheritdoc cref="IAccountProvider.HasUser"/>
        /// <remarks>
        /// The user database must be loaded first using <see cref="LoadUserDatabase"/>.
        /// Usernames are case-sensitive.
        /// </remarks>
        /// <exception cref="InvalidOperationException">The user database has not been loaded yet.</exception>
        public bool HasUser(string username)
        {
            if (_users == null)
            {
                throw new InvalidOperationException("The user database has not been loaded yet.");
            }

            return _users.ContainsKey(username);
        }

        /// <inheritdoc cref="IAccountProvider.AuthenticateUser"/>
        /// <remarks>
        /// The user database must be loaded first using <see cref="LoadUserDatabase"/>.
        /// Usernames and passwords are case-sensitive.
        /// </remarks>
        /// <exception cref="InvalidOperationException">The user database has not been loaded yet.</exception>
        public bool AuthenticateUser(string username, string password)
        {
            if (_users == null)
            {
                throw new InvalidOperationException("The user database has not been loaded yet.");
            }

            return _users.ContainsKey(username) && _users[username] == password;
        }
    }
}