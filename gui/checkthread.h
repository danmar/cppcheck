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


#ifndef CHECKTHREAD_H
#define CHECKTHREAD_H

#include <QThread>
#include "cppcheck.h"
#include "threadresult.h"

class Settings;

/// @addtogroup GUI
/// @{

/**
* @brief Thread to run cppcheck
*
*/
class CheckThread : public QThread {
    Q_OBJECT
public:
    explicit CheckThread(ThreadResult &result);
    virtual ~CheckThread();

    /**
    * @brief Set settings for cppcheck
    *
    * @param settings settings for cppcheck
    */
    void check(const Settings &settings);

    /**
    * @brief Run whole program analysis
    * @param files    All files
    */
    void analyseWholeProgram(const QStringList &files);

    void setAddonsAndTools(const QStringList &addonsAndTools) {
        mAddonsAndTools = addonsAndTools;
    }

    void setMisraFile(const QString &misraFile) {
        mMisraFile = misraFile;
    }

    void setDataDir(const QString &dataDir) {
        mDataDir = dataDir;
    }

    void setClangIncludePaths(const QStringList &s) {
        mClangIncludePaths = s;
    }

    void setSuppressions(const QStringList s) {
        mSuppressions = s;
    }

    /**
    * @brief method that is run in a thread
    *
    */
    void run();

    void stop();

    /**
     * Determine command to run clang
     * \return Command to run clang, empty if it is not found
     */
    static QString clangCmd();

    /**
     * Determine command to run clang-tidy
     * \return Command to run clang-tidy, empty if it is not found
     */
    static QString clangTidyCmd();

    /**
     * Determine command to run python
     * \return Command to run python, empty if it is not found
     */
    static QString pythonCmd();

    /**
     * Look for addon and return path
     * \return path to addon if found, empty if it is not found
     */
    static QString getAddonFilePath(const QString &dataDir, const QString &addonFile);

signals:

    /**
    * @brief cpp checking is done
    *
    */
    void done();

    void fileChecked(const QString &file);
protected:

    /**
    * @brief States for the check thread.
    * Whole purpose of these states is to allow stopping of the checking. When
    * stopping we say for the thread (Stopping) that stop when current check
    * has been completed. Thread must be stopped cleanly, just terminating thread
    * likely causes unpredictable side-effects.
    */
    enum State {
        Running, /**< The thread is checking. */
        Stopping, /**< The thread will stop after current work. */
        Stopped, /**< The thread has been stopped. */
        Ready, /**< The thread is ready. */
    };

    /**
    * @brief Thread's current execution state.
    */
    State mState;

    ThreadResult &mResult;
    /**
    * @brief Cppcheck itself
    */
    CppCheck mCppcheck;

private:
    void runAddonsAndTools(const ImportProject::FileSettings *fileSettings, const QString &fileName);

    void parseAddonErrors(QString err, QString tool);
    void parseClangErrors(const QString &tool, const QString &file0, QString err);

    QStringList mFiles;
    bool mAnalyseWholeProgram;
    QStringList mAddonsAndTools;
    QString mDataDir;
    QStringList mClangIncludePaths;
    QStringList mSuppressions;
    QString mMisraFile;
};
/// @}
#endif // CHECKTHREAD_H
