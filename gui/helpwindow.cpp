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

#include "helpwindow.h"
#include "ui_helpwindow.h"

#include <QtHelp>
#include <QtHelp/QHelpEngine>
#include <QTextBrowser>
#include <QSplitter>

/** @brief Help browser */
class HelpBrowser : public QTextBrowser
{
public:
    HelpBrowser(QHelpEngine *helpEngine, QWidget *parent = 0)
        : QTextBrowser(parent), helpEngine(helpEngine)
    {
    }

    QVariant loadResource(int type, const QUrl &url)
    {
        if (url.scheme() == "qthelp")
            return QVariant(helpEngine->fileData(url));
        else
            return QTextBrowser::loadResource(type, url);
    }

private:
    QHelpEngine *helpEngine;
};



HelpWindow::HelpWindow(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::HelpWindow)
{
    m_ui->setupUi(this);
    helpEngine = NULL;
}

bool HelpWindow::load(const QString &helpFile)
{
    helpEngine = new QHelpEngine(helpFile, this);
    if (!helpEngine->setupData())
    {
        return false;
    }

    QSplitter *helpPanel = new QSplitter(Qt::Horizontal);
    HelpBrowser *helpBrowser = new HelpBrowser(helpEngine, this);

    helpPanel->insertWidget(0, helpEngine->contentWidget());
    helpPanel->insertWidget(1, helpBrowser);
    helpPanel->setStretchFactor(1, 1);

    m_ui->mainLayout->addWidget(helpPanel);

    connect(helpEngine->contentWidget(), SIGNAL(linkActivated(const QUrl &)),
            helpBrowser, SLOT(setSource(const QUrl &)));

    connect(m_ui->backButton, SIGNAL(clicked()), helpBrowser, SLOT(backward()));
    connect(m_ui->forwardButton, SIGNAL(clicked()), helpBrowser, SLOT(forward()));
    connect(m_ui->homeButton, SIGNAL(clicked()), helpBrowser, SLOT(home()));
    //connect(m_ui->zoomInButton, SIGNAL(clicked()), helpBrowser, SLOT(zoomIn()));
    //connect(m_ui->zoomOutButton, SIGNAL(clicked()), helpBrowser, SLOT(zoomOut()));

    return true;
}

HelpWindow::~HelpWindow()
{
    delete helpEngine;
    delete m_ui;
}

