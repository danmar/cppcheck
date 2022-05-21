/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2022 Cppcheck team.
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

#include "aboutdialog.h"

#include "ui_about.h"

#include <QDialogButtonBox>
#include <QLabel>

AboutDialog::AboutDialog(const QString &version, const QString &extraVersion, QWidget *parent)
    : QDialog(parent)
    , mUI(new Ui::About)
{
    mUI->setupUi(this);

    QString fmtVersion(version);
    if (!extraVersion.isEmpty()) {
        fmtVersion += " (" + extraVersion + ")";
    }
    mUI->mVersion->setText(mUI->mVersion->text().arg(fmtVersion));
    QString date = __DATE__;
    mUI->mCopyright->setText(mUI->mCopyright->text().arg(date.right(4)));
    QString url = "<a href=\"https://cppcheck.sourceforge.io/\">https://cppcheck.sourceforge.io/</a>";
    mUI->mHomepage->setText(mUI->mHomepage->text().arg(url));
    connect(mUI->mButtons, &QDialogButtonBox::accepted, this, &AboutDialog::accept);
}

AboutDialog::~AboutDialog()
{
    delete mUI;
}
