/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2017 Cppcheck team.
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

#include "scratchpad.h"
#include "mainwindow.h"
#include <QWidget>

ScratchPad::ScratchPad(MainWindow& mainWindow)
    : QDialog(&mainWindow)
    , mMainWindow(mainWindow)
{
    mUI.setupUi(this);

    connect(mUI.mCheckButton, &QPushButton::clicked, this, &ScratchPad::checkButtonClicked);
}

void ScratchPad::checkButtonClicked()
{
    QString filename = mUI.lineEdit->text();
    if (filename.isEmpty())
        filename = "test.cpp";
    mMainWindow.analyzeCode(mUI.plainTextEdit->toPlainText(), filename);
}
