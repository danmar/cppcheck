/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam,
 * Leandro Penz, Kimmo Varis, Vesa Pikki
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


#ifndef THREADHANDLER_H
#define THREADHANDLER_H

#include <QObject>
#include <QStringList>
#include "../src/cppcheck.h"
#include "threadresult.h"
#include "checkthread.h"
#include "resultsview.h"

/**
* @brief This class handles creating threadresult and starting threads
*
*/
class ThreadHandler : public QObject
{
    Q_OBJECT
public:
    ThreadHandler();
    virtual ~ThreadHandler();
    void SetThreadCount(const int count);
    void Initialize(ResultsView *view);
    void LoadSettings(QSettings &settings);
    void SaveSettings(QSettings &settings);

    /**
    * @brief Clear all files from cppcheck
    *
    */
    void ClearFiles();

    /**
    * @brief Set files to check
    *
    * @param files files to check
    */
    void SetFiles(const QStringList &files);

    void Check(Settings settings);


signals:
    void Done();
protected slots:
    void Stop();
    void ThreadDone();
protected:
    void RemoveThreads();
    ThreadResult mResults;
    QList<CheckThread *> mThreads;
    int mRunningThreadCount;
private:
};

#endif // THREADHANDLER_H
