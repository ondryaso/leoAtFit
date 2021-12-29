using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace ID3
{
    public record Attribute(string Name, string[] Values);

    public record Object(int Id, Dictionary<string, string> Values, string Class);

    public class Solver
    {
        private List<Attribute> _attributes;
        private List<string> _classes;
        private List<Object> _objects;

        public void Load(string file)
        {
            using var sr = new StreamReader(file, Encoding.UTF8);
            var state = 4; // 0 - attribs, 1 - classes, 2 - objects, 3 - questions, 4 - none 

            _attributes = new();
            _classes = new();
            _objects = new();

            while (!sr.EndOfStream)
            {
                var line = sr.ReadLine();
                if (string.IsNullOrWhiteSpace(line)) continue;

                if (state != 4 && line.Contains('}'))
                {
                    state = 4;
                    continue;
                }

                switch (state)
                {
                    case 0:
                        var parts = line.Split(':');
                        var name = parts[0].Trim();
                        var values = parts[1].Trim().Split(' ');
                        _attributes.Add(new Attribute(name, values));
                        break;
                    case 1:
                        _classes.Add(line.Trim());
                        break;
                    case 2:
                        parts = line.Trim().Split(' ');
                        var id = int.Parse(parts[0]);
                        var cls = parts[1];
                        if (parts.Length - 2 != _attributes.Count)
                        {
                            Console.WriteLine("Error: unknown attributes");
                            return;
                        }

                        var vals = new Dictionary<string, string>();
                        for (var i = 0; i < _attributes.Count; i++)
                        {
                            vals.Add(_attributes[i].Name, parts[i + 2]);
                        }

                        _objects.Add(new Object(id, vals, cls));
                        break;
                    case 3:
                        break;
                    case 4:
                        if (line.Contains('{'))
                        {
                            if (line.Contains("attributes")) state = 0;
                            else if (line.Contains("classes")) state = 1;
                            else if (line.Contains("objects")) state = 2;
                            else if (line.Contains("questions")) state = 3;
                        }

                        break;
                }
            }
        }

        private static void Spacing(int r)
        {
            for(var i = 0; i < r*2; i++) Console.Write(' ');
        }
        
        public void Calculate(string name = "Table", List<Object> objects = null, int rec = 0)
        {
            objects ??= _objects;
            var tableEntropy = CalculateEntropy(objects);
            if (tableEntropy == 0)
            {
                Spacing(rec);
                Console.WriteLine($"]] '{name}' entropy zero, finished: {objects?.FirstOrDefault()?.Class}.");
                Spacing(rec);
                Console.WriteLine("]] " + string.Join(", ", objects.Select(o => o.Id)));
                return;
            }
            else
            {
                Spacing(rec);
                Console.WriteLine($"]] '{name}' entropy: " + tableEntropy);
                Spacing(rec);
                Console.WriteLine("]] " + string.Join(", ", objects.Select(o => o.Id)));
            }

            var maxGain = 0M;
            Attribute maxBranch = null;

            foreach (var attribute in _attributes)
            {
                if (!objects[0].Values.ContainsKey(attribute.Name)) continue;
                
                var e = CalculateEntropyForAttribute(objects, attribute.Name);
                var gain = tableEntropy - e;
                Spacing(rec);
                Console.WriteLine($"E({attribute.Name}) = {e}");
                Spacing(rec);
                Console.WriteLine($"  gain = {gain}");

                if (gain > maxGain)
                {
                    maxGain = gain;
                    maxBranch = attribute;
                }
            }

            if (maxGain == 0)
            {
                Spacing(rec);
                Console.WriteLine("All gains are zero.");
                return;
            }
            else
            {
                Spacing(rec);
                Console.WriteLine("Maximum gain from: " + maxBranch.Name);
            }
            
            foreach (var attrValue in maxBranch.Values)
            {
                var o = objects.Where(obj => obj.Values[maxBranch.Name] == attrValue)
                    .Select(obj =>
                    {
                        var d = new Dictionary<string, string>(obj.Values);
                        d.Remove(maxBranch.Name);
                        return new Object(obj.Id, d, obj.Class);
                    })
                    .ToList();
                
                
                Console.WriteLine();
                Calculate(attrValue, o, rec + 1);
            }
        }

        private decimal CalculateEntropy(List<Object> objects)
        {
            decimal res = 0;
            decimal total = objects.Count;

            var groups = objects.GroupBy(o => o.Class);
            foreach (var g in groups)
            {
                var prob = decimal.Divide(g.Count(), total);
                //Console.WriteLine($"p({g.Key}) = {prob}");
                var log = (decimal) Math.Log2(decimal.ToDouble(prob));
                res += decimal.Multiply(prob, log);
            }

            //Console.WriteLine(res);
            return -res;
        }

        private decimal CalculateEntropyForAttribute(List<Object> objects, string attr)
        {
            var attribute = _attributes.First(a => a.Name == attr);
            decimal res = 0;

            foreach (var attrVal in attribute.Values)
            {
                var boundObjects = objects.Where(o => o.Values[attr] == attrVal)
                    .ToList();
                var entropy = CalculateEntropy(boundObjects);

                var coef = decimal.Divide(boundObjects.Count, objects.Count);
                res += decimal.Multiply(coef, entropy);

                //Console.WriteLine($"E({attr}_{attrVal}) = {entropy}");
            }

            return res;
        }
    }
}