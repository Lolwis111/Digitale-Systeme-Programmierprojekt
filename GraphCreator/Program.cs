using System;
using System.Collections.Generic;
using System.IO;

namespace GraphCreator
{
    public class Program
    {
        const int limit = 200;
        public static void Main(string[] args)
        {
            Random rand = new Random();
            
            Console.WriteLine($"0 {limit} 3999999999");

            for (int i = 0; i < limit; i++)
            {
                if(i == ((limit / 2) + 10)) Console.WriteLine("{0} {1} 1", i + 1, i);
                else Console.WriteLine("{0} {1} 1", i, i + 1);
            }

            Console.WriteLine($"{limit/2}");
        }
    }
}
