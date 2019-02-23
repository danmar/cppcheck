/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2019 Cppcheck team.
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
#ifndef ctuH
#define ctuH
//---------------------------------------------------------------------------

#include "check.h"
#include "valueflow.h"

/// @addtogroup Core
/// @{


/** @brief Whole program analysis (ctu=Cross Translation Unit) */
namespace CTU {
    class CPPCHECKLIB FileInfo : public Check::FileInfo {
    public:
        enum InvalidValueType { null, uninit };

        std::string toString() const OVERRIDE;

        struct Location {
            Location() = default;
            Location(const Tokenizer *tokenizer, const Token *tok);
            Location(const std::string &fileName, unsigned int linenr) : fileName(fileName), linenr(linenr) {}
            std::string fileName;
            unsigned int linenr;
        };

        struct UnsafeUsage {
            UnsafeUsage() = default;
            UnsafeUsage(const std::string &myId, unsigned int myArgNr, const std::string &myArgumentName, const Location &location) : myId(myId), myArgNr(myArgNr), myArgumentName(myArgumentName), location(location) {}
            std::string myId;
            unsigned int myArgNr;
            std::string myArgumentName;
            Location location;
            std::string toString() const;
        };

        class CallBase {
        public:
            CallBase() = default;
            CallBase(const std::string &callId, int callArgNr, const std::string &callFunctionName, const Location &loc)
                : callId(callId), callArgNr(callArgNr), callFunctionName(callFunctionName), location(loc)
            {}
            CallBase(const Tokenizer *tokenizer, const Token *callToken);
            virtual ~CallBase() {}
            std::string callId;
            int callArgNr;
            std::string callFunctionName;
            Location location;
        protected:
            std::string toBaseXmlString() const;
            bool loadBaseFromXml(const tinyxml2::XMLElement *xmlElement);
        };

        class FunctionCall : public CallBase {
        public:
            std::string callArgumentExpression;
            MathLib::bigint callArgValue;
            ValueFlow::Value::ValueType callValueType;
            std::vector<ErrorLogger::ErrorMessage::FileLocation> callValuePath;
            bool warning;

            std::string toXmlString() const;
            bool loadFromXml(const tinyxml2::XMLElement *xmlElement);
        };

        class NestedCall : public CallBase {
        public:
            NestedCall() = default;

            NestedCall(const std::string &myId, unsigned int myArgNr, const std::string &callId, unsigned int callArgnr, const std::string &callFunctionName, const Location &location)
                : CallBase(callId, callArgnr, callFunctionName, location),
                  myId(myId),
                  myArgNr(myArgNr) {
            }

            NestedCall(const Tokenizer *tokenizer, const Function *myFunction, const Token *callToken);

            std::string toXmlString() const;
            bool loadFromXml(const tinyxml2::XMLElement *xmlElement);

            std::string myId;
            unsigned int myArgNr;
        };

        std::list<FunctionCall> functionCalls;
        std::list<NestedCall> nestedCalls;

        void loadFromXml(const tinyxml2::XMLElement *xmlElement);
        std::map<std::string, std::list<const CallBase *>> getCallsMap() const;

        std::list<ErrorLogger::ErrorMessage::FileLocation> getErrorPath(InvalidValueType invalidValue,
                const UnsafeUsage &unsafeUsage,
                const std::map<std::string, std::list<const CallBase *>> &callsMap,
                const char info[],
                const FunctionCall * * const functionCallPtr,
                bool warning) const;
    };

    extern int maxCtuDepth;

    CPPCHECKLIB std::string toString(const std::list<FileInfo::UnsafeUsage> &unsafeUsage);

    CPPCHECKLIB std::string getFunctionId(const Tokenizer *tokenizer, const Function *function);

    /** @brief Parse current TU and extract file info */
    CPPCHECKLIB FileInfo *getFileInfo(const Tokenizer *tokenizer);

    CPPCHECKLIB std::list<FileInfo::UnsafeUsage> getUnsafeUsage(const Tokenizer *tokenizer, const Settings *settings, const Check *check, bool (*isUnsafeUsage)(const Check *check, const Token *argtok));

    CPPCHECKLIB std::list<FileInfo::UnsafeUsage> loadUnsafeUsageListFromXml(const tinyxml2::XMLElement *xmlElement);
}

/// @}
//---------------------------------------------------------------------------
#endif // ctuH
