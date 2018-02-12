using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Text.RegularExpressions;

namespace difftool
{
    class Program
    {
        static private List<string> GetDiff(string[] s1, string[] s2)
        {
            List<string> ret = new List<string>();

            int i2 = 0;
            for (int i1 = 0; i1 < s1.Length; ++i1)
            {
                if (i2 >= s2.Length)
                {
                    ret.Add(s1[i1]);
                    continue;
                }
                if (s1[i1] == s2[i2])
                {
                    ++i2;
                    continue;
                }
                for (int i3 = i2 + 1; i3 < i2 + 300 && i3 < s2.Length; ++i3)
                {
                    if (s1[i1] == s2[i3])
                    {
                        i2 = i3;
                        break;
                    }
                }
                if (s1[i1] == s2[i2])
                {
                    ++i2;
                    continue;
                }
                ret.Add(s1[i1]);
            }
            return ret;
        }

        static private string textToHtml(string s) {
            return s.Replace("&", "&amp;").Replace("<", "&lt;").Replace(">", "&gt;");
        }

        static Boolean isNameChar(char c) {
            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
        }

        static private void writeSourceFile(string path, string w, string filename) {
            string errorLoc = w.Substring(0, w.IndexOf(']'));
            string f = errorLoc.Substring(1, errorLoc.LastIndexOf(':')-1);
            int errorLine = Int32.Parse(errorLoc.Substring(1 + errorLoc.LastIndexOf(':')));
            string[] lines = System.IO.File.ReadAllLines(path + f);
            string[] keywords = { "void", "bool", "char", "short", "int", "long", "class", "struct", "enum", "for", "if", "while", "using", "namespace", "unsigned", "signed" };
            System.IO.StreamWriter file = new System.IO.StreamWriter(path + filename);
            file.Write("<html><body><pre>" + textToHtml(w) + "\n");
            int linenr = 1;
            foreach (string line in lines) {
                if (errorLine == linenr)
                {
                    file.Write("<span style=\"background-color: red\">" + (linenr.ToString() + ":       ").Substring(0, 8) + textToHtml(line) + "</span>\n");
                    file.Write("<span style=\"background-color: red\">" + textToHtml(w.Substring(errorLoc.Length+2)) + "</span>\n");
                    linenr++;
                    continue;
                }

                file.Write((linenr.ToString() + ":       ").Substring(0, 8));
                linenr++;

                for (int i = 0; i < line.Length;) {
                    if (line[i] == ' ')
                    {
                        file.Write(line[i]);
                        i++;
                    }
                    else if (isNameChar(line[i]))
                    {
                        string currentToken = "";
                        while (i < line.Length && isNameChar(line[i]))
                        {
                            currentToken += line[i];
                            i++;
                        }
                        if (keywords.Contains(currentToken))
                            file.Write("<b>" + currentToken + "</b>");
                        else if (currentToken[0] >= '0' && currentToken[0] <= '9')
                            file.Write("<font color=\"blue\">" + currentToken + "</font>");
                        else
                            file.Write(currentToken);
                    }
                    else if (line[i] == '\"' || line[i] == '\'')
                    {
                        string currentToken = "";
                        while (i < line.Length)
                        {
                            currentToken += line[i];
                            i++;
                            if (currentToken.Length > 1 && currentToken[0] == line[i - 1])
                                break;
                        }
                        file.Write("<font color=\"red\">" + textToHtml(currentToken) + "</font>");
                    }
                    else if (line[i] == '/' && i + 1 < line.Length && line[i + 1] == '/')
                    {
                        file.Write("<font color=\"gray\">" + textToHtml(line.Substring(i)) + "</font>");
                        i = line.Length;
                    }
                    else
                    {
                        file.Write(textToHtml(line.Substring(i,1)));
                        i++;
                    }
                }
                file.Write('\n');
            }
            file.Write("</pre></body></html>");
            file.Close();
        }

        static void Main(string[] args)
        {
            string path;
            if (Directory.Exists("/home/danielm/shared/testing"))
                path = "/home/danielm/shared/testing/";
            else if (Directory.Exists(@"c:\Users\danielm.evidente\testing"))
                path = @"c:\Users\danielm.evidente\testing\";
            else
                return;

            string[] old = System.IO.File.ReadAllLines(path + "1.82.txt");
            string[] head = System.IO.File.ReadAllLines(path + "head.txt");

            Array.Sort(old, StringComparer.InvariantCulture);
            Array.Sort(head, StringComparer.InvariantCulture);

            List<string> removed = GetDiff(old, head);
            List<string> added = GetDiff(head, old);

            int fileIndex = 1;

            using (System.IO.StreamWriter file = new System.IO.StreamWriter(path + "main.html"))
            {
                file.WriteLine("<html>");
                file.WriteLine("<head>");
                file.WriteLine("<title>Comparison 1.82 / Head</title>");
                file.WriteLine("</head>");
                file.WriteLine("<body>");

                file.WriteLine("<h1>Comparison 1.82 / Head</h1>");
                file.WriteLine("<p>Package: {0}</p>", @"ftp://ftp.se.debian.org/debian/pool/main/f/firefox/firefox_59.0~b4.orig.tar.bz2");
                file.WriteLine("<p>+ Added warnings {0}</p>", added.Count);
                file.WriteLine("<p>- Removed warnings {0}</p>", removed.Count);

                file.WriteLine("<h2>Added warnings</h2>");
                file.WriteLine("<pre>");
                foreach (string w in added)
                {
                    string filename = "code" + fileIndex.ToString() + ".html";
                    fileIndex++;
                    writeSourceFile(path, w, filename);
                    file.WriteLine("<a href=\"{0}\">{1}</a>", filename, textToHtml(w));
                }
                file.WriteLine("</pre>");

                file.WriteLine("<h2>Removed warnings</h2>");
                file.WriteLine("<pre>");
                foreach (string w in removed)
                {
                    string filename = "code" + fileIndex.ToString() + ".html";
                    fileIndex++;
                    writeSourceFile(path, w, filename);
                    file.WriteLine("<a href=\"{0}\">{1}</a>", filename, textToHtml(w));
                }
                file.WriteLine("</pre>");

                file.WriteLine("</body>");
                file.WriteLine("</html>");
            }
        }
    }
}
