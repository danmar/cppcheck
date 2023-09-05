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
#ifndef settingsH
#define settingsH
//---------------------------------------------------------------------------

#include "config.h"
#include "errortypes.h"
#include "importproject.h"
#include "library.h"
#include "platform.h"
#include "standards.h"
#include "suppressions.h"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <list>
#include <set>
#include <string>
#include <tuple>
#include <vector>
#include <unordered_set>

enum class SHOWTIME_MODES;
namespace ValueFlow {
    class Value;
}

/// @addtogroup Core
/// @{

template<typename T>
class SimpleEnableGroup {
    uint32_t mFlags = 0;
public:
    uint32_t intValue() const {
        return mFlags;
    }
    void clear() {
        mFlags = 0;
    }
    void fill() {
        mFlags = 0xFFFFFFFF;
    }
    bool isEnabled(T flag) const {
        return (mFlags & (1U << (uint32_t)flag)) != 0;
    }
    void enable(T flag) {
        mFlags |= (1U << (uint32_t)flag);
    }
    void enable(SimpleEnableGroup<T> group) {
        mFlags |= group.intValue();
    }
    void disable(T flag) {
        mFlags &= ~(1U << (uint32_t)flag);
    }
    void disable(SimpleEnableGroup<T> group) {
        mFlags &= ~(group.intValue());
    }
    void setEnabled(T flag, bool enabled) {
        if (enabled)
            enable(flag);
        else
            disable(flag);
    }
};


/**
 * @brief This is just a container for general settings so that we don't need
 * to pass individual values to functions or constructors now or in the
 * future when we might have even more detailed settings.
 */
class CPPCHECKLIB WARN_UNUSED Settings {
private:

    /** @brief terminate checking */
    static std::atomic<bool> mTerminated;

public:
    Settings();

    void loadCppcheckCfg();

    /** @brief addons, either filename of python/json file or json data */
    std::unordered_set<std::string> addons;

    /** @brief Path to the python interpreter to be used to run addons. */
    std::string addonPython;

    /** @brief Paths used as base for conversion to relative paths. */
    std::vector<std::string> basePaths;

    /** @brief --cppcheck-build-dir. Always uses / as path separator. No trailing path separator. */
    std::string buildDir;

    /** @brief check all configurations (false if -D or --max-configs is used */
    bool checkAllConfigurations = true;

    /** Is the 'configuration checking' wanted? */
    bool checkConfiguration{};

    /**
     * Check code in the headers, this is on by default but can
     * be turned off to save CPU */
    bool checkHeaders = true;

    /** Check for incomplete info in library files? */
    bool checkLibrary{};

    /** @brief The maximum time in seconds for the checks of a single file */
    int checksMaxTime{};

    /** @brief --checkers-report=<filename> : Generate report of executed checkers */
    std::string checkersReportFilename;

    /** @brief check unknown function return values */
    std::set<std::string> checkUnknownFunctionReturn;

    /** Check unused/uninstantiated templates */
    bool checkUnusedTemplates = true;

    /** Use Clang */
    bool clang{};

    /** Custom Clang executable */
    std::string clangExecutable = "clang";

    /** Use clang-tidy */
    bool clangTidy{};

    /** Internal: Clear the simplecpp non-existing include cache */
    bool clearIncludeCache{};

    /** @brief include paths excluded from checking the configuration */
    std::set<std::string> configExcludePaths;

    /** cppcheck.cfg: Custom product name */
    std::string cppcheckCfgProductName;

    /** cppcheck.cfg: About text */
    std::string cppcheckCfgAbout;

    /** @brief Are we running from DACA script? */
    bool daca{};

    /** @brief Is --debug-normal given? */
    bool debugnormal{};

    /** @brief Is --debug-simplified given? */
    bool debugSimplified{};

    /** @brief Is --debug-template given? */
    bool debugtemplate{};

    /** @brief Is --debug-warnings given? */
    bool debugwarnings{};

    /** @brief Is --dump given? */
    bool dump{};
    std::string dumpFile;

    enum Language {
        None, C, CPP
    };

    /** @brief Name of the language that is enforced. Empty per default. */
    Language enforcedLang{};

    /** @brief Is --exception-handling given */
    bool exceptionHandling{};

    // argv[0]
    std::string exename;

    /** @brief If errors are found, this value is returned from main().
        Default value is 0. */
    int exitCode{};

    /** @brief List of --file-filter for analyzing special files */
    std::vector<std::string> fileFilters;

    /** @brief Force checking the files with "too many" configurations (--force). */
    bool force{};

    /** @brief List of include paths, e.g. "my/includes/" which should be used
        for finding include files inside source files. (-I) */
    std::list<std::string> includePaths;

    /** @brief Is --inline-suppr given? */
    bool inlineSuppressions{};

    /** @brief How many processes/threads should do checking at the same
        time. Default is 1. (-j N) */
    unsigned int jobs = 1;

    /** @brief --library= */
    std::list<std::string> libraries;

    /** Library */
    Library library;

    /** @brief Load average value */
    int loadAverage{};

    /** @brief Maximum number of configurations to check before bailing.
        Default is 12. (--max-configs=N) */
    int maxConfigs = 12;

    /** @brief --max-ctu-depth */
    int maxCtuDepth = 2;

