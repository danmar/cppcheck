/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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


#ifndef errorloggerH
#define errorloggerH

#include <list>
#include <string>

class Token;
class Tokenizer;

/// @addtogroup Core
/// @{

/** @brief enum class for severity. Used when reporting errors. */
class Severity
{
public:
    enum SeverityType { none, error, warning, style, performance, debug };
    static std::string toString(SeverityType severity)
    {
        switch (severity)
        {
        case none:
            return "";
        case error:
            return "error";
        case warning:
            return "warning";
        case style:
            return "style";
        case performance:
            return "performance";
        case debug:
            return "debug";
        };
        return "???";
    }
    static SeverityType fromString(const std::string &severity)
    {
        if (severity.empty())
            return none;
        if (severity == "none")
            return none;
        if (severity == "error")
            return error;
        if (severity == "warning")
            return warning;
        if (severity == "style")
            return style;
        if (severity == "performance")
            return performance;
        if (severity == "debug")
            return debug;
        return none;
    }
};

/**
 * @brief This is an interface, which the class responsible of error logging
 * should implement.
 */
class ErrorLogger
{
public:

    /**
     * Wrapper for error messages, provided by reportErr()
     */
    class ErrorMessage
    {
    public:
        /**
         * File name and line number.
         * Internally paths are stored with / separator. When getting the filename
         * it is by default converted to native separators.
         */
        class FileLocation
        {
        public:
            FileLocation()
            {
                line = 0;
            }

            /**
             * Return the filename.
             * @param convert If true convert path to native separators.
             * @return filename.
             */
            std::string getfile(bool convert = true) const;

            /**
             * Set the filename.
             * @param file Filename to set.
             */
            void setfile(const std::string &file);
            unsigned int line;
        private:
            std::string _file;

        };

        ErrorMessage(const std::list<FileLocation> &callStack, Severity::SeverityType severity, const std::string &msg, const std::string &id);
        ErrorMessage();
        std::string toXML() const;

        static std::string getXMLHeader();
        static std::string getXMLFooter();

        /**
         * Format the error message into a string.
         * @param outputFormat Empty string to use default output format
         * or template to be used. E.g. "{file}:{line},{severity},{id},{message}"
         */
        std::string toString(const std::string &outputFormat = "") const;

        /**
         * Replace all occurances of searchFor with replaceWith in the
         * given source.
         * @param source The string to modify
         * @param searchFor What should be searched for
         * @param replaceWith What will replace the found item
         */
        static void findAndReplace(std::string &source, const std::string &searchFor, const std::string &replaceWith);
        std::string serialize() const;
        bool deserialize(const std::string &data);
        std::list<FileLocation> _callStack;
        Severity::SeverityType _severity;
        std::string _msg;
        std::string _id;

        /** source file (not header) */
        std::string file0;
    };

    ErrorLogger() { }
    virtual ~ErrorLogger() { }

    /**
     * Information about progress is directed here.
     * Override this to receive the progress messages.
     *
     * @param outmsg Message to show e.g. "Checking main.cpp..."
     */
    virtual void reportOut(const std::string &outmsg) = 0;

    /**
     * Information about found errors and warnings is directed
     * here. Override this to receive the errormessages.
     *
     * @param msg Location and other information about the found.
     * error
     */
    virtual void reportErr(const ErrorLogger::ErrorMessage &msg) = 0;

    /**
     * Information about how many files have been checked
     *
     * @param index This many files have been checked.
     * @param max This many files there are in total.
     */
    virtual void reportStatus(unsigned int index, unsigned int max) = 0;

    /**
     * Report progress to client
     * @param filename main file that is checked
     * @param stage for example preprocess / tokenize / simplify / check
     * @param value progress value (0-100)
     */
    virtual void reportProgress(const std::string &filename, const char stage[], const unsigned int value)
    {
        (void)filename;
        (void)stage;
        (void)value;
    }

    static std::string callStackToString(const std::list<ErrorLogger::ErrorMessage::FileLocation> &callStack);
};


/// @}

#endif
