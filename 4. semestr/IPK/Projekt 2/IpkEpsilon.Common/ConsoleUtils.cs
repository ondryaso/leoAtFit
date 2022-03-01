using System;
using System.Threading;
using System.Threading.Tasks;

namespace IpkEpsilon.Common
{
    public static class ConsoleUtils
    {
        /// <summary>
        /// Presents the user with a question and asks them to press Y(es) or N(o).
        /// Repeats until Y, N or Enter is pressed.
        /// </summary>
        /// <param name="message">The question to write to the standard output before waiting for an answer.</param>
        /// <param name="preferYes">If true, Y(es) is the default answer and pressing Enter returns true.</param>
        /// <returns>True if the user responded with a Y(es), false if they responded with a N(o).</returns>
        public static bool ReadYesNo(string message, bool preferYes = true)
        {
            Console.WriteLine($"{message} Continue? {(preferYes ? "[Y/n]" : "[y/N]")}");

            var key = Console.ReadKey().Key;
            while (key != ConsoleKey.Y && key != ConsoleKey.N && key != ConsoleKey.Enter)
            {
                Console.WriteLine(preferYes ? "[Y/n]?" : "[y/N]?");
                key = Console.ReadKey().Key;
            }

            return (key == ConsoleKey.Enter && preferYes) || key == ConsoleKey.Y;
        }

        public static string ReadLine(CancellationToken cancellationToken)
        {
            var readTask = Task.Factory.StartNew(Console.ReadLine, cancellationToken);
            
            try
            {
                Task.WaitAny(new Task[] {readTask}, cancellationToken);
                return readTask.Result;
            }
            catch (OperationCanceledException)
            {
                return null;
            }
        }
    }
}