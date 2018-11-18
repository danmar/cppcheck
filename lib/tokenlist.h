/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2018 Cppcheck team.
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
#ifndef tokenlistH
#define tokenlistH
//---------------------------------------------------------------------------

#include "config.h"
#include "token.h"

#include <string>
#include <vector>

class Settings;
class Token;

namespace simplecpp {
    class TokenList;
}

/// @addtogroup Core
/// @{

class CPPCHECKLIB TokenList {
public:
    explicit TokenList(const Settings* settings);
    ~TokenList();

    void setSettings(const Settings *settings) {
        mSettings = settings;
    }

    const Settings *getSettings() const {
        return mSettings;
    }

    /** @return the source file path. e.g. "file.cpp" */
    const std::string& getSourceFilePath() const;

    /** Is the code C. Used for bailouts */
    bool isC() const {
        return mIsC;
    }

    /** Is the code CPP. Used for bailouts */
    bool isCPP() const {
        return mIsCpp;
    }

    /**
     * Delete all tokens in given token list
     * @param tok token list to delete
     */
    static void deleteTokens(Token *tok);

    void addtoken(std::string str, const unsigned int lineno, const unsigned int fileno, bool split = false);
    void addtoken(const Token *tok, const unsigned int lineno, const unsigned int fileno);

    static void insertTokens(Token *dest, const Token *src, unsigned int n);

    /**
     * Copy tokens.
     * @param dest destination token where copied tokens will be inserted after
     * @param first first token to copy
     * @param last last token to copy
     * @param one_line true=>copy all tokens to the same line as dest. false=>copy all tokens to dest while keeping the 'line breaks'
     * @return new location of last token copied
     */
    static Token *copyTokens(Token *dest, const Token *first, const Token *last, bool one_line = true);

    /**
     * Create tokens from code.
     * The code must be preprocessed first:
     * - multiline strings are not handled.
     * - UTF in the code are not handled.
     * - comments are not handled.
     * @param code input stream for code
     * @param file0 source file name
     */
    bool createTokens(std::istream &code, const std::string& file0 = emptyString);

    void createTokens(const simplecpp::TokenList *tokenList);

    /** Deallocate list */
    void deallocateTokens();

    /** append file name if seen the first time; return its index in any case */
    unsigned int appendFileIfNew(const std::string &fileName);

    /** get first token of list */
    const Token *front() const {
        return mTokensFrontBack.front;
    }
    Token *front() {
        return mTokensFrontBack.front;
    }

    /** get last token of list */
    const Token *back() const {
        return mTokensFrontBack.back;
    }
    Token *back() {
        return mTokensFrontBack.back;
    }

    /**
     * Get filenames (the sourcefile + the files it include).
     * The first filename is the filename for the sourcefile
     * @return vector with filenames
     */
    const std::vector<std::string>& getFiles() const {
        return mFiles;
    }

    std::string getOrigFile(const Token *tok) const;

    /**
     * get filename for given token
     * @param tok The given token
     * @return filename for the given token
     */
    const std::string& file(const Token *tok) const;

    /**
     * Get file:line for a given token
     * @param tok given token
     * @return location for given token
     */
    std::string fileLine(const Token *tok) const;

    /**
    * Calculates a 64-bit checksum of the token list used to compare
    * multiple token lists with each other as quickly as possible.
    */
    unsigned long long calculateChecksum() const;

    /**
     * Create abstract syntax tree.
     */
    void createAst();

    /**
     * Check abstract syntax tree.
     * Throws InternalError on failure
     */
    void validateAst() const;

    /**
     * Verify that the given token is an element of the tokenlist.
     * That method is implemented for debugging purposes.
     * @param[in] tok token to be checked
     * \return true if token was found in tokenlist, false else. In case of nullptr true is returned.
     */
    bool validateToken(const Token* tok) const;

    /**
     * Collapse compound standard types into a single token.
     * unsigned long long int => long _isUnsigned=true,_isLong=true
     */
    void simplifyStdType();

private:

    /** Disable copy constructor, no implementation */
    TokenList(const TokenList &);

    /** Disable assignment operator, no implementation */
    TokenList &operator=(const TokenList &);

    /** Token list */
    TokensFrontBack mTokensFrontBack;

    /** filenames for the tokenized source code (source + included) */
    std::vector<std::string> mFiles;

    /** Original filenames for the tokenized source code (source + included) */
    std::vector<std::string> mOrigFiles;

    /** settings */
    const Settings* mSettings;

    /** File is known to be C/C++ code */
    bool mIsC, mIsCpp;
};

/// @}

//---------------------------------------------------------------------------
#endif // tokenlistH
