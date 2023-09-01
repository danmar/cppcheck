/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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

#include "cppcheck.h"
#include "importproject.h"
#include "suppressions.h"

#include <atomic>

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QThread>

class Settings;
class ThreadResult;

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

    void setClangIncludePaths(const QStringList &s) {
        mClangIncludePaths = s;
    }

    void setSuppressions(const QList<Suppressions::Suppression> &s) {
        mSuppressions = s;
    }

    /**
     * @brief method that is run in a thread
     *
     */
    void run() override;

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

signals:

    /**
     * @brief cpp checking is done
     *
     */
    void done();

    // NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name) - caused by generated MOC code
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
     * @brief Thread's current execution state. Can be changed from outside
     */
    std::atomic<State> mState{Ready};

    ThreadResult &mResult;
    /**
     * @brief Cppcheck itself
     */
    CppCheck mCppcheck;

private:
    void runAddonsAndTools(const ImportProject::FileSettings *fileSettings, const QString &fileName);

    void parseClangErrors(const QString &tool, const QString &file0, QString err);

    bool isSuppressed(const Suppressions::ErrorMessage &errorMessage) const;

    QStringList mFiles;
    bool mAnalyseWholeProgram{};
    QStringList mAddonsAndTools;
    QStringList mClangIncludePaths;
    QList<Suppressions::Suppression> mSuppressions;
};
/// @}
#endif // CHECKTHREAD_H
