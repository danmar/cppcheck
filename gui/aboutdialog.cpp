/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "aboutdialog.h"

AboutDialog::AboutDialog(const QString &version, QWidget *parent)
        : QDialog(parent)
        , mVersion(version)
{
    setWindowTitle(tr("About cppcheck"));
    // Left icon area and right text area
    QHBoxLayout *iconLayout = new QHBoxLayout();
    // Keep icon at the top of the dialog
    QVBoxLayout *iconVLayout = new QVBoxLayout();
    // Texts
    QVBoxLayout *mainLayout = new QVBoxLayout();
    // Keep close button in right
    QHBoxLayout *btnLayout = new QHBoxLayout();

    QIcon *icon = new QIcon(":icon.png");
    QLabel *iconLabel = new QLabel();
    iconLabel->setPixmap(icon->pixmap(QSize(48, 48)));
    QLabel *name = new QLabel(tr("Cppcheck - A tool for static C/C++ code analysis."));
    QLabel *ver = new QLabel(QString(tr("Version %1")).arg(mVersion));
    QLabel *copy = new QLabel(("Copyright (C) 2007-2009 Daniel Marjamäki and cppcheck team."));
    copy->setWordWrap(true);
    QLabel *gpl = new QLabel(tr("This program is licensed under the terms " \
                                "of the GNU General Public License version 3"));
    gpl->setWordWrap(true);
    QString url = "<a href=\"http://cppcheck.wiki.sourceforge.net/\">http://cppcheck.wiki.sourceforge.net/</a>";
    QString homepageText = QString(tr("Visit Cppcheck homepage at %1")).arg(url);
    QLabel *homepage = new QLabel(homepageText);
    homepage->setOpenExternalLinks(true);
    homepage->setWordWrap(true);
    QPushButton *quit = new QPushButton(tr("Close"));

    mainLayout->addWidget(name);
    mainLayout->addWidget(ver);
    mainLayout->addWidget(copy);
    mainLayout->addWidget(gpl);
    mainLayout->addWidget(homepage);
    mainLayout->addStretch();

    mainLayout->addLayout(btnLayout);

    iconVLayout->addWidget(iconLabel);
    iconVLayout->addStretch();
    iconLayout->addLayout(iconVLayout);
    iconLayout->addLayout(mainLayout);

    btnLayout->addStretch();
    btnLayout->addWidget(quit);
    setLayout(iconLayout);

    connect(quit, SIGNAL(clicked()), this, SLOT(close()));
}
