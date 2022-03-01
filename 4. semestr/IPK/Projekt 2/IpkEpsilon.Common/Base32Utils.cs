using System.Text;

namespace IpkEpsilon.Common
{
    public static class Base32Utils
    {
        /// <summary>
        /// Encodes the specified byte array to a Base32 string.
        /// </summary>
        /// <remarks>
        /// Source: https://stackoverflow.com/a/42231034
        /// </remarks>
        /// <param name="bytes">An array of bytes.</param>
        /// <returns>A Base32 string.</returns>
        public static string ToBase32String(byte[] bytes)
        {
            const string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
            var sb = new StringBuilder(bytes.Length * 8 / 5 + 1);

            for (var bitIndex = 0; bitIndex < bytes.Length * 8; bitIndex += 5)
            {
                var bytePair = bytes[bitIndex / 8] << 8;
                if (bitIndex / 8 + 1 < bytes.Length)
                    bytePair |= bytes[bitIndex / 8 + 1];
                bytePair = 0x1f & (bytePair >> (16 - bitIndex % 8 - 5));

                sb.Append(alphabet[bytePair]);
            }

            return sb.ToString();
        }
    }
}