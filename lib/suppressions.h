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
#ifndef suppressionsH
#define suppressionsH
//---------------------------------------------------------------------------

#include "config.h"

#include <cstddef>
#include <istream>
#include <list>
#include <string>
#include <utility>
#include <vector>

/// @addtogroup Core
/// @{

class Tokenizer;
class ErrorMessage;
class ErrorLogger;
enum class Certainty;

/** @brief class for handling suppressions */
class CPPCHECKLIB Suppressions {
public:

    struct CPPCHECKLIB ErrorMessage {
        std::size_t hash;
        std::string errorId;
        void setFileName(std::string s);
        const std::string &getFileName() const {
            return mFileName;
        }
        int lineNumber;
        Certainty certainty;
        std::string symbolNames;

        static Suppressions::ErrorMessage fromErrorMessage(const ::ErrorMessage &msg);
    private:
        std::string mFileName;
    };

    struct CPPCHECKLIB Suppression {
        Suppression() = default;
        Suppression(std::string id, std::string file, int line=NO_LINE) : errorId(std::move(id)), fileName(std::move(file)), lineNumber(line) {}

        bool operator<(const Suppression &other) const {
            if (errorId != other.errorId)
                return errorId < other.errorId;
            if (lineNumber < other.lineNumber)
                return true;
            if (fileName != other.fileName)
                return fileName < other.fileName;
            if (symbolName != other.symbolName)
                return symbolName < other.symbolName;
            if (hash != other.hash)
                return hash < other.hash;
            if (thisAndNextLine != other.thisAndNextLine)
                return thisAndNextLine;
            return false;
        }

        /**
         * Parse inline suppression in comment
         * @param comment the full comment text
         * @param errorMessage output parameter for error message (wrong suppression attribute)
         * @return true if it is a inline comment.
         */
        bool parseComment(std::string comment, std::string *errorMessage);

        bool isSuppressed(const ErrorMessage &errmsg) const;

        bool isMatch(const ErrorMessage &errmsg);

        std::string getText() const;

        bool isLocal() const {
            return !fileName.empty() && fileName.find_first_of("?*") == std::string::npos;
        }

        bool isSameParameters(const Suppression &other) const {
            return errorId == other.errorId &&
                   fileName == other.fileName &&
                   lineNumber == other.lineNumber &&
                   symbolName == other.symbolName &&
                   hash == other.hash &&
                   thisAndNextLine == other.thisAndNextLine;
        }

        std::string errorId;
        std::string fileName;
        int lineNumber = NO_LINE;
        std::string symbolName;
        std::size_t hash{};
        bool thisAndNextLine{}; // Special case for backwards compatibility: { // cppcheck-suppress something
        bool matched{};
        bool checked{}; // for inline suppressions, checked or not

        enum { NO_LINE = -1 };
    };

    /**
     * @brief Don't show errors listed in the file.
     * @param istr Open file stream where errors can be read.
     * @return error message. empty upon success
     */
    std::string parseFile(std::istream &istr);

    /**
     * @brief Don't show errors listed in the file.
     * @param filename file name
     * @return error message. empty upon success
     */
    std::string parseXmlFile(const char *filename);

    /**
     * Parse multi inline suppression in comment
     * @param comment the full comment text
     * @param errorMessage output parameter for error message (wrong suppression attribute)
     * @return empty vector if something wrong.
     */
    static std::vector<Suppression> parseMultiSuppressComment(const std::string &comment, std::string *errorMessage);

    /**
     * @brief Don't show the given error.
     * @param line Description of error to suppress (in id:file:line format).
     * @return error message. empty upon success
     */
    std::string addSuppressionLine(const std::string &line);

    /**
     * @brief Don't show this error. File and/or line are optional. In which case
     * the errorId alone is used for filtering.
     * @param suppression suppression details
     * @return error message. empty upon success
     */
    std::string addSuppression(Suppression suppression);

    /**
     * @brief Combine list of suppressions into the current suppressions.
     * @param suppressions list of suppression details
     * @return error message. empty upon success
     */
    std::string addSuppressions(std::list<Suppression> suppressions);

    /**
     * @brief Returns true if this message should not be shown to the user.
     * @param errmsg error message
     * @param global use global suppressions
     * @return true if this error is suppressed.
     */
    bool isSuppressed(const ErrorMessage &errmsg, bool global = true);

    /**
     * @brief Returns true if this message should not be shown to the user.
     * @param errmsg error message
     * @return true if this error is suppressed.
     */
    bool isSuppressed(const ::ErrorMessage &errmsg);

    /**
     * @brief Create an xml dump of suppressions
     * @param out stream to write XML to
     */
    void dump(std::ostream &out) const;

    /**
     * @brief Returns list of unmatched local (per-file) suppressions.
     * @return list of unmatched suppressions
     */
    std::list<Suppression> getUnmatchedLocalSuppressions(const std::string &file, const bool unusedFunctionChecking) const;

    /**
     * @brief Returns list of unmatched global (glob pattern) suppressions.
     * @return list of unmatched suppressions
     */
    std::list<Suppression> getUnmatchedGlobalSuppressions(const bool unusedFunctionChecking) const;

    /**
     * @brief Returns list of all suppressions.
     * @return list of suppressions
     */
    const std::list<Suppression> &getSuppressions() const;

    /**
     * @brief Marks Inline Suppressions as checked if source line is in the token stream
     */
    void markUnmatchedInlineSuppressionsAsChecked(const Tokenizer &tokenizer);

    /**
     * Report unmatched suppressions
     * @param unmatched list of unmatched suppressions (from Settings::Suppressions::getUnmatched(Local|Global)Suppressions)
     * @return true is returned if errors are reported
     */
    static bool reportUnmatchedSuppressions(const std::list<Suppressions::Suppression> &unmatched, ErrorLogger &errorLogger);

private:
    /** @brief List of error which the user doesn't want to see. */
    std::list<Suppression> mSuppressions;
};

/// @}
//---------------------------------------------------------------------------
#endif // suppressionsH
