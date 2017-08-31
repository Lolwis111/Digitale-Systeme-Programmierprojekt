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
            if (!File.Exists(args[0]))
            {
                Console.Error.WriteLine("Input file not found!");

                return;
            }

            StreamReader reader = null;
            StreamWriter writer = null;
            try
            {
                reader = new StreamReader(File.Open(args[0], FileMode.Open));

                List<string> data = new List<string>();
                string saveHouses = null;
                string firstLine = null;

                firstLine = reader.ReadLine();

                while (!reader.EndOfStream)
                {
                    string line = reader.ReadLine();

                    if (line.Contains(' '))
                    {
                        data.Add(reader.ReadLine());
                    }
                    else
                    {
                        saveHouses = line + reader.ReadToEnd();
                    }
                }

                writer = new StreamWriter(File.Open(args[1], FileMode.Create));

                writer.WriteLine(firstLine);

                while (data.Any())
                {
                    writer.WriteLine(data.Last());
                    data.RemoveAt(data.Count - 1);
                }

                writer.WriteLine(saveHouses);
            }
            catch(Exception ex)
            {
                Console.Error.WriteLine(ex.Message);
            }
            finally
            {
                reader?.Dispose();
                reader = null;
                writer?.Dispose();
                writer = null;
            }
        }
    }
}
