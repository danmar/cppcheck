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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "preprocessor.h"
#include "tokenize.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <set>

static void arrayIndex(const Tokenizer &tokenizer, std::set<unsigned int> &errorlines);

static unsigned char readChar(std::istream &istr)
{
    unsigned char ch = (unsigned char)istr.get();

    // Handling of newlines..
    if (ch == '\r')
    {
        ch = '\n';
        if ((char)istr.peek() == '\n')
            (void)istr.get();
    }

    return ch;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    const std::string fileName("../lib/tokenize.cpp");

    setWindowTitle(fileName.c_str());

    Tokenizer tokenizer;

    {
        // Preprocess the file..
        Preprocessor preprocessor;
        std::ifstream fin(fileName.c_str());
        std::string filedata;
        std::list<std::string> configurations;
        std::list<std::string> includePaths;
        preprocessor.preprocess(fin,
                                filedata,
                                configurations,
                                fileName,
                                includePaths);
        filedata = Preprocessor::getcode(filedata, "", fileName, NULL, NULL);

        // Tokenize the preprocessed code..
        std::istringstream istr(filedata);
        tokenizer.tokenize(istr, fileName.c_str(), "");
    }

    // Check the tokens..
    std::set<unsigned int> errorlines;
    arrayIndex(tokenizer, errorlines);

    // show report..
    {
        std::ostringstream report;
        report << std::string(8,' ');
        unsigned int lineno = 1;
        std::ifstream fin(fileName.c_str());
        for (unsigned char c = readChar(fin); fin.good(); c = readChar(fin))
        {
            report << c;
            if (c == '\n')
            {
                ++lineno;
                if (errorlines.find(lineno) != errorlines.end())
                    report << std::string(6,'!') << "  ";
                else
                    report << std::string(8,' ');
            }
        }
        ui->plainTextEdit->setPlainText(QString::fromStdString(report.str()));
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}




/**
 * Check that array indexes are within bounds
 * 1. Locate array access through: [ .. ]
 * 2. Try to determine if index is within bounds.
 * 3. If it fails to determine that the index is within bounds then write warning
 * \param tokenizer The tokenizer
 * \param errout output stream to write warnings to
 */

static void arrayIndex(const Tokenizer &tokenizer, std::set<unsigned int> &errorlines)
{
    for (const Token *tok = tokenizer.tokens(); tok; tok = tok->next())
    {
        // 1. Locate array access through: [ .. ]
        if (tok->fileIndex() == 0 && tok->str() == "[")
        {
            // 2. try to determine if the array index is within bounds

            // array declaration
            if (Token::simpleMatch(tok, "[ ]"))
                continue;
            if (Token::Match(tok->tokAt(-2), "%type% %var% [ %num% ] ;|="))
                continue;

            // 3. If it fails to determine that the index is within bounds then write warning
            errorlines.insert(tok->linenr());
        }
    }
}
