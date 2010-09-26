#ifndef REDIRECT_H
#define REDIRECT_H

#include <iostream>

extern std::ostringstream errout;
extern std::ostringstream output;

class RedirectOutputError
{
public:
    RedirectOutputError()
    {
        // flush all old output
        std::cout.flush();
        std::cerr.flush();

        _oldCout = std::cout.rdbuf(); // back up cout's streambuf
        _oldCerr = std::cerr.rdbuf(); // back up cerr's streambuf

        std::cout.rdbuf(_out.rdbuf()); // assign streambuf to cout
        std::cerr.rdbuf(_err.rdbuf()); // assign streambuf to cerr
    }

    ~RedirectOutputError()
    {
        std::cout.rdbuf(_oldCout); // restore cout's original streambuf
        std::cerr.rdbuf(_oldCerr); // restore cerrs's original streambuf

        errout << _err.str();
        output << _out.str();
    }

private:
    std::stringstream _out;
    std::stringstream _err;
    std::streambuf* _oldCout;
    std::streambuf *_oldCerr;
};

#define REDIRECT RedirectOutputError redir;

#endif
