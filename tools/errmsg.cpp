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
public:
    enum Settings {always, all, style, style_all, never};

    Message(std::string funcname, Settings settings, std::string msg);
    Message(std::string funcname, Settings settings, std::string msg, std::string par1);
    Message(std::string funcname, Settings settings, std::string msg, std::string par1, std::string par2);

    void generateCode(std::ostream &ostr) const;

    void generateDoc(std::ostream &ostr, Settings i) const;

private:
    std::string _funcname;
    std::string _msg;
    std::string _par1;
    std::string _par2;
    Settings _settings;

    std::string msg(bool code) const;
};




int main()
{
    // Error messages..
    std::list<Message> err;

    // checkbufferoverrun.cpp
    err.push_back(Message("arrayIndexOutOfBounds", Message::all, "Array index out of bounds"));
    err.push_back(Message("bufferOverrun", Message::all, "Buffer overrun"));
    err.push_back(Message("outOfBounds", Message::always, "%1 is out of bounds", "what"));

    // checkclass.cpp..
    err.push_back(Message("noConstructor", Message::style, "The class '%1' has no constructor", "classname"));
    err.push_back(Message("uninitVar", Message::always, "Uninitialized member variable '%1::%2'", "classname", "varname"));
    err.push_back(Message("unusedPrivateFunction", Message::style, "Unused private function '%1::%2'", "classname", "funcname"));
    err.push_back(Message("memsetClass", Message::always, "Using '%1' on class", "memfunc"));
    err.push_back(Message("memsetStruct", Message::always, "Using '%1' on struct that contains a 'std::%2'", "memfunc", "classname"));
    err.push_back(Message("operatorEq", Message::style, "'operator=' should return something"));
    err.push_back(Message("virtualDestructor", Message::always, "Class %1 which is inherited by class %2 does not have a virtual destructor", "Base", "Derived"));

    // checkfunctionusage.cpp..
    err.push_back(Message("unusedFunction", Message::style_all, "[%1]: The function '%2' is never used", "filename", "funcname"));

    // checkmemoryleak.cpp..
    err.push_back(Message("mismatchAllocDealloc", Message::always, "Mismatching allocation and deallocation: %1", "varname"));
    err.push_back(Message("memleak", Message::always, "Memory leak: %1", "varname"));
    err.push_back(Message("resourceLeak", Message::always, "Resource leak: %1", "varname"));
    err.push_back(Message("deallocDealloc", Message::always, "Deallocating a deallocated pointer: %1", "varname"));

    // checkother.cpp..
    err.push_back(Message("cstyleCast", Message::style, "C-style pointer casting"));
    err.push_back(Message("redundantIfDelete0", Message::style, "Redundant condition. It is safe to deallocate a NULL pointer"));
    err.push_back(Message("redundantIfRemove", Message::style, "Redundant condition. The remove function in the STL will not do anything if element doesn't exist"));
    err.push_back(Message("dangerousUsageStrtol", Message::always, "Invalid radix in call to strtol or strtoul. Must be 0 or 2-36"));
    err.push_back(Message("ifNoAction", Message::style, "Found redundant if condition - 'if (condition);'"));
    err.push_back(Message("sprintfOverlappingData", Message::always, "Overlapping data buffer %1", "varname"));
    err.push_back(Message("udivError", Message::always, "Unsigned division. The result will be wrong."));
    err.push_back(Message("udivWarning", Message::style_all, "Warning: Division with signed and unsigned operators"));
    err.push_back(Message("unusedStructMember", Message::style, "struct or union member '%1::%2' is never used", "structname", "varname"));
    err.push_back(Message("unreachableCode", Message::always, "Unreachable code below a 'return'"));
    err.push_back(Message("passedByValue", Message::always, "Function parameter '%1' is passed by value. It could be passed by reference instead.", "parname"));
    err.push_back(Message("unusedVariable", Message::style, "Unused variable '%1'", "varname"));
    err.push_back(Message("unreadVariable", Message::style, "Variable '%1' is assigned a value that is never used", "varname"));
    err.push_back(Message("unassignedVariable", Message::style, "Variable '%1' is not assigned a value", "varname"));
    err.push_back(Message("constStatement", Message::style, "Redundant code: Found a statement that begins with %1 constant", "type"));
    err.push_back(Message("charArrayIndex", Message::style, "Warning - using char variable as array index"));
    err.push_back(Message("charBitOp", Message::style, "Warning - using char variable in bit operation"));
    err.push_back(Message("variableScope", Message::never, "The scope of the variable %1 can be limited", "varname"));
    err.push_back(Message("ifAssignment", Message::style, "Assignment in if-condition"));
    err.push_back(Message("conditionAlwaysTrueFalse", Message::style, "Condition is always %1", "truefalse"));
    err.push_back(Message("strPlusChar", Message::always, "Unusual pointer arithmetic"));

    // Generate code..
    std::cout << "Generate code.." << std::endl;
    std::ofstream fout("errormessage.h");

    fout << "/*\n";
    fout << " * cppcheck - c/c++ syntax checking\n";
    fout << " * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam\n";
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
        const char *suite[4] = { "always", "all", "style", "all + style" };
        const Message::Settings settings[4] = { Message::always, Message::all, Message::style, Message::style_all };
        std::cout << "    =" << suite[i] << "=" << std::endl;
        for (std::list<Message>::const_iterator it = err.begin(); it != err.end(); ++it)
            it->generateDoc(std::cout, settings[i]);
    }
    std::cout << std::endl;

    return 0;
}





Message::Message(std::string funcname, Settings settings, std::string msg)
        : _funcname(funcname), _msg(msg), _par1(""), _par2(""), _settings(settings)
{ }

Message::Message(std::string funcname, Settings settings, std::string msg, std::string par1)
        : _funcname(funcname), _msg(msg), _par1(par1), _par2(""), _settings(settings)
{ }

Message::Message(std::string funcname, Settings settings, std::string msg, std::string par1, std::string par2)
        : _funcname(funcname), _msg(msg), _par1(par1), _par2(par2), _settings(settings)
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
    ostr << "    static std::string " << _funcname << "(";
    if (loc)
        ostr << "const Tokenizer *tokenizer, const Token *Location";
    if (! _par1.empty())
        ostr << (loc ? ", " : "") <<  "const std::string &" << _par1;
    if (! _par2.empty())
        ostr << ", const std::string &" << _par2;
    ostr << ")\n";
    ostr << "    {\n";
    ostr << "        return ";
    if (loc)
        ostr << "msg1(tokenizer, Location) + ";
    ostr << msg(true) << ";\n";
    ostr << "    }\n";

    // Settings..
    ostr << "    static bool " << _funcname << "(const Settings &s)" << std::endl;
    ostr << "    {\n";
    ostr << "        return ";
    switch (_settings)
    {
    case always:
        ostr << "true";
        break;
    case all:
        ostr << "s._showAll";
        break;
    case style:
        ostr << "s._checkCodingStyle";
        break;
    case style_all:
        ostr << "s._showAll & s._checkCodingStyle";
        break;
    case never:
        ostr << "false";
        break;
    }
    ostr << ";\n";
    ostr << "    }\n\n";
}

void Message::generateDoc(std::ostream &ostr, Message::Settings i) const
{
    if (_settings == i)
    {
        ostr << "    " << msg(false) << std::endl;
    }
}





