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

#include <QtTest>
#include <QObject>
#include <QFile>
#include <QString>
#include <QByteArray>
#include <sstream>
#include "benchmarksimple.h"
#include "tokenize.h"
#include "token.h"
#include "settings.h"
#include "errorlogger.h"

void BenchmarkSimple::tokenize()
{
    QFile file(QString(SRCDIR) + "/../../data/benchmark/simple.cpp");
    QByteArray data = file.readAll();

    Settings settings;
    settings.debugwarnings = true;

    // tokenize..
    Tokenizer tokenizer(&settings, this);
    std::istringstream istr(data.constData());
    QBENCHMARK {
        tokenizer.tokenize(istr, "test.cpp");
    }
}

void BenchmarkSimple::simplify()
{
    QFile file(QString(SRCDIR) + "/../../data/benchmark/simple.cpp");
    QByteArray data = file.readAll();

    Settings settings;
    settings.debugwarnings = true;

    // tokenize..
    Tokenizer tokenizer(&settings, this);
    std::istringstream istr(data.constData());
    tokenizer.tokenize(istr, "test.cpp");
    QBENCHMARK {
        tokenizer.simplifyTokenList2();
    }
}

void BenchmarkSimple::tokenizeAndSimplify()
{
    QFile file(QString(SRCDIR) + "/../../data/benchmark/simple.cpp");
    QByteArray data = file.readAll();

    Settings settings;
    settings.debugwarnings = true;

    // tokenize..
    Tokenizer tokenizer(&settings, this);
    std::istringstream istr(data.constData());
    QBENCHMARK {
        tokenizer.tokenize(istr, "test.cpp");
        tokenizer.simplifyTokenList2();
    }
}

QTEST_MAIN(BenchmarkSimple)
