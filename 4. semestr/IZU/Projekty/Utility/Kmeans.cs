using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;

namespace IZU4
{
    class Program
    {
        public static void Main(string[] args)
        {
            var p = new Vector3[]
            {
                new(0, -1, -2), new(-3, -1, -3), new(1, -3, 2), new(-2, -2, 2), new(1, 2, -4), new(0, -4, 3),
                new(1, 0, -3), new(-3, 0, 0), new(-2, 2, -4), new(-2, 4, 3), new(3, -2, 4), new(2, -5, -4)
            };

            var w = new Vector3[] {new(-3, 4, 1), new(-3, -1, -3), new(0, -1, -6)};
            var lastMinW = new int[p.Length];
            var s = 0;
            var sb = new StringBuilder();

            while (true)
            {
                var wStrs = new string[w.Length];
                Console.WriteLine($"\\subsection*{{Krok {++s}}}");
                Console.WriteLine(@"\begin{align*}");
                for (var wi = 0; wi < w.Length; wi++)
                {
                    Console.WriteLine($"    w_{wi + 1} &= [{w[wi].X:0.##}, {w[wi].Y:0.##}, {w[wi].Z:0.##}] \\\\");
                }

                Console.WriteLine(@"\end{align*}");


                // Pro každý bod spočítám vzdálenosti od W a vyberu nejmenší
                var currentMinW = new int[p.Length];
                for (var pi = 0; pi < p.Length; pi++)
                {
                    var minDist = float.MaxValue;
                    var minW = -1;

                    for (var wi = 0; wi < w.Length; wi++)
                    {
                        var distance = Vector3.Distance(w[wi], p[pi]);
                        if (distance < minDist)
                        {
                            minDist = distance;
                            minW = wi;
                        }

                        currentMinW[pi] = minW;
                    }

                    for (var wi = 0; wi < w.Length; wi++)
                    {
                        // HAhaa
                        var distance = Vector3.Distance(w[wi], p[pi]);

                        if (wi % 2 == 0)
                        {
                            sb.Append($"        p_{{{pi + 1}}}: \\sqrt{{");
                        }
                        else
                        {
                            sb.Append(@"        \sqrt{");
                        }

                        sb.Append($"({w[wi].X:0.##}");
                        sb.Append($"{-p[pi].X:+#.##;-#.##;-0})^2+");

                        sb.Append($"({w[wi].Y:0.##}");
                        sb.Append($"{-p[pi].Y:+#.##;-#.##;-0})^2+");

                        sb.Append($"({w[wi].Z:0.##}");
                        sb.Append($"{-p[pi].Z:+#.##;-#.##;-0})^2}} &= ");

                        if (wi == currentMinW[pi])
                        {
                            sb.Append(@"\mathbf{");
                            sb.Append($"{distance:0.##}}}\\\\\n");
                        }
                        else
                        {
                            sb.Append($"{distance:0.##}\\\\\n");
                        }

                        wStrs[wi] += sb.ToString();
                        sb.Clear();
                    }
                }

                // Zkontroluju, jestli se nějakému bodu změnila příslušnost k vektoru
                var changed = false;
                for (var pi = 0; pi < w.Length; pi++)
                {
                    if (lastMinW[pi] != currentMinW[pi])
                    {
                        // Nějaký bod změnil příslušnost
                        changed = true;
                        lastMinW = currentMinW;
                        break;
                    }
                }

                if (changed)
                {
                    // Pokud ano, přepočítám vektory
                    var wSums = new Vector3[w.Length];
                    var wCounts = new int[w.Length];

                    for (var pi = 0; pi < p.Length; pi++)
                    {
                        wSums[currentMinW[pi]] += p[pi];
                        wCounts[currentMinW[pi]]++;
                    }

                    for (var wi = 0; wi < wSums.Length; wi++)
                    {
                        wSums[wi] /= wCounts[wi];
                    }

                    w = wSums;
                }

                for (var wi = 0; wi < w.Length; wi += 2)
                {
                    Console.WriteLine(@"\begin{paracol}{2}");
                    WriteColumn(wi, wStrs);

                    if ((wi + 1) < w.Length)
                    {
                        Console.WriteLine(@"\switchcolumn");
                        WriteColumn(wi + 1, wStrs);
                    }
                    else
                    {
                        Console.WriteLine("\\switchcolumn\n\\textbf{Nové zařazení bodů do clusterů:}\\\\\n    \\[");
                        Console.Write("        (");
                        Console.Write(string.Join(", ", currentMinW.Select(v => v + 1)));
                        Console.WriteLine(")\n    \\]");
                    }

                    //Console.WriteLine("\\colplacechunks\n\\end{parcolumns}\n\\vspace{0.7cm}");

                    Console.WriteLine("\\end{paracol}\n\\vspace{0.7cm}");
                }

                if (!changed)
                {
                    return;
                }
            }
        }

        static void WriteColumn(int i, string[] wStr)
        {
            Console.WriteLine($"    \\textbf{{Vzdálenosti od $w_{i + 1}$:}}");
            Console.WriteLine(@"    \begin{align*}");
            Console.WriteLine(wStr[i][..^3]);
            Console.WriteLine(@"    \end{align*}");
        }
    }
}