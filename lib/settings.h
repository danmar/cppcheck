/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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

#ifndef settingsH
#define settingsH

#include <list>
#include <string>
#include <istream>
#include <map>
#include <set>

/// @addtogroup Core
/// @{


/**
 * @brief This is just a container for general settings so that we don't need
 * to pass individual values to functions or constructors now or in the
 * future when we might have even more detailed settings.
 */
class Settings
{
private:
    /** @brief Code to append in the checks */
    std::string _append;

    /** @brief enable extra checks by id */
    std::set<std::string> _enabled;

    /** @brief terminate checking */
    bool _terminate;

public:
    Settings();

    /** @brief Is --debug given? */
    bool debug;

    /** @brief Is --debug-warnings given? */
    bool debugwarnings;

    /** @brief Is --debug-fp given? */
    bool debugFalsePositive;

    /** @brief Inconclusive checks */
    bool inconclusive;

    /**
     * When this flag is false (default) then experimental
     * heuristics and checks are disabled.
     *
     * It should not be possible to enable this from any client.
     */
    bool experimental;

    /** @brief Is --quiet given? */
    bool _errorsOnly;

    /** @brief Is --inline-suppr given? */
    bool _inlineSuppressions;

    /** @brief Is --verbose given? */
    bool _verbose;

    /** @brief Request termination of checking */
    void terminate()
    {
        _terminate = true;
    }

    /** @brief termination requested? */
    bool terminated() const
    {
        return _terminate;
    }

    /** @brief Force checking the files with "too many" configurations (--force). */
    bool _force;

    /** @brief write XML results (--xml) */
    bool _xml;

    /** @brief XML version (--xmlver=..) */
    int _xml_version;

    /** @brief How many processes/threads should do checking at the same
        time. Default is 1. (-j N) */
    unsigned int _jobs;

    /** @brief If errors are found, this value is returned from main().
        Default value is 0. */
    int _exitCode;

    /** @brief The output format in which the errors are printed in text mode,
        e.g. "{severity} {file}:{line} {message} {id}" */
    std::string _outputFormat;

    /** @brief show timing information (--showtime=file|summary|top5) */
    unsigned int _showtime;

    /** @brief List of include paths, e.g. "my/includes/" which should be used
        for finding include files inside source files. (-I) */
    std::list<std::string> _includePaths;

    /** @brief assign append code (--append) */
    void append(const std::string &filename);

    /** @brief get append code (--append) */
    std::string append() const;

    /**
     * @brief Returns true if given id is in the list of
     * enabled extra checks (--enable)
     * @param str id for the extra check, e.g. "style"
     * @return true if the check is enabled.
     */
    bool isEnabled(const std::string &str) const;

    /**
     * @brief Enable extra checks by id. See isEnabled()
     * @param str single id or list of id values to be enabled
     * or empty string to enable all. e.g. "style,possibleError"
     * @return error message. empty upon success
     */
    std::string addEnabled(const std::string &str);

    /** @brief class for handling suppressions */
    class Suppressions
    {
    private:
        class FileMatcher
        {
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
         * @brief Don't show this error. If file and/or line are optional. In which case
         * the errorId alone is used for filtering.
         * @param errorId the id for the error, e.g. "arrayIndexOutOfBounds"
         * @param file File name with the path, e.g. "src/main.cpp"
         * @param line number, e.g. "123"
         * @return error message. empty upon success
         */
        std::string addSuppression(const std::string &errorId, const std::string &file = "", unsigned int line = 0);

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

        struct SuppressionEntry
        {
            SuppressionEntry(const std::string &aid, const std::string &afile, const unsigned int &aline)
                : id(aid), file(afile), line(aline)
            { }

            std::string id;
            std::string file;
            unsigned int line;
        };

        /**
         * @brief Returns list of unmatched local (per-file) suppressions.
         * @return list of unmatched suppressions
         */
        std::list<SuppressionEntry> getUnmatchedLocalSuppressions(const std::string &file) const;

        /**
         * @brief Returns list of unmatched global (glob pattern) suppressions.
         * @return list of unmatched suppressions
         */
        std::list<SuppressionEntry> getUnmatchedGlobalSuppressions() const;
    };

    /** @brief suppress message (--suppressions) */
    Suppressions nomsg;

    /** @brief suppress exitcode */
    Suppressions nofail;

    /** @brief defines given by the user */
    std::string userDefines;

    /** @brief Experimental 2 pass checking of files */
    bool test_2_pass;

    /** @brief --report-progress */
    bool reportProgress;

    /**
     * @brief Is there any preprocessor configurations in the source code?
     * As usual, include guards are not counted.
     */
    bool ifcfg;

    /** Rule */
    class Rule
    {
    public:
        Rule()
        {
            // default id
            id = "rule";

            // default severity
            severity = "style";
        }

        std::string pattern;
        std::string id;
        std::string severity;
        std::string summary;
    };

    /**
     * @brief Extra rules
     */
    std::list<Rule> rules;

    /** Is the 'configuration checking' wanted? */
    bool checkConfiguration;

    /** Code is posix - it is not compatible with non-posix environments */
    bool posix;
};

/// @}

#endif // SETTINGS_H
