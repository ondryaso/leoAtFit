using System;
using System.IO;

namespace IpkEpsilon.Common
{
    public static class AsciiStreamUtils
    {
        /// <summary>
        /// Reads the input stream byte-by-byte, converts LF line endings to CRLF, replaces lone CR characters
        /// with CR\0, replaces non-ASCII characters with a '?' and saves the result to a newly created
        /// MemoryStream that is then returned. Disposes the input stream.
        /// </summary>
        /// <remarks>
        /// This method is not particularly effective and blocks until the whole stream is processed. However,
        /// it is only used in the ASCII transfer mode that should never be used in the first place.
        /// </remarks>
        /// <param name="fs">The input <see cref="System.IO.Stream"/>.</param>
        /// <returns>A tuple of <see cref="System.IO.MemoryStream"/> instance that contains the converted data
        /// and the total number of bytes in it.</returns>
        public static (MemoryStream AsciiStream, long Length) LoadAsciiFile(Stream fs)
        {
            if (fs == null || !fs.CanRead) throw new ArgumentException();

            var asciiStream = new MemoryStream();

            var cr = new[] {(byte) '\r', (byte) '\n'};
            long totalLen = 0;

            byte prevB = 0;

            while (true)
            {
                var readByte = fs.ReadByte();
                if (readByte == -1) break;

                var b = (byte) readByte;
                if (b > 0x7F) b = (byte) '?';

                if (b == '\n')
                {
                    if (prevB != '\r')
                    {
                        asciiStream.Write(cr);
                        totalLen += 2;
                    }
                    else
                    {
                        asciiStream.WriteByte(b);
                        totalLen++;
                    }
                }
                else
                {
                    if (prevB == '\r')
                    {
                        asciiStream.WriteByte(0);
                        totalLen++;
                    }

                    asciiStream.WriteByte(b);
                    totalLen++;
                }

                prevB = b;
            }

            if (prevB == '\r')
            {
                asciiStream.WriteByte(0);
                totalLen++;
            }

            fs.Dispose();
            asciiStream.Position = 0;

            return (asciiStream, totalLen);
        }
    }
}