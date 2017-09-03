using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace GraphDebugger
{
    internal class Program
    {
        public static void Main(string[] args)
        {
            // X in [A; B]
            // Y in [C; D]
            // -> Y = (X - A) / (B - A) * (D - C) + C

            /*int[] mapping = new int[50];
            for (int i = 0; i < 50; i++)
                mapping[i] = int.MaxValue;*/

            Random rand = new Random();

            List<int> raw = new List<int>();
            for (int i = 1; i < 51; i++)
            {
                int r = rand.Next(99, 1000);
                while (raw.Contains(r))
                    r = rand.Next();

                raw.Add(r);
            }

            for (int i = 1; i < 51; i++)
            {
                int R = (1000 - 99) / (51 - 1);
                int x = raw[i - 1];
                int y = (x - 1) * R + 99;

                Console.WriteLine("{0} -> {1} ", x, y);
            }

            return;
        }
    }
}
