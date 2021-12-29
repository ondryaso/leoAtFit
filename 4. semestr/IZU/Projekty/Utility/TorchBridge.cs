using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Priority_Queue;

namespace Test
{
    public record State(bool TorchOnLeft, List<char> PeopleLeft, List<char> PeopleRight, State PreviousState,
        Rule GotByRule)
    {
        public virtual bool Equals(State other)
        {
            if (ReferenceEquals(null, other)) return false;
            if (ReferenceEquals(this, other)) return true;

            return TorchOnLeft == other.TorchOnLeft
                   && !PeopleLeft.Except(other.PeopleLeft).Any()
                   && !other.PeopleLeft.Except(PeopleLeft).Any()
                   && !PeopleRight.Except(other.PeopleRight).Any()
                   && !other.PeopleRight.Except(PeopleRight).Any();
        }

        public override int GetHashCode()
        {
            var pl = string.Join(null, PeopleLeft);
            var pr = string.Join(null, PeopleRight);
            return HashCode.Combine(TorchOnLeft, pl, pr);
        }

        public string ToString(int time)
        {
            var sb = new StringBuilder("[");
            if (TorchOnLeft) sb.Append('*');
            foreach (var c in PeopleLeft)
            {
                sb.Append(c);
            }

            sb.Append(time);
            foreach (var c in PeopleRight)
            {
                sb.Append(c);
            }

            if (!TorchOnLeft) sb.Append('*');
            sb.Append(']');
            return sb.ToString();
        }

        public State ApplyRule(Rule rule)
        {
            if (TorchOnLeft && !rule.LeftToRight) return null;
            if (!TorchOnLeft && rule.LeftToRight) return null;
            
            if (rule.People.TrueForAll(p => PeopleLeft.Contains(p)) && TorchOnLeft)
            {
                var l = new List<char>(PeopleLeft);
                l.RemoveAll(p => rule.People.Contains(p));
                l.Sort();
                var r = PeopleRight.Union(rule.People).ToList();
                r.Sort();
                return new State(false, l, r, this, rule);
            }

            if (rule.People.TrueForAll(p => PeopleRight.Contains(p)) && !TorchOnLeft)
            {
                var r = new List<char>(PeopleRight);
                r.RemoveAll(p => rule.People.Contains(p));
                r.Sort();
                var l = PeopleLeft.Union(rule.People).ToList();
                l.Sort();
                return new State(true, l, r, this, rule);
            }

            return null;
        }
    }

    public record Rule(bool LeftToRight, List<char> People, int Time);

    public class Izu1Test
    {
        public static void MainIzu(string[] args)
        {
            var i = new Izu1Test();
            i.Calculate();
        }

        private List<Rule> _rules =
            new()
            {
                new Rule(false, new() {'A'}, 1),
                new Rule(false, new() {'B'}, 3),
                new Rule(false, new() {'C'}, 6),
                new Rule(false, new() {'D'}, 8),
                new Rule(true, new() {'A', 'B'}, 3),
                new Rule(true, new() {'A', 'C'}, 6),
                new Rule(true, new() {'A', 'D'}, 8),
                new Rule(true, new() {'B', 'C'}, 6),
                new Rule(true, new() {'B', 'D'}, 8),
                new Rule(true, new() {'C', 'D'}, 8),
            };

        public void Calculate()
        {
            var init = new State(true, new List<char> {'A', 'B', 'C', 'D'},
                new List<char>(), null, null);

            var open = new SimplePriorityQueue<State, int>();
            var openList = new List<(State, int)>();
            var closed = new List<(State, int)>();

            open.Enqueue(init, 0);
            var cntr = 0;
            while (open.Count != 0)
            {
                Console.Write("Open:\t");
                var outCntr = 1;
                foreach (var state in openList)
                {
                    Console.Write(state.Item1.ToString(state.Item2) + "\t");
                    if (++outCntr % 9 == 0) Console.WriteLine();
                }

                while (outCntr != 27)
                {
                    Console.Write("\t ");
                    if (++outCntr % 9 == 0) Console.WriteLine();
                }
                /*
                Console.Write("\nClosed: ");
                foreach (var state in closed)
                {
                    Console.Write(state.Item1.ToString(state.Item2) + "\t");
                }*/
                Console.WriteLine("\n");
                
                var node = open.First;
                var currentTime = open.GetPriority(node);
                open.Dequeue();
                openList.Remove((node, currentTime));

                cntr++;
                if (!node.TorchOnLeft && node.PeopleRight.Count == 4 && node.PeopleLeft.Count == 0)
                {
                    var st = new Stack<State>();
                    var previous = node.PreviousState;
                    while (previous != null)
                    {
                        st.Push(previous);
                        previous = previous.PreviousState;
                    }

                    var time = 0;
                    while (st.TryPop(out var s))
                    {
                        if (s.GotByRule != null) time += s.GotByRule.Time;
                        Console.WriteLine(s.ToString(time));
                    }

                    Console.WriteLine("Result: " + node.ToString(currentTime));
                    Console.WriteLine(cntr);
                    Console.WriteLine();
                    return;
                }

                closed.Add((node, currentTime));

                foreach (var rule in _rules)
                {
                    var newState = node.ApplyRule(rule);
                    if (newState == null) continue;

                    var newTime = currentTime + rule.Time;

                    if (closed.Any(c=> c.Item1 == newState))
                        continue;
                    
                    if (open.Contains(newState))
                    {
                        var prio = open.GetPriority(newState);
                        if (prio > newTime)
                        {
                            open.Remove(newState);
                            open.Enqueue(newState, newTime);
                            //open.UpdatePriority(newState, newTime);

                            var listIndex = openList.IndexOf((newState, prio));
                            openList.RemoveAt(listIndex);
                            openList.Insert(listIndex, (newState, newTime));
                        }
                    }
                    else
                    {
                        open.Enqueue(newState, newTime);
                        openList.Add((newState, newTime));
                    }
                }
            }

            Console.WriteLine("Failed");
        }
    }
}