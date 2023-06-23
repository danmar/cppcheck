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

#ifndef PLATFORMS_H
#define PLATFORMS_H

#include "platform.h"

#include <QList>
#include <QObject>
#include <QString>

class QAction;

/// @addtogroup GUI
/// @{

/**
 * @brief Checked platform GUI-data.
 */
struct Platform {
    QString mTitle;  /**< Text visible in the GUI. */
    cppcheck::Platform::Type mType; /**< Type in the core. */
    QAction *mActMainWindow; /**< Pointer to main window action item. */
};

/**
 * @brief List of checked platforms.
 */
class Platforms : public QObject {
    Q_OBJECT

public:
    explicit Platforms(QObject *parent = nullptr);
    void add(const QString &title, cppcheck::Platform::Type platform);
    int getCount() const;
    void init();
    Platform& get(cppcheck::Platform::Type platform);

    QList<Platform> mPlatforms;
};

/// @}
#endif // PLATFORMS_H
