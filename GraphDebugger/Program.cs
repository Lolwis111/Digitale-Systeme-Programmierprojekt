using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Diagnostics;

namespace GraphDebugger
{
    internal class Program
    {
        public static void Main(string[] args)
        {
            for (uint i = 0; i < 3999999999UL; i++)
            {
                Process process = Process.Start("DS_WIn.exe");

                

                StreamWriter writer = process.StandardInput;

                writer.WriteLine("{0} {0} {0}", i);
                writer.Write("{0}", i);
                writer.Flush();
                writer.Close();

                process.WaitForExit();

                StreamReader reader = process.StandardOutput;

                if (uint.TryParse(reader.ReadLine(), out uint result))
                {
                    if (result != i) Console.WriteLine("calculated wrongly!");
                }
                else
                {
                    Console.WriteLine("Number out of range {0}", i);
                }
            }
        }
    }
}