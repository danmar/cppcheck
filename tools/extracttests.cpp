

#include <iostream>
#include <fstream>
#include <sstream>

const std::string ext(".cpp");

static std::string str(unsigned int value)
{
    std::ostringstream ostr;
    ostr << value;
    return ostr.str();
}

int main(const int argc, const char * const * const argv)
{
    if (argc != 2)
    {
        std::cerr << "syntax: extracttests testfile" << std::endl;
        return 0;
    }

    std::string testname;
    unsigned int subcount = 0;

    std::ifstream f(argv[1]);
    std::string line;
    while (std::getline(f, line))
    {
        if (line.compare(0, 9, "    void ") == 0)
        {
            testname = line.substr(9, line.size() - 11);
            std::cout << "\"" << testname << "\"" << std::endl;
            subcount = 0;
            continue;
        }

        if (line.find("}") != std::string::npos)
        {
            testname = "";
            subcount = 0;
            continue;
        }

        if (!testname.empty() && line.find(" check(\"") != std::string::npos)
        {
            std::ofstream fout((testname + str(++subcount) + ext).c_str());
            fout << "#include <string.h>" << std::endl;
            fout << "#include <stdio.h>" << std::endl;
            fout << "#include <stdlib.h>" << std::endl;
            do
            {
                std::string::size_type pos = line.find("\"");
                if (pos == std::string::npos)
                    break;
                line.erase(0, pos + 1);

                pos = line.rfind("\"");
                if (pos == std::string::npos)
                    break;
                const bool lastline(line.find(");", pos) != std::string::npos);
                line.erase(pos);

                pos = 0;
                while ((pos = line.find("\\", pos)) != std::string::npos)
                {
                    line.erase(pos, 1);
                    if (line[pos] == 'n')
                        line.erase(pos, 1);
                    else
                        ++pos;
                }

                fout << line << std::endl;

                if (lastline)
                    break;
            }
            while (std::getline(f, line));
            fout << std::endl;
            continue;
        }
    }

    return 0;
}




