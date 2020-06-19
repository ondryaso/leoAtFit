using System;
using System.Collections;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading;

namespace ILG
{
    class Matrix
    {
        public virtual int Columns { get; }
        public virtual int Rows { get; }

        protected readonly decimal[] _values;

        public decimal this[int row, int column]
        {
            get => _values[row * Columns + column];

            set => _values[row * Columns + column] = value;
        }

        public Matrix(int columns, int rows)
        {
            Columns = columns;
            Rows = rows;
            _values = new decimal[columns * rows];
        }

        public Matrix(int columns, decimal[] values)
        {
            Columns = columns;
            Rows = values.Length / columns;
            _values = values;
        }

        public override string ToString()
        {
            var sb = new StringBuilder();

            for (int y = 0; y < Columns; y++)
            {
                for (int x = 0; x < Rows; x++)
                {
                    sb.Append(this[y, x]);
                    sb.Append('\t');
                }

                sb.AppendLine();
            }

            return sb.ToString();
        }
    }

    class Vector : Matrix
    {
        public override int Columns => 1;

        public decimal this[int y]
        {
            get => this[0, y];
            set => this[0, y] = value;
        }

        public Vector(int rows) : base(1, rows)
        {
        }

        public Vector(int rows, decimal[] values) : this(rows)
        {
            Array.Copy(values, _values, rows);
        }

        public Vector(Vector orig) : base(1, orig.Rows)
        {
            Array.Copy(orig._values, _values, orig.Rows);
        }

        public override string ToString()
        {
            var sb = new StringBuilder();
            sb.Append('[');
            for (int i = 0; i < Rows; i++)
            {
                sb.Append(this[i]);
                if (i != Rows - 1)
                {
                    sb.Append(", ");
                }
            }

            sb.Append(']');
            return sb.ToString();
        }
    }

    class Program
    {
        static void Main(string[] args)
        {
            Thread.CurrentThread.CurrentCulture = CultureInfo.InvariantCulture;

            Matrix input = new Matrix(3, new decimal[] {200, -3, 2, 1, -500, 2, 1, -3, 100});
            Vector inputVals = new Vector(3, new decimal[] {765, 987, 123});
            Vector startVals = new Vector(3, new decimal[] {3.5m, -2, 1});
            const decimal accuracy = 0.01m;

            Console.WriteLine("Input matrix:\n{0}", input);
            Console.WriteLine("Input result vector: {0}", inputVals);
            Console.WriteLine("Commencing calculations!\n");

            Console.WriteLine("Gauss-Seidel (with {0:e2} accuracy):", accuracy);
            var res = GaussSeidel(input, inputVals, accuracy, startVals);
            Console.WriteLine($"Result: {res}");
        }

static Vector GaussSeidel(Matrix system, Vector values, decimal accuracy = 0.01m, Vector startingValues = null,
    int maxCycles = 128, char startChar = 'x')
{
    int a = values.Rows;
    if (a != system.Rows) return null;

    // TODO: convergence
    int i = 0;

    Vector temp = startingValues ?? new Vector(system.Columns);
    if (temp.Rows != system.Columns) return null;
    
    while (i < maxCycles)
    {
        Vector prev = new Vector(temp);
        bool accurateEnough = true;

        for (int v = 0; v < temp.Rows; v++)
        {
            temp[v] = values[v];
            var line = "";
            
            line += $"{(char) (startChar + v)}_{{{i + 1}}} &= \\frac{{{temp[v]}";
            
            for (int c = 0; c < system.Columns; c++)
            {
                if (c == v) continue;
                var val = system[v, c] * temp[c];
                temp[v] -= val;
                
                line += $"{(val > 0 ? "-" : "+")}{Math.Abs(system[v, c])}\\cdot{Math.Abs(temp[c])}";
            }

            temp[v] /= system[v, v];

            line +=
                $"}}{{{system[v, v]}}} = {temp[v]} \\tag*{{$\\Delta {(char) (startChar + v)} = {Math.Abs(prev[v] - temp[v])}$}}\\\\\n";
            
            if(v == temp.Rows - 1) line += "\\\\\n";
            Console.Write(line.Replace(".", "{,}"));

            if (Math.Abs(prev[v] - temp[v]) > accuracy)
            {
                accurateEnough = false;
            }
        }

        if (accurateEnough) break;
        i++;
    }

    if (i == maxCycles) return null;
    return temp;
}
    }
}