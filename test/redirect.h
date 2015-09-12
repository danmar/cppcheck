// Cppcheck - A tool for static C/C++ code analysis
// Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef REDIRECT_H
#define REDIRECT_H

#include <iostream>

extern std::ostringstream errout;
extern std::ostringstream output;
/**
  * @brief Utility class for capturing cout and cerr to ostringstream buffers
  * for later use. Uses RAII to stop redirection when the object goes out of
  * scope.
  */
class RedirectOutputError {
public:
    /** Set up redirection, flushing anything in the pipes. */
    RedirectOutputError() {
        // flush all old output
        std::cout.flush();
        std::cerr.flush();

        _oldCout = std::cout.rdbuf(); // back up cout's streambuf
        _oldCerr = std::cerr.rdbuf(); // back up cerr's streambuf

        std::cout.rdbuf(_out.rdbuf()); // assign streambuf to cout
        std::cerr.rdbuf(_err.rdbuf()); // assign streambuf to cerr
    }

    /** Revert cout and cerr behaviour */
    ~RedirectOutputError() {
        std::cout.rdbuf(_oldCout); // restore cout's original streambuf
        std::cerr.rdbuf(_oldCerr); // restore cerrs's original streambuf

        errout << _err.str();
        output << _out.str();
    }

    /** Return what would be printed to cout. See also clearOutput() */
    std::string getOutput() const {
        return _out.str();
    }

    /** Normally called after getOutput() to prevent same text to be returned
    twice. */
    void clearOutput() {
        _out.str("");
    }

    /** Return what would be printed to cerr. See also clearErrout() */
    std::string getErrout() const {
        return _err.str();
    }

    /** Normally called after getErrout() to prevent same text to be returned
    twice. */
    void clearErrout() {
        _err.str("");
    }

private:
    std::ostringstream _out;
    std::ostringstream _err;
    std::streambuf *_oldCout;
    std::streambuf *_oldCerr;
};

#define REDIRECT RedirectOutputError redir;
#define GET_REDIRECT_OUTPUT redir.getOutput()
#define CLEAR_REDIRECT_OUTPUT redir.clearOutput()
#define GET_REDIRECT_ERROUT redir.getErrout()
#define CLEAR_REDIRECT_ERROUT redir.clearErrout()

#endif
