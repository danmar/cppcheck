/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis
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
public:
    enum Settings {error, all, style, style_all, security, never};

    Message(std::string funcname, Settings settings, std::string msg);
    Message(std::string funcname, Settings settings, std::string msg, std::string par1);
    Message(std::string funcname, Settings settings, std::string msg, std::string par1, std::string par2);
    Message(std::string funcname, Settings settings, std::string msg, std::string par1, std::string par2, std::string details);

    void generateCode(std::ostream &ostr) const;

    void generateDoc(std::ostream &ostr, Settings i) const;

    std::string stringifySettings(bool text) const;

private:
    std::string _funcname;
    std::string _msg;
    std::string _par1;
    std::string _par2;
    Settings _settings;
    std::string _details;

    std::string msg(bool code) const;
};




int main()
{
    // Error messages..
    std::list<Message> err;

    // checkbufferoverrun.cpp
    err.push_back(Message("arrayIndexOutOfBounds", Message::all, "Array index out of bounds"));
    err.push_back(Message("bufferOverrun", Message::all, "Buffer overrun"));
    err.push_back(Message("outOfBounds", Message::error, "%1 is out of bounds", "what"));
    err.push_back(Message("stlOutOfBounds", Message::error, "%1 is out of bounds", "what"));

    // checkclass.cpp..
    err.push_back(Message("noConstructor", Message::style, "The class '%1' has no constructor", "classname"));
    err.push_back(Message("uninitVar", Message::style, "Member variable not initialized in the constructor '%1::%2'", "classname", "varname"));
    err.push_back(Message("unusedPrivateFunction", Message::style, "Unused private function '%1::%2'", "classname", "funcname"));
    err.push_back(Message("memsetClass", Message::error, "Using '%1' on class", "memfunc"));
    err.push_back(Message("memsetStruct", Message::error, "Using '%1' on struct that contains a 'std::%2'", "memfunc", "classname"));
    err.push_back(Message("operatorEq", Message::style, "'operator=' should return something"));
    err.push_back(Message("virtualDestructor", Message::error, "Class %1 which is inherited by class %2 does not have a virtual destructor", "Base", "Derived"));

    // checkfunctionusage.cpp..
    err.push_back(Message("unusedFunction", Message::style_all, "[%1]: The function '%2' is never used", "filename", "funcname"));

    // checkmemoryleak.cpp..
    err.push_back(Message("mismatchAllocDealloc", Message::error, "Mismatching allocation and deallocation: %1", "varname"));
    err.push_back(Message("memleak", Message::error, "Memory leak: %1", "varname"));
    err.push_back(Message("memleakall", Message::all, "Memory leak: %1", "varname"));
    err.push_back(Message("resourceLeak", Message::error, "Resource leak: %1", "varname"));
    err.push_back(Message("deallocDealloc", Message::error, "Deallocating a deallocated pointer: %1", "varname"));
    err.push_back(Message("deallocuse", Message::error, "Using '%1' after it is deallocated / released", "varname"));
    err.push_back(Message("mismatchSize", Message::error, "The given size %1 is mismatching", "sz"));

    // checkother.cpp..
    err.push_back(Message("cstyleCast", Message::style, "C-style pointer casting"));
    err.push_back(Message("redundantIfDelete0", Message::style, "Redundant condition. It is safe to deallocate a NULL pointer"));
    err.push_back(Message("redundantIfRemove", Message::style, "Redundant condition. The remove function in the STL will not do anything if element doesn't exist"));
    err.push_back(Message("dangerousUsageStrtol", Message::error, "Invalid radix in call to strtol or strtoul. Must be 0 or 2-36"));
    err.push_back(Message("ifNoAction", Message::style, "Found redundant if condition - 'if (condition);'"));
    err.push_back(Message("sprintfOverlappingData", Message::error, "Overlapping data buffer %1", "varname", "",
                          "    -- If copying takes place between objects that overlap as a result of a\n"
                          "       call to sprintf() or snprintf(), the results are undefined.\n"
                          "       http://www.opengroup.org/onlinepubs/000095399/functions/printf.html"));
    err.push_back(Message("udivError", Message::error, "Unsigned division. The result will be wrong."));
    err.push_back(Message("udivWarning", Message::style_all, "Warning: Division with signed and unsigned operators"));
    err.push_back(Message("unusedStructMember", Message::style, "struct or union member '%1::%2' is never used", "structname", "varname"));
    err.push_back(Message("passedByValue", Message::style, "Function parameter '%1' is passed by value. It could be passed by reference instead.", "parname"));
    err.push_back(Message("constStatement", Message::style, "Redundant code: Found a statement that begins with %1 constant", "type"));
    err.push_back(Message("charArrayIndex", Message::style, "Warning - using char variable as array index"));
    err.push_back(Message("charBitOp", Message::style, "Warning - using char variable in bit operation"));
    err.push_back(Message("variableScope", Message::never, "The scope of the variable %1 can be limited", "varname"));
    err.push_back(Message("conditionAlwaysTrueFalse", Message::style, "Condition is always %1", "truefalse"));
    err.push_back(Message("strPlusChar", Message::error, "Unusual pointer arithmetic"));
    err.push_back(Message("returnLocalVariable", Message::error, "Returning pointer to local array variable"));

    // checkdangerousfunctions.cpp..
    err.push_back(Message("dangerousFunctionmktemp", Message::style, "Found 'mktemp'. You should use 'mkstemp' instead"));
    err.push_back(Message("dangerousFunctiongets", Message::style, "Found 'gets'. You should use 'fgets' instead"));
    err.push_back(Message("dangerousFunctionscanf", Message::style, "Found 'scanf'. You should use 'fgets' instead"));

    // checkstl.cpp..
    err.push_back(Message("iteratorUsage", Message::error, "Same iterator is used with both %1 and %2", "container1", "container2"));
    err.push_back(Message("erase", Message::error, "Dangerous usage of erase"));
    err.push_back(Message("pushback", Message::error, "After push_back or push_front, the iterator '%1' may be invalid", "iterator_name"));


    // checkvalidate.cpp
    err.push_back(Message("unvalidatedInput", Message::security, "Unvalidated input"));


    // Generate code..
    std::cout << "Generate code.." << std::endl;
    std::ofstream fout("errorlogger.h");

    fout << "/*\n";
    fout << " * Cppcheck - A tool for static C/C++ code analysis\n";
    fout << " * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,\n";
    fout << " * Leandro Penz, Kimmo Varis\n";
    fout << " *\n";
    fout << " * This program is free software: you can redistribute it and/or modify\n";
    fout << " * it under the terms of the GNU General Public License as published by\n";
    fout << " * the Free Software Foundation, either version 3 of the License, or\n";
    fout << " * (at your option) any later version.\n";
    fout << " *\n";
    fout << " * This program is distributed in the hope that it will be useful,\n";
    fout << " * but WITHOUT ANY WARRANTY; without even the implied warranty of\n";
    fout << " * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n";
    fout << " * GNU General Public License for more details.\n";
    fout << " *\n";
    fout << " * You should have received a copy of the GNU General Public License\n";
    fout << " * along with this program.  If not, see <http://www.gnu.org/licenses/\n";
    fout << " */\n\n";
    fout << "// THIS FILE IS GENERATED BY MACHINE, SEE ../tools/errmsg.cpp !\n\n";
    fout << "#ifndef errorloggerH\n";
    fout << "#define errorloggerH\n";
    fout << "#include <list>\n";
    fout << "#include <string>\n";
    fout << "#include \"settings.h\"\n";
    fout << "class Token;\n";
    fout << "class Tokenizer;\n";
    fout << "\n";
    fout << "/**\n";
    fout << " * This is an interface, which the class responsible of error logging\n";
    fout << " * should implement.\n";
    fout << " */\n";
    fout << "class ErrorLogger\n";
    fout << "{\n";
    fout << "public:\n";
    fout << "\n";
    fout << "    /**\n";
    fout << "     * Wrapper for error messages, provided by reportErr()\n";
    fout << "     */\n";
    fout << "    class ErrorMessage\n";
    fout << "    {\n";
    fout << "    public:\n";
    fout << "        /**\n";
    fout << "         * File name and line number.\n";
    fout << "         */\n";
    fout << "        class FileLocation\n";
    fout << "        {\n";
    fout << "        public:\n";
    fout << "            std::string file;\n";
    fout << "            unsigned int line;\n";
    fout << "        };\n";
    fout << "\n";
    fout << "        ErrorMessage(const std::list<FileLocation> &callStack, const std::string &severity, const std::string &msg, const std::string &id);\n";
    fout << "        ErrorMessage();\n";
    fout << "        std::string toXML() const;\n";
    fout << "        std::string toText() const;\n";
    fout << "        std::string serialize() const;\n";
    fout << "        bool deserialize( const std::string &data );\n";
    fout << "    private:\n";
    fout << "        std::list<FileLocation> _callStack;\n";
    fout << "        std::string _severity;\n";
    fout << "        std::string _msg;\n";
    fout << "        std::string _id;\n";
    fout << "    };\n";
    fout << "\n";
    fout << "    ErrorLogger() { }\n";
    fout << "    virtual ~ErrorLogger() { }\n";
    fout << "\n";
    fout << "    /**\n";
    fout << "     * Information about progress is directed here.\n";
    fout << "     * Override this to receive the progress messages.\n";
    fout << "     *\n";
    fout << "     * @param outmsg, E.g. \"Checking main.cpp...\"\n";
    fout << "     */\n";
    fout << "    virtual void reportOut(const std::string &outmsg) = 0;\n";
    fout << "\n";
    fout << "    /**\n";
    fout << "     * Information about found errors and warnings is directed\n";
    fout << "     * here. Override this to receive the errormessages.\n";
    fout << "     *\n";
    fout << "     * @param msg Location and other information about the found.\n";
    fout << "     * error\n";
    fout << "     */\n";
    fout << "    virtual void reportErr(const ErrorLogger::ErrorMessage &msg) = 0;\n";
    fout << "\n";
    fout << "    /**\n";
    fout << "     * Information about how many files have been checked\n";
    fout << "     *\n";
    fout << "     * @param index This many files have been checked.\n";
    fout << "     * @param max This many files there are in total.\n";
    fout << "     */\n";
    fout << "    virtual void reportStatus(unsigned int index, unsigned int max) = 0;\n";
    fout << "\n";

    for (std::list<Message>::const_iterator it = err.begin(); it != err.end(); ++it)
        it->generateCode(fout);

    fout << "\n";
    fout << "    static std::string callStackToString(const std::list<ErrorLogger::ErrorMessage::FileLocation> &callStack);\n";
    fout << "\n";
    fout << "private:\n";
    fout << "    void _writemsg(const Tokenizer *tokenizer, const Token *tok, const char severity[], const std::string msg, const std::string &id);\n";
    fout << "    void _writemsg(const Tokenizer *tokenizer, const std::list<const Token *> &callstack, const char severity[], const std::string msg, const std::string &id);\n";
    fout << "    void _writemsg(const std::string msg, const std::string &id);\n";
    fout << "};\n";
    fout << "#endif\n";
    std::cout << std::endl;

    // Generate documentation..
    std::cout << "Generate doc.." << std::endl;
    const unsigned int NUMSUITE = 5;
    for (unsigned int i = 0; i < NUMSUITE; ++i)
    {
        const char *suite[NUMSUITE] = { "error", "all", "style", "all + style", "security" };
        const Message::Settings settings[NUMSUITE] = { Message::error, Message::all, Message::style, Message::style_all, Message::security };
        std::cout << "    =" << suite[i] << "=" << std::endl;
        for (std::list<Message>::const_iterator it = err.begin(); it != err.end(); ++it)
            it->generateDoc(std::cout, settings[i]);
    }
    std::cout << std::endl;

    return 0;
}





