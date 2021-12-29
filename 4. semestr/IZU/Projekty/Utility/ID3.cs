using System;

namespace ID3
{
    class Program
    {
        static void Main(string[] args)
        {
            var s = new Solver();
            s.Load("C:\\Users\\ondry\\id3-16.txt");
            s.Calculate();
            //Console.ReadKey();
        }
    }
}
