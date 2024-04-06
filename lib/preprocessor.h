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

//---------------------------------------------------------------------------
#ifndef preprocessorH
#define preprocessorH
//---------------------------------------------------------------------------

#include "config.h"

#include <cstddef>
#include <istream>
#include <list>
#include <map>
#include <set>
#include <string>
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
 * file and linenr denote the location where where the directive is defined.
 *
 */

class CPPCHECKLIB Directive {
public:
    /** name of (possibly included) file where directive is defined */
    std::string file;

    /** line number in (possibly included) file where directive is defined */
    unsigned int linenr;

    /** the actual directive text */
    std::string str;

    /** record a directive (possibly filtering src) */
    Directive(std::string _file, const int _linenr, const std::string &_str);
};

/// @addtogroup Core
/// @{

/**
 * @brief The cppcheck preprocessor.
 * The preprocessor has special functionality for extracting the various ifdef
 * configurations that exist in a source file.
 */
class CPPCHECKLIB Preprocessor {
    // TODO: get rid of this
    friend class PreprocessorHelper;
    friend class TestPreprocessor;
    friend class TestUnusedVar;

public:

    /**
     * Include file types.
     */
    enum HeaderTypes {
        UserHeader = 1,
        SystemHeader
    };

    /** character that is inserted in expanded macros */
    static char macroChar;

    explicit Preprocessor(const Settings& settings, ErrorLogger *errorLogger = nullptr);
    virtual ~Preprocessor();

    void inlineSuppressions(const simplecpp::TokenList &tokens, SuppressionList &suppressions);

    void setDirectives(const simplecpp::TokenList &tokens);

    /** list of all directives met while preprocessing file */
    const std::list<Directive> &getDirectives() const {
        return mDirectives;
    }

    std::set<std::string> getConfigs(const simplecpp::TokenList &tokens) const;

    void handleErrors(const simplecpp::OutputList &outputList, bool throwError);

    bool loadFiles(const simplecpp::TokenList &rawtokens, std::vector<std::string> &files);

    void removeComments();

    void setPlatformInfo(simplecpp::TokenList *tokens) const;

    simplecpp::TokenList preprocess(const simplecpp::TokenList &tokens1, const std::string &cfg, std::vector<std::string> &files, bool throwError = false);

    std::string getcode(const simplecpp::TokenList &tokens1, const std::string &cfg, std::vector<std::string> &files, const bool writeLocations);

    /**
     * Calculate HASH. Using toolinfo, tokens1, filedata.
     *
     * @param tokens1    Sourcefile tokens
     * @param toolinfo   Arbitrary extra toolinfo
     * @return HASH
     */
    std::size_t calculateHash(const simplecpp::TokenList &tokens1, const std::string &toolinfo) const;

    void simplifyPragmaAsm(simplecpp::TokenList *tokenList) const;

private:

    static void simplifyPragmaAsmPrivate(simplecpp::TokenList *tokenList);

public:


    static void getErrorMessages(ErrorLogger *errorLogger, const Settings &settings);

    /**
     * dump all directives present in source file
     */
    void dump(std::ostream &out) const;

    void reportOutput(const simplecpp::OutputList &outputList, bool showerror);

    static bool hasErrors(const simplecpp::Output &output);

private:
    void missingInclude(const std::string &filename, unsigned int linenr, const std::string &header, HeaderTypes headerType);
    void error(const std::string &filename, unsigned int linenr, const std::string &msg);

    static bool hasErrors(const simplecpp::OutputList &outputList);

    void setDirectives(const std::list<Directive> &directives) {
        mDirectives = directives;
    }

    const Settings& mSettings;
    ErrorLogger *mErrorLogger;

    /** list of all directives met while preprocessing file */
    std::list<Directive> mDirectives;

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
