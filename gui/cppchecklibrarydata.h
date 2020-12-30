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

#ifndef CPPCHECKLIBRARYDATA_H
#define CPPCHECKLIBRARYDATA_H

#include <QList>
#include <QString>
#include <QStringList>
#include <QIODevice>

class CppcheckLibraryData {
public:
    CppcheckLibraryData();

    struct Container {
        Container() : access_arrayLike(false), size_templateParameter(-1) {}

        QString id;
        QString inherits;
        QString startPattern;
        QString endPattern;
        QString opLessAllowed;
        QString itEndPattern;

        bool access_arrayLike;
        int  size_templateParameter;

        struct {
            QString templateParameter;
            QString string;
        } type;

        struct Function {
            QString name;
            QString yields;
            QString action;
        };
        QList<struct Function> accessFunctions;
        QList<struct Function> otherFunctions;
        QList<struct Function> sizeFunctions;
    };

    struct Define {
        QString name;
        QString value;
    };

    struct Function {
        Function() : noreturn(Unknown), gccPure(false), gccConst(false),
            leakignore(false), useretval(false) {
        }

        QString comments;
        QString name;
        enum TrueFalseUnknown { False, True, Unknown } noreturn;
        bool gccPure;
        bool gccConst;
        bool leakignore;
        bool useretval;
        struct ReturnValue {
            ReturnValue() : container(-1) {}
            QString type;
            QString value;
            int container;
            bool empty() const {
                return type.isNull() && value.isNull() && container < 0;
            }
        } returnValue;
        struct {
            QString scan;
            QString secure;
        } formatstr;
        struct Arg {
            Arg() : nr(0), notbool(false), notnull(false), notuninit(false),
                formatstr(false), strz(false) {
            }

            QString name;
            unsigned int nr;
            static const unsigned int ANY;
            static const unsigned int VARIADIC;
            QString defaultValue;
            bool notbool;
            bool notnull;
            bool notuninit;
            bool formatstr;
            bool strz;
            QString valid;
            struct MinSize {
                QString type;
                QString arg;
                QString arg2;
            };
            QList<struct MinSize> minsizes;
            struct Iterator {
                Iterator() : container(-1) {}
                int container;
                QString type;
            } iterator;
        };
        QList<struct Arg> args;

        struct {
            QString severity;
            QString cstd;
            QString reason;
            QString alternatives;
            QString msg;

            bool isEmpty() const {
                return cstd.isEmpty() &&
                       severity.isEmpty() &&
                       reason.isEmpty() &&
                       alternatives.isEmpty() &&
                       msg.isEmpty();
            }
        } warn;
    };

    struct MemoryResource {
        QString type; // "memory" or "resource"
        struct Alloc {
            Alloc() :
                isRealloc(false),
                init(false),
                arg(-1),        // -1: Has no optional "arg" attribute
                reallocArg(-1)  // -1: Has no optional "realloc-arg" attribute
            {}

            bool isRealloc;
            bool init;
            int arg;
            int reallocArg;
            QString bufferSize;
            QString name;
        };
        struct Dealloc {
            Dealloc() :
                arg(-1)        // -1: Has no optional "arg" attribute
            {}

            int arg;
            QString name;
        };

        QList<struct Alloc> alloc;
        QList<struct Dealloc> dealloc;
        QStringList use;
    };

    struct PodType {
        QString name;
        QString size;
        QString sign;
    };

    void clear() {
        containers.clear();
        defines.clear();
        undefines.clear();
        functions.clear();
        memoryresource.clear();
        podtypes.clear();
    }


    void swap(CppcheckLibraryData &other) {
        containers.swap(other.containers);
        defines.swap(other.defines);
        undefines.swap(other.undefines);
        functions.swap(other.functions);
        memoryresource.swap(other.memoryresource);
        podtypes.swap(other.podtypes);
    }

    QString open(QIODevice &file);
    QString toString() const;

    QList<struct Container> containers;
    QList<struct Define> defines;
    QList<struct Function> functions;
    QList<struct MemoryResource> memoryresource;
    QList<struct PodType> podtypes;
    QStringList undefines;
};

#endif // CPPCHECKLIBRARYDATA_H
