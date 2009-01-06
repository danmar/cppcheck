
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
            : _funcname(funcname), _settings(settings), _msg(msg), _par1(par1)
    { }

    static const unsigned int ALL = 1;
    static const unsigned int STYLE = 2;

    std::string msg() const
    {
        std::string ret("\"" + _msg + "\"");

        if (! _par1.empty())
        {
            std::string::size_type pos = 0;
            while ((pos = ret.find("%1", pos)) != std::string::npos)
            {
                ret.erase(pos, 2);
                ret.insert(pos, "\" + " + _par1 + " + \"");
            }
        }

        return ret;
    }

    void generateCode() const
    {
        // Error message..
        std::cout << "    static std::string " << _funcname << "(";
        if (! _par1.empty())
            std::cout << "const std::string &" << _par1;
        std::cout << ") const\n";

        std::cout << "    { return " << msg() << "; }" << std::endl;

        // Settings..
        std::cout << std::endl;
        std::cout << "    static bool " << _funcname << "(const Settings &s) const" << std::endl;
        std::cout << "    { return ";
        if (_settings == 0)
            std::cout << "true";
        else
        {
            if (_settings & ALL)
                std::cout << "s._showAll";
            if (_settings & (ALL | STYLE))
                std::cout << " & ";
            if (_settings & STYLE)
                std::cout << "s._checkCodingStyle";
        }
        std::cout << "; }" << std::endl;
    }

};




int main()
{
    // Error messages..
    std::list<Message> err;
    err.push_back(Message("memleak", 0, "Memory leak: %1", "varname"));

    // Generate code..
    for (std::list<Message>::const_iterator it = err.begin(); it != err.end(); ++it)
        it->generateCode();

    return 0;
}

