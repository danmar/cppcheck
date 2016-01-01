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

#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QTextStream>
#include <QPushButton>
#include "common.h"
#include "logview.h"

LogView::LogView(QWidget *parent)
    : QWidget(parent)
{
    mUI.setupUi(this);
    setWindowFlags(Qt::Tool);

    mUI.mButtonBox->button(QDialogButtonBox::Reset)->setText(tr("Clear"));
    connect(mUI.mButtonBox->button(QDialogButtonBox::Close), SIGNAL(clicked()), this, SLOT(CloseButtonClicked()));
    connect(mUI.mButtonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked()), this, SLOT(ClearButtonClicked()));
    connect(mUI.mButtonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), this, SLOT(SaveButtonClicked()));

    QSettings settings;
    resize(settings.value(SETTINGS_LOG_VIEW_WIDTH, 400).toInt(),
           settings.value(SETTINGS_LOG_VIEW_HEIGHT, 300).toInt());
}

LogView::~LogView()
{
    QSettings settings;
    settings.setValue(SETTINGS_LOG_VIEW_WIDTH, size().width());
    settings.setValue(SETTINGS_LOG_VIEW_HEIGHT, size().height());
}

void LogView::AppendLine(const QString &line)
{
    mUI.mLogEdit->appendPlainText(line);
}

void LogView::CloseButtonClicked()
{
    close();
}

void LogView::ClearButtonClicked()
{
    mUI.mLogEdit->clear();
}

void LogView::SaveButtonClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Log"),
                       "", tr("Text files (*.txt *.log);;All files (*.*)"));
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, tr("Cppcheck"),
                                 tr("Could not open file for writing: \"%1\"").arg(fileName));
            return;
        }

        QTextStream out(&file);
        out << mUI.mLogEdit->toPlainText();
    }
}
