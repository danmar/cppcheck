/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
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

#include "config.h"
#include "importproject.h"
#include "library.h"
#include "platform.h"
#include "standards.h"
#include "suppressions.h"
#include "timer.h"

#include <algorithm>
#include <atomic>
#include <list>
#include <set>
#include <string>
#include <vector>

namespace ValueFlow {
    class Value;
}

/// @addtogroup Core
/// @{

/**
 * @brief This is just a container for general settings so that we don't need
 * to pass individual values to functions or constructors now or in the
 * future when we might have even more detailed settings.
 */
class CPPCHECKLIB Settings : public cppcheck::Platform {
public:
    enum EnabledGroup {
        WARNING = 0x1,
        STYLE = 0x2,
        PERFORMANCE = 0x4,
        PORTABILITY = 0x8,
        INFORMATION = 0x10,
        UNUSED_FUNCTION = 0x20,
        MISSING_INCLUDE = 0x40,
        INTERNAL = 0x80
    };

private:
    /** @brief enable extra checks by id */
    int mEnabled;

    /** @brief terminate checking */
    static std::atomic<bool> mTerminated;

public:
    Settings();

    /** @brief addons, either filename of python/json file or json data */
    std::list<std::string> addons;

    /** @brief Path to the python interpreter to be used to run addons. */
    std::string addonPython;

    /** @brief Paths used as base for conversion to relative paths. */
    std::vector<std::string> basePaths;

    /** @brief Bug hunting */
    bool bugHunting;

    /** @brief Max time for bug hunting analysis in seconds, after
     * timeout the analysis will just stop. */
    int bugHuntingCheckFunctionMaxTime;

    /** Filename for bug hunting report */
    std::string bugHuntingReport;

    /** @brief --cppcheck-build-dir */
    std::string buildDir;

    /** @brief check all configurations (false if -D or --max-configs is used */
    bool checkAllConfigurations;

    /** Is the 'configuration checking' wanted? */
    bool checkConfiguration;

    /**
     * Check code in the headers, this is on by default but can
     * be turned off to save CPU */
    bool checkHeaders;

    /** Check for incomplete info in library files? */
    bool checkLibrary;

    /** @brief List of selected Visual Studio configurations that should be checks */
    std::list<std::string> checkVsConfigs;

    /** @brief check unknown function return values */
    std::set<std::string> checkUnknownFunctionReturn;

    /** Check unused/uninstantiated templates */
    bool checkUnusedTemplates;

    /** Use Clang */
    bool clang;

    /** Custom Clang executable */
    std::string clangExecutable;

    /** Use clang-tidy */
    bool clangTidy;

    /** @brief include paths excluded from checking the configuration */
    std::set<std::string> configExcludePaths;

    /** @brief Are we running from DACA script? */
    bool daca;

    /** @brief Debug bug hunting */
    bool debugBugHunting;

    /** @brief Is --debug-normal given? */
    bool debugnormal;

    /** @brief Is --debug-simplified given? */
    bool debugSimplified;

    /** @brief Is --debug-template given? */
    bool debugtemplate;

    /** @brief Is --debug-warnings given? */
    bool debugwarnings;

    /** @brief Is --dump given? */
    bool dump;
    std::string dumpFile;

    enum Language {
        None, C, CPP
    };

    /** @brief Name of the language that is enforced. Empty per default. */
    Language enforcedLang;

    /** @brief Is --exception-handling given */
    bool exceptionHandling;

    // argv[0]
    std::string exename;

    /** @brief If errors are found, this value is returned from main().
        Default value is 0. */
    int exitCode;

    /**
     * When this flag is false (default) then experimental
     * heuristics and checks are disabled.
     *
     * It should not be possible to enable this from any client.
     */
    bool experimental;

    /** @brief --file-filter for analyzing special files */
    std::string fileFilter;

    /** @brief Force checking the files with "too many" configurations (--force). */
    bool force;

    std::map<std::string, std::string> functionContracts;

    struct VariableContracts {
        std::string minValue;
        std::string maxValue;
    };
    std::map<std::string, VariableContracts> variableContracts;

    /** @brief List of include paths, e.g. "my/includes/" which should be used
        for finding include files inside source files. (-I) */
    std::list<std::string> includePaths;

    /** @brief Inconclusive checks */
    bool inconclusive;

    /** @brief Is --inline-suppr given? */
    bool inlineSuppressions;

    /** @brief How many processes/threads should do checking at the same
        time. Default is 1. (-j N) */
    unsigned int jobs;

    /** @brief Collect unmatched suppressions in one run.
      * This delays the reporting until all files are checked.
      * It is needed by checks that analyse the whole code base. */
    bool jointSuppressionReport;

