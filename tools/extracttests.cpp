

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
    if (argc != 2) {
        std::cerr << "syntax: extracttests testfile" << std::endl;
        return 0;
    }

    std::string testname;
    unsigned int subcount = 0;

    std::ifstream f(argv[1]);
    std::string line;
    while (std::getline(f, line)) {
        {
            std::string::size_type pos = line.find_first_not_of(" ");
            if (pos > 0 && pos != std::string::npos)
                line.erase(0, pos);
        }

        if (line.compare(0, 5, "void ") == 0) {
            testname = line.substr(5, line.size() - 7);
            subcount = 0;
            continue;
        }

        if (line == "}") {
            testname = "";
            subcount = 0;
            continue;
        }

        if (!testname.empty() && line.compare(0, 5, "check") == 0 && line.find("(\"") != std::string::npos) {
            std::ofstream fout((testname + str(++subcount) + ext).c_str());
            fout << "#include <string.h>" << std::endl;
            fout << "#include <stdio.h>" << std::endl;
            fout << "#include <stdlib.h>" << std::endl;

            if (testname == "nullpointer1") {
                if (subcount < 6) {
                    fout << "class Token\n"
                         << "{\n"
                         << "public:\n"
                         << "    const char  *str() const;\n"
                         << "    const Token *next() const;\n"
                         << "    unsigned int size() const;\n"
                         << "    char         read () const;\n"
                         << "    operator bool() const;\n"
                         << "};\n"
                         << "static Token *tokens;\n";
                } else {
                    fout << "struct A\n"
                         "{\n"
                         "    char b();\n"
                         "    A *next;\n"
                         "};\n";
                }
            }

            if (testname == "nullpointer2") {
                fout << "class Fred\n"
                     << "{\n"
                     << "public:\n"
                     << "    void hello() const;\n"
                     << "    operator bool() const;\n"
                     << "};\n";
            }

            if (testname == "nullpointer3") {
                fout << "struct DEF { };\n"
                     << "struct ABC : public DEF\n"
                     << "{\n"
                     << "    int a,b,c;\n"
                     << "    struct ABC *next;\n"
                     << "};\n"
                     << "void bar(int); void f(struct ABC **);\n";
            }

            if (testname == "nullpointer4") {
                fout << "void bar(int);\n"
                     << "int** f(int **p = 0);\n"
                     << "extern int x;\n"
                     << "struct P {\n"
                     << "    bool check() const;\n"
                     << "    P* next() const;\n"
                     << "};\n";
            }

            if (testname == "nullpointer5") {
                fout << "struct A {\n"
                     << "    char c() const;\n"
                     << "    operator bool() const;\n"
                     << "};\n";
            }

            if (testname == "nullpointer6") {
                fout << "struct Foo {\n"
                     << "    void abcd() const;\n"
                     << "};\n"
                     << "struct FooBar : public Foo { };\n"
                     << "struct FooCar : public Foo { };\n"
                     << "extern int a;\n";
            }

            if (testname == "nullpointer7") {
                fout << "struct wxLongLong {\n"
                     << "    wxLongLong(int) { }\n"
                     << "    long GetValue() const;\n"
                     << "};\n";
            }

            do {
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
                while ((pos = line.find("\\", pos)) != std::string::npos) {
                    line.erase(pos, 1);
                    if (line[pos] == 'n')
                        line.erase(pos, 1);
                    else
                        ++pos;
                }

                fout << line << std::endl;

                if (lastline)
                    break;
            } while (std::getline(f, line));
            fout << std::endl;
            continue;
        }
    }

    return 0;
}




