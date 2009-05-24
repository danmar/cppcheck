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


#ifndef CHECKTHREAD_H
#define CHECKTHREAD_H

#include <QThread>
#include "../src/cppcheck.h"
#include "../src/settings.h"
#include "threadresult.h"

/**
* @brief Thread to run cppcheck
*
*/
class CheckThread : public QThread
{
    Q_OBJECT
public:
    CheckThread(ThreadResult &result);
    virtual ~CheckThread();

    /**
    * @brief Set settings for cppcheck
    *
    * @param settings settings for cppcheck
    */
    void Check(Settings settings);

    /**
    * @brief method that is run in a thread
    *
    */
    void run();


signals:

    /**
    * @brief cpp checking is done
    *
    */
    void Done();

    void FileChecked(const QString &file);
protected:
    ThreadResult &mResult;
    /**
    * @brief Cppcheck itself
    *
    */
    CppCheck mCppcheck;
private:
};

#endif // CHECKTHREAD_H