    /** @brief --library= */
    std::list<std::string> libraries;

    /** Library */
    Library library;

    /** @brief Load average value */
    int loadAverage;

    /** @brief Maximum number of configurations to check before bailing.
        Default is 12. (--max-configs=N) */
    int maxConfigs;

    /** @brief --max-ctu-depth */
    int maxCtuDepth;

    /** @brief max template recursion */
    int maxTemplateRecursion;

    /** @brief suppress exitcode */
    Suppressions nofail;

    /** @brief suppress message (--suppressions) */
    Suppressions nomsg;

    /** @brief write results (--output-file=&lt;file&gt;) */
    std::string outputFile;

    /** @brief plist output (--plist-output=&lt;dir&gt;) */
    std::string plistOutput;

    /** @brief Using -E for debugging purposes */
    bool preprocessOnly;

    ImportProject project;

    /** @brief Is --quiet given? */
    bool quiet;

    /** @brief Use relative paths in output. */
    bool relativePaths;

    /** @brief --report-progress */
    bool reportProgress;

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

    /** Do not only check how interface is used. Also check that interface is safe. */
    class CPPCHECKLIB SafeChecks {
    public:
        SafeChecks() : classes(false), externalFunctions(false), internalFunctions(false), externalVariables(false) {}

        static const char XmlRootName[];
        static const char XmlClasses[];
        static const char XmlExternalFunctions[];
        static const char XmlInternalFunctions[];
        static const char XmlExternalVariables[];

        void clear() {
            classes = externalFunctions = internalFunctions = externalVariables = false;
        }

        /**
         * Public interface of classes
         * - public function parameters can have any value
         * - public functions can be called in any order
         * - public variables can have any value
         */
        bool classes;

        /**
         * External functions
         * - external functions can be called in any order
         * - function parameters can have any values
         */
        bool externalFunctions;

        /**
         * Experimental: assume that internal functions can be used in any way
         * This is only available in the GUI.
         */
        bool internalFunctions;

        /**
         * Global variables that can be modified outside the TU.
         * - Such variable can have "any" value
         */
        bool externalVariables;
    };

    SafeChecks safeChecks;

    /** @brief show timing information (--showtime=file|summary|top5) */
    SHOWTIME_MODES showtime;

    /** Struct contains standards settings */
    Standards standards;

    /** @brief The output format in which the errors are printed in text mode,
        e.g. "{severity} {file}:{line} {message} {id}" */
    std::string templateFormat;

    /** @brief The output format in which the error locations are printed in
     *  text mode, e.g. "{file}:{line} {info}" */
    std::string templateLocation;

    /** @brief defines given by the user */
    std::string userDefines;

    /** @brief undefines given by the user */
    std::set<std::string> userUndefs;

    /** @brief forced includes given by the user */
    std::list<std::string> userIncludes;

    /** @brief Is --verbose given? */
    bool verbose;

    /** @brief write XML results (--xml) */
    bool xml;

    /** @brief XML version (--xml-version=..) */
    int xml_version;

    /**
     * @brief return true if a included file is to be excluded in Preprocessor::getConfigs
     * @return true for the file to be excluded.
     */
    bool configurationExcluded(const std::string &file) const {
        for (const std::string & configExcludePath : configExcludePaths) {
            if (file.length()>=configExcludePath.length() && file.compare(0,configExcludePath.length(),configExcludePath)==0) {
                return true;
            }
        }
        return false;
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
        mEnabled = 0;
    }

    /**
     * @brief Returns true if given id is in the list of
     * enabled extra checks (--enable)
     * @param group group to be enabled
     * @return true if the check is enabled.
     */
    bool isEnabled(EnabledGroup group) const {
        return (mEnabled & group) == group;
    }

    /**
    * @brief Returns true if given severity is enabled
    * @return true if the check is enabled.
    */
    bool isEnabled(Severity::SeverityType severity) const;

    /**
    * @brief Returns true if given value can be shown
    * @return true if the value can be shown
    */
    bool isEnabled(const ValueFlow::Value *value, bool inconclusiveCheck=false) const;

    /** Is posix library specified? */
    bool posix() const {
        return std::find(libraries.begin(), libraries.end(), "posix") != libraries.end();
    }

    /** @brief Request termination of checking */
    static void terminate(bool t = true) {
        Settings::mTerminated = t;
    }

    /** @brief termination requested? */
    static bool terminated() {
        return Settings::mTerminated;
    }

    std::set<std::string> summaryReturn;

    void loadSummaries();
};

/// @}
//---------------------------------------------------------------------------
#endif // settingsH
