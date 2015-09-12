/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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

#ifndef COMMON_H
#define COMMON_H

#include <QString>

/// @addtogroup GUI
/// @{


/**
* QSetting value names
*/

// Window/dialog sizes
#define SETTINGS_WINDOW_MAXIMIZED       "Window maximized"
#define SETTINGS_WINDOW_WIDTH           "Window width"
#define SETTINGS_WINDOW_HEIGHT          "Window height"
#define SETTINGS_LOG_VIEW_WIDTH         "Log/View width"
#define SETTINGS_LOG_VIEW_HEIGHT        "Log/View height"
#define SETTINGS_MAINWND_SPLITTER_STATE "Mainwindow/Vertical splitter state"
#define SETTINGS_CHECK_DIALOG_WIDTH     "Check dialog width"
#define SETTINGS_CHECK_DIALOG_HEIGHT    "Check dialog height"
#define SETTINGS_PROJECT_DIALOG_WIDTH   "Project dialog width"
#define SETTINGS_PROJECT_DIALOG_HEIGHT  "Project dialog height"

// Main window settings
#define SETTINGS_RESULT_COLUMN_WIDTH    "Result column %1 width"
#define SETTINGS_TOOLBARS_MAIN_SHOW     "Toolbars/ShowStandard"
#define SETTINGS_TOOLBARS_VIEW_SHOW     "Toolbars/ShowView"
#define SETTINGS_TOOLBARS_FILTER_SHOW   "Toolbars/ShowFilter"

// Show * states
#define SETTINGS_SHOW_STYLE             "Show style"
#define SETTINGS_SHOW_ERRORS            "Show errors"
#define SETTINGS_SHOW_WARNINGS          "Show warnings"
#define SETTINGS_SHOW_PERFORMANCE       "Show performance"
#define SETTINGS_SHOW_INFORMATION       "Show information"
#define SETTINGS_SHOW_PORTABILITY       "Show portability"

// Standards support
#define SETTINGS_STD_CPP03              "Platform CPP03"
#define SETTINGS_STD_CPP11              "Platform CPP11"
#define SETTINGS_STD_C89                "Platform C89"
#define SETTINGS_STD_C99                "Platform C99"
#define SETTINGS_STD_C11                "Platform C11"
#define SETTINGS_STD_POSIX              "Platform Posix"

// Other settings
#define SETTINGS_CHECK_FORCE            "Check force"
#define SETTINGS_CHECK_THREADS          "Check threads"
#define SETTINGS_SHOW_FULL_PATH         "Show full path"
#define SETTINGS_SHOW_NO_ERRORS         "Show no errors message"
#define SETTINGS_SHOW_DEBUG_WARNINGS    "Show debug warnings"
#define SETTINGS_SAVE_ALL_ERRORS        "Save all errors"
#define SETTINGS_SAVE_FULL_PATH         "Save full path"
#define SETTINGS_APPLICATION_NAMES      "Application names"
#define SETTINGS_APPLICATION_PATHS      "Application paths"
#define SETTINGS_APPLICATION_PARAMS     "Application parameters"
#define SETTINGS_APPLICATION_DEFAULT    "Default Application"
#define SETTINGS_LANGUAGE               "Application language"
#define SETTINGS_GLOBAL_INCLUDE_PATHS   "Global include paths"
#define SETTINGS_INLINE_SUPPRESSIONS    "Inline suppressions"
#define SETTINGS_INCONCLUSIVE_ERRORS    "Inconclusive errors"
#define SETTINGS_MRU_PROJECTS           "MRU Projects"
#define SETTINGS_SHOW_ERROR_ID          "Show error Id"

// The maximum value for the progress bar
#define PROGRESS_MAX                    1024.0

#define SETTINGS_CHECKED_PLATFORM       "Checked platform"

#define SETTINGS_LAST_CHECK_PATH        "Last check path"
#define SETTINGS_LAST_PROJECT_PATH      "Last project path"
#define SETTINGS_LAST_RESULT_PATH       "Last result path"
#define SETTINGS_LAST_SOURCE_PATH       "Last source path"
#define SETTINGS_LAST_INCLUDE_PATH      "Last include path"
#define SETTINGS_LAST_APP_PATH          "Last application path"


/**
 * @brief Obtains the path of specified type
 * Returns the path of specified type if not empty. Otherwise returns last check
 * path if valid or user's home directory.
 * @param type Type of path to obtain
 * @return Best path fo provided type
 */
QString GetPath(const QString &type);

/**
 * @brief Stores last used path of specified type
 * Stores provided path as last used path for specified type.
 * @param type Type of the path to store
 * @param value Path to store
 */
void SetPath(const QString &type, const QString &value);

/// @}
#endif
