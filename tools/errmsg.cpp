/*
 * cppcheck - c/c++ syntax checking
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */

#include <fstream>
#include <iostream>
#include <list>
#include <string>

class Message
{
private:
    std::string _funcname;
    std::string _msg;
    std::string _par1;
    unsigned int _settings;

public:
    Message(std::string funcname, unsigned int settings, std::string msg, std::string par1)
            : _funcname(funcname), _msg(msg), _par1(par1), _settings(settings)
    { }

    static const unsigned int ALL = 1;
    static const unsigned int STYLE = 2;

    std::string msg(bool code) const
    {
        const char *str = code ? "\"" : "";
        std::string ret(str + _msg + str);
        if (! _par1.empty())
        {
            std::string::size_type pos = 0;
            while ((pos = ret.find("%1", pos)) != std::string::npos)
            {
                ret.erase(pos, 2);
                if (code)
                    ret.insert(pos, "\" + " + _par1 + " + \"");
                else
                    ret.insert(pos, _par1);
            }
        }
        return ret;
    }

    void generateCode(std::ostream &ostr) const
    {
        // Error message..
        ostr << "    static std::string " << _funcname << "(const Tokenizer *tokenizer, const Token *Location, ";
        if (! _par1.empty())
            ostr << "const std::string &" << _par1;
        ostr << ")\n";
        ostr << "    { return msg1(tokenizer, Location) + " << msg(true) << "; }" << std::endl;

        // Settings..
        ostr << std::endl;
        ostr << "    static bool " << _funcname << "(const Settings &s)" << std::endl;
        ostr << "    { return ";
        if (_settings == 0)
            ostr << "true";
        else
        {
            if (_settings & ALL)
                ostr << "s._showAll";
            if (_settings & (ALL | STYLE))
                ostr << " & ";
            if (_settings & STYLE)
                ostr << "s._checkCodingStyle";
        }
        ostr << "; }" << std::endl;
    }

    void generateDoc(std::ostream &ostr, unsigned int i) const
    {
        if (_settings == i)
        {
            ostr << "    " << msg(false) << std::endl;
        }
    }

};




int main()
{
    // Error messages..
    std::list<Message> err;
    err.push_back(Message("memleak", 0, "Memory leak: %1", "varname"));
    err.push_back(Message("resourceLeak", 0, "Resource leak: %1", "varname"));

    // Generate code..
    std::cout << "Generate code.." << std::endl;
    std::ofstream fout("errormessage.h");
    fout << "#ifndef errormessageH\n";
    fout << "#define errormessageH\n";
    fout << "#include <string>\n";
    fout << "#include \"settings.h\"\n";
    fout << "class Token;\n";
    fout << "class Tokenizer;\n";
    fout << "class ErrorMessage\n";
    fout << "{\n";
    fout << "private:\n";
    fout << "    ErrorMessage() { }\n";
    fout << "    static std::string msg1(const Tokenizer *tokenizer, const Token *Location);\n";
    fout << "public:\n";
    for (std::list<Message>::const_iterator it = err.begin(); it != err.end(); ++it)
        it->generateCode(fout);
    fout << "};\n";
    fout << "#endif\n";
    std::cout << std::endl;

    // Generate documentation..
    std::cout << "Generate doc.." << std::endl;
    for (unsigned int i = 0; i < 4; ++i)
    {
        const char *suite[4] = { "standard", "all", "style", "all + style" };
        std::cout << "    =" << suite[i] << "=" << std::endl;
        for (std::list<Message>::const_iterator it = err.begin(); it != err.end(); ++it)
            it->generateDoc(std::cout, i);
    }
    std::cout << std::endl;

    return 0;
}

