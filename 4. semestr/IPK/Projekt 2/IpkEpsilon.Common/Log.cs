using System;

namespace IpkEpsilon.Common
{
    public static class Log
    {
        public static bool AppendTime { get; set; } = false;

        public static void Info(string msg)
            => Console.WriteLine(AppendTime
                ? $"[{DateTime.Now:MM-dd HH:mm:ss}][INFO] {msg}"
                : $"[INFO] {msg}");

        public static void Warning(string msg)
            => Console.Error.WriteLine(AppendTime
                ? $"[{DateTime.Now:MM-dd HH:mm:ss}][WARN] {msg}"
                : $"[WARN] {msg}");

        public static void Error(string msg)
            => Console.Error.WriteLine(AppendTime
                ? $"[{DateTime.Now:MM-dd HH:mm:ss}][ERR ] {msg}"
                : $"[ERR] {msg}");
    }
}