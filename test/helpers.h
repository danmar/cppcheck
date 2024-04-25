/* -*- C++ -*-
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2024 Cppcheck team.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef helpersH
#define helpersH

#include "config.h"
#include "library.h"
#include "preprocessor.h"
#include "settings.h"
#include "standards.h"
#include "tokenize.h"
#include "tokenlist.h"

#include <cstddef>
#include <map>
#include <set>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>

class Token;
class SuppressionList;
class ErrorLogger;
namespace simplecpp {
    struct DUI;
}
namespace tinyxml2 {
    class XMLDocument;
}

// TODO: make Tokenizer private
class SimpleTokenizer : public Tokenizer {
public:
    template<size_t size>
    SimpleTokenizer(ErrorLogger& errorlogger, const char (&code)[size], bool cpp = true)
        : Tokenizer{s_settings, errorlogger}
    {
        if (!tokenize(code, cpp))
            throw std::runtime_error("creating tokens failed");
    }

    SimpleTokenizer(const Settings& settings, ErrorLogger& errorlogger)
        : Tokenizer{settings, errorlogger}
    {}

    /*
        Token* tokens() {
        return Tokenizer::tokens();
        }

        const Token* tokens() const {
        return Tokenizer::tokens();
        }
     */

    /**
     * Tokenize code
     * @param code The code
     * @param cpp Indicates if the code is C++
     * @param configuration E.g. "A" for code where "#ifdef A" is true
     * @return false if source code contains syntax errors
     */
    template<size_t size>
    bool tokenize(const char (&code)[size],
                  bool cpp = true,
                  const std::string &configuration = emptyString)
    {
        std::istringstream istr(code);
        if (!list.createTokens(istr, cpp ? "test.cpp" : "test.c"))
            return false;

        return simplifyTokens1(configuration);
    }

    // TODO: get rid of this
    bool tokenize(const std::string& code,
                  bool cpp = true,
                  const std::string &configuration = emptyString)
    {
        std::istringstream istr(code);
        if (!list.createTokens(istr, cpp ? "test.cpp" : "test.c"))
            return false;

        return simplifyTokens1(configuration);
    }

private:
    // TODO. find a better solution
    static const Settings s_settings;
};

class SimpleTokenList
{
public:
    template<size_t size>
    explicit SimpleTokenList(const char (&code)[size], Standards::Language lang = Standards::Language::CPP)
    {
        std::istringstream iss(code);
        if (!list.createTokens(iss, lang))
            throw std::runtime_error("creating tokens failed");
    }

    Token* front() {
        return list.front();
    }

    const Token* front() const {
        return list.front();
    }

private:
    const Settings settings;
    TokenList list{&settings};
};


class ScopedFile {
public:
    ScopedFile(std::string name, const std::string &content, std::string path = "");
    ~ScopedFile();

    const std::string& path() const
    {
        return mFullPath;
    }

    const std::string& name() const {
        return mName;
    }

    ScopedFile(const ScopedFile&) = delete;
    ScopedFile(ScopedFile&&) = delete;
    ScopedFile& operator=(const ScopedFile&) = delete;
    ScopedFile& operator=(ScopedFile&&) = delete;

private:
    const std::string mName;
    const std::string mPath;
    const std::string mFullPath;
};

class PreprocessorHelper
{
public:
    /**
     * Get preprocessed code for a given configuration
     *
     * Note: for testing only.
     *
     * @param filedata file data including preprocessing 'if', 'define', etc
     * @param cfg configuration to read out
     * @param filename name of source file
     * @param inlineSuppression the inline suppressions
     */
    static std::string getcode(const Settings& settings, ErrorLogger& errorlogger, const std::string &filedata, const std::string &cfg, const std::string &filename, SuppressionList *inlineSuppression = nullptr);
    static std::map<std::string, std::string> getcode(const Settings& settings, ErrorLogger& errorlogger, const char code[], const std::string &filename = "file.c", SuppressionList *inlineSuppression = nullptr);

    static void preprocess(const char code[], std::vector<std::string> &files, Tokenizer& tokenizer, ErrorLogger& errorlogger);
    static void preprocess(const char code[], std::vector<std::string> &files, Tokenizer& tokenizer, ErrorLogger& errorlogger, const simplecpp::DUI& dui);

    /** get remark comments */
    static std::vector<RemarkComment> getRemarkComments(const char code[], ErrorLogger& errorLogger);

private:
    static std::map<std::string, std::string> getcode(const Settings& settings, ErrorLogger& errorlogger, const char code[], std::set<std::string> cfgs, const std::string &filename = "file.c", SuppressionList *inlineSuppression = nullptr);
};

namespace cppcheck {
    template<typename T>
    std::size_t count_all_of(const std::string& str, T sub) {
        std::size_t n = 0;
        std::string::size_type pos = 0;
        while ((pos = str.find(sub, pos)) != std::string::npos) {
            ++pos;
            ++n;
        }
        return n;
    }
}

/* designated initialization helper
    Usage:
    struct S
    {
        int i;
    };

    const auto s = dinit(S,
        $.i = 1
    );
 */
#define dinit(T, ...) \
    ([&] { T ${}; __VA_ARGS__; return $; }())

// Default construct object to avoid bug in clang
// error: default member initializer for 'y' needed within definition of enclosing class 'X' outside of member functions
// see https://stackoverflow.com/questions/53408962
struct make_default_obj
{
    template<class T>
    operator T() const // NOLINT
    {
        return T{};
    }
};

inline std::string filter_valueflow(const std::string& s) {
    bool filtered = false;
    std::istringstream istr(s);
    std::string ostr;
    std::string errline;
    while (std::getline(istr, errline)) {
        if (errline.find("valueflow.cpp") != std::string::npos)
        {
            filtered = true;
            continue;
        }
        ostr += errline;
        ostr += '\n'; // TODO: last line might not contain a newline
    }
    if (!filtered)
        throw std::runtime_error("no valueflow.cpp messages were filtered");
    return ostr;
}

struct LibraryHelper
{
    static bool loadxmldata(Library &lib, const char xmldata[], std::size_t len);
    static bool loadxmldata(Library &lib, Library::Error& liberr, const char xmldata[], std::size_t len);
    static Library::Error loadxmldoc(Library &lib, const tinyxml2::XMLDocument& doc);
};

#endif // helpersH
