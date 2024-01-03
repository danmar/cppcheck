/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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

#include "settings.h"
#include "tokenize.h"
#include "tokenlist.h"

#include <cstddef>
#include <sstream> // IWYU pragma: keep
#include <string>
#include <vector>

class Token;
class Preprocessor;
class Suppressions;
namespace simplecpp {
    struct DUI;
}

class givenACodeSampleToTokenize {
private:
    Tokenizer tokenizer;
    const Settings settings;

public:
    explicit givenACodeSampleToTokenize(const char sample[], bool createOnly = false, bool cpp = true)
        : tokenizer(&settings, nullptr) {
        std::istringstream iss(sample);
        if (createOnly)
            tokenizer.list.createTokens(iss, cpp ? "test.cpp" : "test.c");
        else
            tokenizer.tokenize(iss, cpp ? "test.cpp" : "test.c");
    }

    Token* tokens() {
        return tokenizer.tokens();
    }

    const Token* tokens() const {
        return tokenizer.tokens();
    }
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
    static std::string getcode(Preprocessor &preprocessor, const std::string &filedata, const std::string &cfg, const std::string &filename, Suppressions *inlineSuppression = nullptr);

    static void preprocess(const char code[], std::vector<std::string> &files, Tokenizer& tokenizer);
    static void preprocess(Preprocessor &preprocessor, const char code[], std::vector<std::string> &files, Tokenizer& tokenizer);
    static void preprocess(Preprocessor &preprocessor, const char code[], std::vector<std::string> &files, Tokenizer& tokenizer, const simplecpp::DUI& dui);
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

#endif // helpersH
