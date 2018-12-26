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
#ifndef ctuH
#define ctuH
//---------------------------------------------------------------------------

#include "check.h"
#include "valueflow.h"

/// @addtogroup Core
/// @{


/** @brief Whole program analysis (ctu=Cross Translation Unit) */
namespace CTU {
    class FileInfo : public Check::FileInfo {
    public:
        std::string toString() const override;

        struct Location {
            Location() = default;
            Location(const Tokenizer *tokenizer, const Token *tok);
            Location(const std::string &fileName, unsigned int linenr) : fileName(fileName), linenr(linenr) {}
            std::string fileName;
            unsigned int linenr;
        };

        struct UnsafeUsage {
            UnsafeUsage() = default;
            UnsafeUsage(const std::string &functionId, unsigned int argnr, const std::string &argumentName, const Location &location) : functionId(functionId), argnr(argnr), argumentName(argumentName), location(location) {}
            std::string functionId;
            unsigned int argnr;
            std::string argumentName;
            Location location;
            std::string toString() const;
        };

        struct FunctionCall {
            std::string functionId;
            std::string functionName;
            std::string argumentExpression;
            unsigned int argnr;
            long long argvalue;
            ValueFlow::Value::ValueType valueType;
            Location location;
        };

        struct NestedCall {
            NestedCall() = default;

            NestedCall(const std::string &id_, const std::string &functionName_, unsigned int argnr_, const std::string &fileName, unsigned int linenr)
                : id(id_),
                  functionName(functionName_),
                  argnr(argnr_),
                  argnr2(0) {
                location.fileName = fileName;
                location.linenr   = linenr;
            }

            NestedCall(const Tokenizer *tokenizer, const Scope *scope, unsigned int argnr_, const Token *tok);

            std::string id;
            std::string id2;
            std::string functionName;
            unsigned int argnr;
            unsigned int argnr2;
            Location location;
        };

        std::list<FunctionCall> functionCalls;
        std::list<NestedCall> nestedCalls;

        void loadFromXml(const tinyxml2::XMLElement *xmlElement);
        std::map<std::string, std::list<NestedCall>> getNestedCallsMap() const;

        enum InvalidValueType { null, uninit };

        std::list<ErrorLogger::ErrorMessage::FileLocation> getErrorPath(InvalidValueType invalidValue,
                const UnsafeUsage &unsafeUsage,
                const std::map<std::string, std::list<NestedCall>> &nestedCallsMap,
                const char info[],
                const FunctionCall * * const functionCallPtr) const;
    };

    std::string toString(const std::list<FileInfo::UnsafeUsage> &unsafeUsage);

    std::string getFunctionId(const Tokenizer *tokenizer, const Function *function);

    /** @brief Parse current TU and extract file info */
    FileInfo *getFileInfo(const Tokenizer *tokenizer);

    std::list<FileInfo::UnsafeUsage> loadUnsafeUsageListFromXml(const tinyxml2::XMLElement *xmlElement);
}

/// @}
//---------------------------------------------------------------------------
#endif // ctuH
