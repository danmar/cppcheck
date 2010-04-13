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


//---------------------------------------------------------------------------
#ifndef classInfoH
#define classInfoH
//---------------------------------------------------------------------------

#include <string>
#include <vector>
#include "token.h"

/// @addtogroup Core
/// @{

/** @brief Holds information about a single class in checked source code. Contains member function and member variable lists. */
class ClassInfo
{
public:

    enum MemberType {PRIVATE, PROTECTED, PUBLIC};

    /**
     * Base class for both member functions and member variables.
     * Contains variable name, token where name was declared and type
     * private/protected/public.
     */
    class MemberInfo
    {
    public:

        MemberInfo() : _declaration(0), _name(), _type(ClassInfo::PRIVATE)
        {

        }

        virtual ~MemberInfo() {}

        const Token *_declaration;
        std::string _name;
        ClassInfo::MemberType _type;
    };

    /**
     * Information about a single member function of a class.
     */
    class MemberFunctionInfo : public MemberInfo
    {
    public:
        MemberFunctionInfo() : _implementation(0)
        {

        }

        const Token *_implementation;
    };

    /**
     * Information about a single member variable of a class.
     */
    class MemberVariableInfo : public MemberInfo
    {
    };

    /**
     * List of member functions
     */
    std::vector<MemberFunctionInfo> _memberFunctions;

    /**
     * List of member variables
     */
    std::vector<MemberVariableInfo> _memberVariables;
};


/// @}

//---------------------------------------------------------------------------
#endif
