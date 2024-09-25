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

//---------------------------------------------------------------------------
#ifndef preprocessorH
#define preprocessorH
//---------------------------------------------------------------------------

#include "config.h"

#include <cstddef>
#include <cstdint>
#include <istream>
#include <list>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <simplecpp.h>

class ErrorLogger;
class Settings;
class SuppressionList;

/**
 * @brief A preprocessor directive
 * Each preprocessor directive (\#include, \#define, \#undef, \#if, \#ifdef, \#else, \#endif)
 * will be recorded as an instance of this class.
 *
 * file and linenr denote the location where the directive is defined.
 *
 */

struct CPPCHECKLIB Directive {
    /** name of (possibly included) file where directive is defined */
    std::string file;

    /** line number in (possibly included) file where directive is defined */
    unsigned int linenr;

    /** the actual directive text */
    std::string str;

    struct DirectiveToken {
        explicit DirectiveToken(const simplecpp::Token & _tok);
        int line;
        int column;
        std::string tokStr;
    };

    std::vector<DirectiveToken> strTokens;

    /** record a directive (possibly filtering src) */
    Directive(const simplecpp::Location & _loc, std::string _str);
    Directive(std::string _file, int _linenr, std::string _str);
};

class CPPCHECKLIB RemarkComment {
public:
    RemarkComment(std::string file, unsigned int lineNumber, std::string str)
        : file(std::move(file))
        , lineNumber(lineNumber)
        , str(std::move(str))
    {}

    /** name of file */
    std::string file;

    /** line number for the code that the remark comment is about */
    unsigned int lineNumber;

    /** remark text */
    std::string str;
};

/// @addtogroup Core
/// @{

/**
 * @brief The cppcheck preprocessor.
 * The preprocessor has special functionality for extracting the various ifdef
 * configurations that exist in a source file.
 */
class CPPCHECKLIB WARN_UNUSED Preprocessor {
    // TODO: get rid of this
    friend class PreprocessorHelper;
    friend class TestPreprocessor;
    friend class TestUnusedVar;

public:
    /** character that is inserted in expanded macros */
    static char macroChar;

    explicit Preprocessor(const Settings& settings, ErrorLogger &errorLogger);
    virtual ~Preprocessor();

    void inlineSuppressions(const simplecpp::TokenList &tokens, SuppressionList &suppressions);

    std::list<Directive> createDirectives(const simplecpp::TokenList &tokens) const;

    std::set<std::string> getConfigs(const simplecpp::TokenList &tokens) const;

    std::vector<RemarkComment> getRemarkComments(const simplecpp::TokenList &tokens) const;

    bool loadFiles(const simplecpp::TokenList &rawtokens, std::vector<std::string> &files);

    void removeComments(simplecpp::TokenList &tokens);

    static void setPlatformInfo(simplecpp::TokenList &tokens, const Settings& settings);

    simplecpp::TokenList preprocess(const simplecpp::TokenList &tokens1, const std::string &cfg, std::vector<std::string> &files, bool throwError = false);

    std::string getcode(const simplecpp::TokenList &tokens1, const std::string &cfg, std::vector<std::string> &files, bool writeLocations);

    /**
     * Calculate HASH. Using toolinfo, tokens1, filedata.
     *
     * @param tokens1    Sourcefile tokens
     * @param toolinfo   Arbitrary extra toolinfo
     * @return HASH
     */
    std::size_t calculateHash(const simplecpp::TokenList &tokens1, const std::string &toolinfo) const;

    void simplifyPragmaAsm(simplecpp::TokenList &tokenList) const;

    static void getErrorMessages(ErrorLogger &errorLogger, const Settings &settings);

    /**
     * dump all directives present in source file
     */
    void dump(std::ostream &out) const;

    static bool hasErrors(const simplecpp::Output &output);

private:
    void handleErrors(const simplecpp::OutputList &outputList, bool throwError);

    void reportOutput(const simplecpp::OutputList &outputList, bool showerror);

    static void simplifyPragmaAsmPrivate(simplecpp::TokenList &tokenList);

    /**
     * Include file types.
     */
    enum HeaderTypes : std::uint8_t {
        UserHeader = 1,
        SystemHeader
    };

    void missingInclude(const std::string &filename, unsigned int linenr, const std::string &header, HeaderTypes headerType);
    void error(const std::string &filename, unsigned int linenr, const std::string &msg);

    static bool hasErrors(const simplecpp::OutputList &outputList);

    void addRemarkComments(const simplecpp::TokenList &tokens, std::vector<RemarkComment> &remarkComments) const;

    const Settings& mSettings;
    ErrorLogger &mErrorLogger;

    /** list of all directives met while preprocessing file */

    std::map<std::string, simplecpp::TokenList *> mTokenLists;

    /** filename for cpp/c file - useful when reporting errors */
    std::string mFile0;

    /** simplecpp tracking info */
    std::list<simplecpp::MacroUsage> mMacroUsage;
    std::list<simplecpp::IfCond> mIfCond;
};

/// @}
//---------------------------------------------------------------------------
#endif // preprocessorH
