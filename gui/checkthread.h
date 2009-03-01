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
#include "../src/errorlogger.h"

/**
* @brief Thread to run cppcheck
*
*/
class CheckThread : public QThread, public ErrorLogger
{
    Q_OBJECT
public:
    CheckThread();
    virtual ~CheckThread();

    /**
    * @brief Set settings for cppcheck
    *
    * @param settings settings for cppcheck
    */
    void SetSettings(Settings settings);

    /**
    * @brief Clear all files from cppcheck
    *
    */
    void ClearFiles();

    /**
    * @brief Add a single file to cppcheck
    *
    * @param file file to add
    */
    void AddFile(const QString &file);

    /**
    * @brief method that is run in a thread
    *
    */
    void run();

    /**
    * ErrorLogger methods
    */
    void reportOut(const std::string &outmsg);
    void reportErr(const ErrorLogger::ErrorMessage &msg);
    void reportStatus(unsigned int index, unsigned int max);

signals:
    /**
    * @brief Currently processed file
    *
    * @param filename filename
    */
    void CurrentFile(const QString &filename);

    /**
    * @brief Cppcheck progress
    *
    * @param value progress
    * @param max maximum progress
    */
    void Progress(int value, int max);

    /**
    * @brief Error in file
    *
    * @param filename filename
    * @param severity error's severity
    * @param message error message
    */
    void Error(const QString &filename,
               const QString &severity,
               const QString &message);

    /**
    * @brief cpp checking is done
    *
    */
    void Done();
protected:

    /**
    * @brief CppCheck itself
    *
    */
    CppCheck mCppCheck;
private:
};

#endif // CHECKTHREAD_H