    /** @brief max template recursion */
    int maxTemplateRecursion = 100;

    /** @brief suppress exitcode */
    Suppressions nofail;

    /** @brief suppress message (--suppressions) */
    Suppressions nomsg;

    /** @brief write results (--output-file=&lt;file&gt;) */
    std::string outputFile;

    cppcheck::Platform platform;

    /** @brief Experimental: --performance-valueflow-max-time=T */
    int performanceValueFlowMaxTime = -1;

    /** @brief --performance-valueflow-max-if-count=C */
    int performanceValueFlowMaxIfCount;

    /** @brief max number of sets of arguments to pass to subfuncions in valueflow */
    int performanceValueFlowMaxSubFunctionArgs;

    /** @brief plist output (--plist-output=&lt;dir&gt;) */
    std::string plistOutput;

    /** @brief Extra arguments for Cppcheck Premium addon */
    std::string premiumArgs;

    /** @brief Using -E for debugging purposes */
    bool preprocessOnly{};

    ImportProject project;

    /** @brief Is --quiet given? */
    bool quiet{};

    /** @brief Use relative paths in output. */
    bool relativePaths{};

    /** @brief --report-progress */
    int reportProgress{-1};

    /** Rule */
    struct CPPCHECKLIB Rule {
        std::string tokenlist = "normal"; // use normal tokenlist
        std::string pattern;
        std::string id = "rule"; // default id
        std::string summary;
        Severity::SeverityType severity = Severity::style; // default severity
    };

#ifdef HAVE_RULES
    /**
     * @brief Extra rules
     */
    std::list<Rule> rules;
#endif

    /** Do not only check how interface is used. Also check that interface is safe. */
    struct CPPCHECKLIB SafeChecks {

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
        bool classes{};

        /**
         * External functions
         * - external functions can be called in any order
         * - function parameters can have any values
         */
        bool externalFunctions{};

        /**
         * Experimental: assume that internal functions can be used in any way
         * This is only available in the GUI.
         */
        bool internalFunctions{};

        /**
         * Global variables that can be modified outside the TU.
         * - Such variable can have "any" value
         */
        bool externalVariables{};
    };

    SafeChecks safeChecks;

    SimpleEnableGroup<Severity::SeverityType> severity;
    SimpleEnableGroup<Certainty> certainty;
    SimpleEnableGroup<Checks> checks;

    /** @brief show timing information (--showtime=file|summary|top5) */
    SHOWTIME_MODES showtime{};

    /** Struct contains standards settings */
    Standards standards;

    /** @brief The output format in which the errors are printed in text mode,
        e.g. "{severity} {file}:{line} {message} {id}" */
    std::string templateFormat;

    /** @brief The output format in which the error locations are printed in
     *  text mode, e.g. "{file}:{line} {info}" */
    std::string templateLocation;

    /** @brief The maximum time in seconds for the template instantiation */
    std::size_t templateMaxTime{};

    /** @brief The maximum time in seconds for the typedef simplification */
    std::size_t typedefMaxTime{};

    /** @brief defines given by the user */
    std::string userDefines;

    /** @brief undefines given by the user */
    std::set<std::string> userUndefs;

    /** @brief forced includes given by the user */
    std::list<std::string> userIncludes;

    /** @brief the maximum iterations of valueflow (--valueflow-max-iterations=T) */
    std::size_t valueFlowMaxIterations = 4;

    /** @brief Is --verbose given? */
    bool verbose{};

    /** @brief write XML results (--xml) */
    bool xml{};

    /** @brief XML version (--xml-version=..) */
    int xml_version = 2;

    /**
     * @brief return true if a included file is to be excluded in Preprocessor::getConfigs
     * @return true for the file to be excluded.
     */
    bool configurationExcluded(const std::string &file) const {
        return std::any_of(configExcludePaths.begin(), configExcludePaths.end(), [&file](const std::string& path) {
            return file.length() >= path.length() && file.compare(0, path.length(), path) == 0;
        });
    }

    /**
     * @brief Enable extra checks by id. See isEnabled()
     * @param str single id or list of id values to be enabled
     * or empty string to enable all. e.g. "style,possibleError"
     * @return error message. empty upon success
     */
    std::string addEnabled(const std::string &str);

    /**
     * @brief Disable extra checks by id
     * @param str single id or list of id values to be enabled
     * or empty string to enable all. e.g. "style,possibleError"
     * @return error message. empty upon success
     */
    std::string removeEnabled(const std::string &str);

    /**
     * @brief Returns true if given value can be shown
     * @return true if the value can be shown
     */
    bool isEnabled(const ValueFlow::Value *value, bool inconclusiveCheck=false) const;

    /** Is library specified? */
    bool hasLib(const std::string &lib) const {
        return std::find(libraries.cbegin(), libraries.cend(), lib) != libraries.cend();
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

    bool useSingleJob() const {
        return jobs == 1;
    }

    void setCheckLevelExhaustive();
    void setCheckLevelNormal();

private:
    static std::string parseEnabled(const std::string &str, std::tuple<SimpleEnableGroup<Severity::SeverityType>, SimpleEnableGroup<Checks>> &groups);
    std::string applyEnabled(const std::string &str, bool enable);
};

/// @}
//---------------------------------------------------------------------------
#endif // settingsH