Message::Message(std::string funcname, Settings settings, std::string msg)
        : _funcname(funcname), _msg(msg), _par1(""), _par2(""), _settings(settings), _details("")
{ }

Message::Message(std::string funcname, Settings settings, std::string msg, std::string par1)
        : _funcname(funcname), _msg(msg), _par1(par1), _par2(""), _settings(settings), _details("")
{ }

Message::Message(std::string funcname, Settings settings, std::string msg, std::string par1, std::string par2)
        : _funcname(funcname), _msg(msg), _par1(par1), _par2(par2), _settings(settings), _details("")
{ }

Message::Message(std::string funcname, Settings settings, std::string msg, std::string par1, std::string par2, std::string details)
        : _funcname(funcname), _msg(msg), _par1(par1), _par2(par2), _settings(settings), _details(details)
{ }

std::string Message::msg(bool code) const
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

    if (! _par2.empty())
    {
        std::string::size_type pos = 0;
        while ((pos = ret.find("%2", pos)) != std::string::npos)
        {
            ret.erase(pos, 2);
            if (code)
                ret.insert(pos, "\" + " + _par2 + " + \"");
            else
                ret.insert(pos, _par2);
        }
    }

    return ret;
}

void Message::generateCode(std::ostream &ostr) const
{
    bool loc = bool(_msg.substr(0, 4) != "[%1]");

    // Error message..
    ostr << "    void " << _funcname << "(";
    if (loc)
    {
        ostr << "const Tokenizer *tokenizer, ";

        if (_funcname == "mismatchAllocDealloc" ||
            _funcname == "arrayIndexOutOfBounds")
            ostr << "const std::list<const Token *> &Location";
        else
            ostr << "const Token *Location";
    }
    /*
    if (_details.size())
        ostr << ", const Settings &settings";
    */
    if (! _par1.empty())
        ostr << (loc ? ", " : "") <<  "const std::string &" << _par1;
    if (! _par2.empty())
        ostr << ", const std::string &" << _par2;
    ostr << ")\n";
    ostr << "    {\n";
    ostr << "        _writemsg(";
    if (loc)
        ostr << "tokenizer, Location, \"" << stringifySettings(true) << "\", ";
    ostr << msg(true) << ", \"" << _funcname << "\");\n";
    /*
        ostr << "        return ";
        if (loc)
            ostr << "msg1(tokenizer, Location) + ";
        ostr << " std::string(\"(" << stringifySettings(true) << ") \") + ";
        ostr << msg(true);
        if (_details.empty())
            ostr << ";\n";
        else
        {
            ostr << " + std::string(settings._verbose ? \"\\n";
            for (std::string::size_type pos = 0; pos < _details.length(); ++pos)
            {
                if (_details[pos] == '\n')
                    ostr << "\\n";
                else
                    ostr << _details[pos];
            }
            ostr << "\" : \"\");\n";
        }
    */
    ostr << "    }\n";

    // Settings..
    ostr << "    static bool " << _funcname << "(";
    if (_settings != error && _settings != never)
        ostr << "const Settings &s";
    ostr << ")" << std::endl;
    ostr << "    {\n";
    ostr << "        return " << stringifySettings(false) << ";\n";
    ostr << "    }\n\n";
}

void Message::generateDoc(std::ostream &ostr, Message::Settings i) const
{
    if (_settings == i)
    {
        ostr << "    " << msg(false) << std::endl;
    }
}


std::string Message::stringifySettings(bool text) const
{
    switch (_settings)
    {
    case error:
        return text ? "error" : "true";
    case all:
        return text ? "all" : "s._showAll";
    case style:
        return text ? "style" : "s._checkCodingStyle";
    case style_all:
        return text ? "all style" : "s._checkCodingStyle || s._showAll";
    case security:
        return text ? "security" : "s._security";
    case never:
        return text ? "never" : "false";
    }
    return "";
}



