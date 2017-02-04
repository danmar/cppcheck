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


#ifndef THREADHANDLER_H
#define THREADHANDLER_H

#include <QObject>
#include <QStringList>
#include <QDateTime>
#include <set>
#include "threadresult.h"
#include "importproject.h"

class ResultsView;
class CheckThread;
class QSettings;
class Settings;

/// @addtogroup GUI
/// @{


/**
* @brief This class handles creating threadresult and starting threads
*
*/
class ThreadHandler : public QObject {
    Q_OBJECT
public:
    explicit ThreadHandler(QObject *parent = 0);
    virtual ~ThreadHandler();

    /**
    * @brief Set the number of threads to use
    * @param count The number of threads to use
    */
    void SetThreadCount(const int count);

    /**
    * @brief Initialize the threads (connect all signals to resultsview's slots)
    *
    * @param view View to show error results
    */
    void Initialize(ResultsView *view);

    /**
    * @brief Load settings
    * @param settings QSettings to load settings from
    */
    void LoadSettings(QSettings &settings);

    /**
    * @brief Save settings
    * @param settings QSettings to save settings to
    */
    void SaveSettings(QSettings &settings) const;

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

    /**
    * @brief Set project to check
    *
    * @param prj project to check
    */
    void SetProject(const ImportProject &prj);

    /**
    * @brief Start the threads to check the files
    *
    * @param settings Settings for checking
    * @param all true if all files, false if modified files
    */
    void Check(const Settings &settings, bool all);

    /**
    * @brief Set files to check
    *
    * @param all true if all files, false if modified files
    */
    void SetCheckFiles(bool all);

    /**
    * @brief Set selected files to check
    *
    * @param files list of files to be checked
    */
    void SetCheckFiles(QStringList files);

    /**
    * @brief Is checking running?
    *
    * @return true if check is running, false otherwise.
    */
    bool IsChecking() const;

    /**
    * @brief Have we checked files already?
    *
    * @return true check has been previously run and recheck can be done
    */
    bool HasPreviousFiles() const;

    /**
    * @brief Return count of files we checked last time.
    *
    * @return count of files that were checked last time.
    */
    int GetPreviousFilesCount() const;

    /**
    * @brief Return the time elapsed while scanning the previous time.
    *
    * @return the time elapsed in milliseconds.
    */
    int GetPreviousScanDuration() const;

    /**
     * @brief Get files that should be rechecked because they have been
     * changed.
     */
    QStringList GetReCheckFiles(bool all) const;

    /**
    * @brief Get start time of last check
    *
    * @return start time of last check
    */
    QDateTime GetCheckStartTime() const;

    /**
    * @brief Set start time of check
    *
    * @param checkStartTime saved start time of the last check
    */
    void SetCheckStartTime(QDateTime checkStartTime);

signals:
    /**
    * @brief Signal that all threads are done
    *
    */
    void Done();

public slots:

    /**
    * @brief Slot to stop all threads
    *
    */
    void Stop();
protected slots:



    /**
    * @brief Slot that a single thread is done
    *
    */
    void ThreadDone();
protected:
    /**
    * @brief List of files checked last time (used when rechecking)
    *
    */
    QStringList mLastFiles;

    /** @brief date and time when current checking started */
    QDateTime mCheckStartTime;

    /**
     * @brief when was the files checked the last time (used when rechecking)
     */
    QDateTime mLastCheckTime;

    /**
    * @brief Timer used for measuring scan duration
    *
    */
    QTime mTime;

    /**
    * @brief The previous scan duration in milliseconds.
    *
    */
    int mScanDuration;

    /**
    * @brief Function to delete all threads
    *
    */
    void RemoveThreads();

    /**
    * @brief Thread results are stored here
    *
    */
    ThreadResult mResults;

    /**
    * @brief List of threads currently in use
    *
    */
    QList<CheckThread *> mThreads;

    /**
    * @brief The amount of threads currently running
    *
    */
    int mRunningThreadCount;

    bool mAnalyseWholeProgram;
private:

    /**
     * @brief Check if a file needs to be rechecked. Recursively checks
     * included headers. Used by GetReCheckFiles()
     */
    bool NeedsReCheck(const QString &filename, std::set<QString> &modified, std::set<QString> &unmodified) const;
};
/// @}
#endif // THREADHANDLER_H
