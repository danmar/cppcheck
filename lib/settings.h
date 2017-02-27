/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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
#ifndef settingsH
#define settingsH
//---------------------------------------------------------------------------

#include <list>
#include <vector>
#include <string>
#include <set>
#include "config.h"
#include "library.h"
#include "platform.h"
#include "importproject.h"
#include "suppressions.h"
#include "standards.h"
#include "errorlogger.h"
#include "timer.h"

/// @addtogroup Core
/// @{

/**
 * @brief This is just a container for general settings so that we don't need
 * to pass individual values to functions or constructors now or in the
 * future when we might have even more detailed settings.
 */
class CPPCHECKLIB Settings : public cppcheck::Platform {
private:
    /** @brief Code to append in the checks */
    std::string _append;

    /** @brief enable extra checks by id */
    std::set<std::string> _enabled;

    /** @brief terminate checking */
    static bool _terminated;

public:
    Settings();

    /** @brief --cppcheck-build-dir */
    std::string buildDir;

    /** @brief Is --debug given? */
    bool debug;

    /** @brief Is --debug-normal given? */
    bool debugnormal;

    /** @brief Is --debug-warnings given? */
    bool debugwarnings;

    /** @brief Is --dump given? */
    bool dump;

    /** @brief Is --exception-handling given */
    bool exceptionHandling;

    /** @brief Inconclusive checks */
    bool inconclusive;

    /** @brief Collect unmatched suppressions in one run.
      * This delays the reporting until all files are checked.
      * It is needed by checks that analyse the whole code base. */
    bool jointSuppressionReport;

    /**
     * When this flag is false (default) then experimental
     * heuristics and checks are disabled.
     *
     * It should not be possible to enable this from any client.
     */
    bool experimental;

    /** @brief Is --quiet given? */
    bool quiet;

    /** @brief Is --inline-suppr given? */
    bool inlineSuppressions;

    /** @brief Is --verbose given? */
    bool verbose;

    /** @brief Request termination of checking */
    static void terminate(bool t = true) {
        Settings::_terminated = t;
    }

    /** @brief termination requested? */
    static bool terminated() {
        return Settings::_terminated;
    }

    /** @brief Force checking the files with "too many" configurations (--force). */
    bool force;

    /** @brief Use relative paths in output. */
    bool relativePaths;

    /** @brief Paths used as base for conversion to relative paths. */
    std::vector<std::string> basePaths;

    /** @brief write XML results (--xml) */
    bool xml;

    /** @brief XML version (--xmlver=..) */
    int xml_version;

    /** @brief How many processes/threads should do checking at the same
        time. Default is 1. (-j N) */
    unsigned int jobs;

    /** @brief Load average value */
    unsigned int loadAverage;

    /** @brief If errors are found, this value is returned from main().
        Default value is 0. */
    int exitCode;

    /** @brief The output format in which the errors are printed in text mode,
        e.g. "{severity} {file}:{line} {message} {id}" */
    std::string outputFormat;

    /** @brief show timing information (--showtime=file|summary|top5) */
    SHOWTIME_MODES showtime;

    /** @brief Using -E for debugging purposes */
    bool preprocessOnly;

    /** @brief List of include paths, e.g. "my/includes/" which should be used
        for finding include files inside source files. (-I) */
    std::list<std::string> includePaths;

    /** @brief assign append code (--append) */
    bool append(const std::string &filename);

    /** @brief get append code (--append) */
    const std::string &append() const;

    /** @brief Maximum number of configurations to check before bailing.
        Default is 12. (--max-configs=N) */
    unsigned int maxConfigs;

    /**
     * @brief Returns true if given id is in the list of
     * enabled extra checks (--enable)
     * @param str id for the extra check, e.g. "style"
     * @return true if the check is enabled.
     */
    template<typename T>
    bool isEnabled(T&& str) const {
        return bool(_enabled.find(str) != _enabled.end());
    }

    /**
     * @brief Enable extra checks by id. See isEnabled()
     * @param str single id or list of id values to be enabled
     * or empty string to enable all. e.g. "style,possibleError"
     * @return error message. empty upon success
     */
    std::string addEnabled(const std::string &str);

    /**
     * @brief Disables all severities, except from error.
     */
    void clearEnabled() {
        _enabled.clear();
    }

    enum Language {
        None, C, CPP
    };

    /** @brief Name of the language that is enforced. Empty per default. */
    Language enforcedLang;

    /** @brief suppress message (--suppressions) */
    Suppressions nomsg;

    /** @brief suppress exitcode */
    Suppressions nofail;

    /** @brief defines given by the user */
    std::string userDefines;

    /** @brief undefines given by the user */
    std::set<std::string> userUndefs;

    /** @brief forced includes given by the user */
    std::list<std::string> userIncludes;

    /** @brief include paths excluded from checking the configuration */
    std::set<std::string> configExcludePaths;


    /** @brief --report-progress */
    bool reportProgress;

    /** Library (--library) */
    Library library;

    /** Rule */
    class CPPCHECKLIB Rule {
    public:
        Rule()
            : tokenlist("simple")         // use simple tokenlist
            , id("rule")                  // default id
            , severity(Severity::style) { // default severity
        }

        std::string tokenlist;
        std::string pattern;
        std::string id;
        std::string summary;
        Severity::SeverityType severity;
    };

    /**
     * @brief Extra rules
     */
    std::list<Rule> rules;

    /** Is the 'configuration checking' wanted? */
    bool checkConfiguration;

    /** Check for incomplete info in library files? */
    bool checkLibrary;

    /** Struct contains standards settings */
    Standards standards;

    ImportProject project;

    /**
     * @brief return true if a included file is to be excluded in Preprocessor::getConfigs
     * @return true for the file to be excluded.
     */
    bool configurationExcluded(const std::string &file) const {
        for (std::set<std::string>::const_iterator i=configExcludePaths.begin(); i!=configExcludePaths.end(); ++i) {
            if (file.length()>=i->length() && file.compare(0,i->length(),*i)==0) {
                return true;
            }
        }
        return false;
    }
};

/// @}
//---------------------------------------------------------------------------
#endif // settingsH
