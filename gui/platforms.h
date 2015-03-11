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

#ifndef PLATFORMS_H
#define PLATFORMS_H

#include <QObject>
#include <QString>
#include <QList>
#include <QAction>
#include "settings.h"

/// @addtogroup GUI
/// @{

/**
 * @brief Checked platform GUI-data.
 */
struct Platform {
    QString mTitle;  /**< Text visible in the GUI. */
    Settings::PlatformType mType; /**< Type in the core. */
    QAction *mActMainWindow; /**< Pointer to main window action item. */
};

/**
  * @brief List of checked platforms.
  */
class Platforms : public QObject {
    Q_OBJECT

public:
    explicit Platforms(QObject *parent = NULL);
    void add(const QString &title, Settings::PlatformType platform);
    int getCount() const;
    void init();
    Platform& get(Settings::PlatformType platform);

    QList<Platform> mPlatforms;
};

/// @}
#endif // PLATFORMS_H
