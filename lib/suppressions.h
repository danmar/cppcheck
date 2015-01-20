/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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

#include <list>
#include <string>
#include <istream>
#include <map>
#include "config.h"

/// @addtogroup Core
/// @{

/** @brief class for handling suppressions */
class CPPCHECKLIB Suppressions {
private:
    class CPPCHECKLIB FileMatcher {
        friend class Suppressions;
    private:
        /** @brief List of filenames suppressed, bool flag indicates whether suppression matched. */
        std::map<std::string, std::map<unsigned int, bool> > _files;
        /** @brief List of globs suppressed, bool flag indicates whether suppression matched. */
        std::map<std::string, std::map<unsigned int, bool> > _globs;

        /**
         * @brief Match a name against a glob pattern.
         * @param pattern The glob pattern to match.
         * @param name The filename to match against the glob pattern.
         * @return match success
         */
        static bool match(const std::string &pattern, const std::string &name);

    public:
        /**
         * @brief Add a file or glob (and line number).
         * @param name File name or glob pattern
         * @param line Line number
         * @return error message. empty upon success
         */
        std::string addFile(const std::string &name, unsigned int line);

        /**
         * @brief Returns true if the file name matches a previously added file or glob pattern.
         * @param file File name to check
         * @param line Line number
         * @return true if this filename/line matches
         */
        bool isSuppressed(const std::string &file, unsigned int line);

        /**
         * @brief Returns true if the file name matches a previously added file (only, not glob pattern).
         * @param file File name to check
         * @param line Line number
         * @return true if this filename/line matches
         */
        bool isSuppressedLocal(const std::string &file, unsigned int line);
    };

    /** @brief List of error which the user doesn't want to see. */
    std::map<std::string, FileMatcher> _suppressions;
public:
    /**
     * @brief Don't show errors listed in the file.
     * @param istr Open file stream where errors can be read.
     * @return error message. empty upon success
     */
    std::string parseFile(std::istream &istr);

    /**
     * @brief Don't show the given error.
     * @param line Description of error to suppress (in id:file:line format).
     * @return error message. empty upon success
     */
    std::string addSuppressionLine(const std::string &line);

    /**
     * @brief Don't show this error. File and/or line are optional. In which case
     * the errorId alone is used for filtering.
     * @param errorId the id for the error, e.g. "arrayIndexOutOfBounds"
     * @param file File name with the path, e.g. "src/main.cpp"
     * @param line number, e.g. "123"
     * @return error message. empty upon success
     */
    std::string addSuppression(const std::string &errorId, const std::string &file = emptyString, unsigned int line = 0);

    /**
     * @brief Returns true if this message should not be shown to the user.
     * @param errorId the id for the error, e.g. "arrayIndexOutOfBounds"
     * @param file File name with the path, e.g. "src/main.cpp"
     * @param line number, e.g. "123"
     * @return true if this error is suppressed.
     */
    bool isSuppressed(const std::string &errorId, const std::string &file, unsigned int line);

    /**
     * @brief Returns true if this message should not be shown to the user (explicit files only, not glob patterns).
     * @param errorId the id for the error, e.g. "arrayIndexOutOfBounds"
     * @param file File name with the path, e.g. "src/main.cpp"
     * @param line number, e.g. "123"
     * @return true if this error is suppressed.
     */
    bool isSuppressedLocal(const std::string &errorId, const std::string &file, unsigned int line);

    struct SuppressionEntry {
        SuppressionEntry(const std::string &aid, const std::string &afile, unsigned int aline)
            : id(aid), file(afile), line(aline) {
        }

        std::string id;
        std::string file;
        unsigned int line;
    };

    /**
     * @brief Returns list of unmatched local (per-file) suppressions.
     * @return list of unmatched suppressions
     */
    std::list<SuppressionEntry> getUnmatchedLocalSuppressions(const std::string &file, const bool unusedFunctionChecking) const;

    /**
     * @brief Returns list of unmatched global (glob pattern) suppressions.
     * @return list of unmatched suppressions
     */
    std::list<SuppressionEntry> getUnmatchedGlobalSuppressions(const bool unusedFunctionChecking) const;
};

/// @}
//---------------------------------------------------------------------------
#endif // suppressionsH
