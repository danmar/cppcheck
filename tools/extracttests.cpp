

#include <iostream>
#include <fstream>
#include <sstream>
#include <list>

const std::string ext(".c");
const std::string gcc("gcc");

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

    std::list<std::string> fnames;

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
            const std::string fname(testname + str(++subcount));

            if (ext == ".c")
            {
                if (fname == "array_index_21" ||
                    fname == "array_index_81" ||
                    fname == "array_index_111" ||
                    fname == "array_index_121" ||
                    fname == "array_index_131" ||
                    fname == "array_index_141" ||
                    fname == "array_index_151" ||
                    fname == "array_index_161" ||
                    fname == "array_index_171" ||
                    fname == "array_index_172" ||
                    fname == "array_index_173" ||
                    fname == "array_index_181" ||
                    fname == "array_index_201" ||
                    fname == "buffer_overrun_41" ||
                    fname == "buffer_overrun_81" ||
                    fname == "buffer_overrun_91" ||
                    fname == "buffer_overrun_101" ||
                    fname == "buffer_overrun_111" ||
                    fname == "sprintf12" ||
                    fname == "cin11" ||
                    fname == "alloc1")
                    continue;
            }

            fnames.push_back(fname);
            std::ofstream fout((fname + ext).c_str());
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

    std::ofstream makefile("Makefile");
    makefile << "all:";
    for (std::list<std::string>::const_iterator it = fnames.begin(); it != fnames.end(); ++it)
        makefile << "\t" << *it << ".o";
    makefile << std::endl;
    for (std::list<std::string>::const_iterator it = fnames.begin(); it != fnames.end(); ++it)
    {
        makefile << *it << ".o:\t" << *it << ext << std::endl;
        makefile << "\t" << gcc << " -c " << *it << ext << std::endl;
    }


    return 0;
}




