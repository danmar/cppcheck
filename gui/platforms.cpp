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

#include "platforms.h"

Platforms::Platforms(QObject *parent)
    : QObject(parent)
{
    init();
}

void Platforms::add(const QString &title, cppcheck::Platform::Type platform)
{
    Platform plat;
    plat.mTitle = title;
    plat.mType = platform;
    plat.mActMainWindow = nullptr;
    mPlatforms << plat;
}

void Platforms::init()
{
    add(tr("Native"), cppcheck::Platform::Type::Native);
    add(tr("Unix 32-bit"), cppcheck::Platform::Type::Unix32);
    add(tr("Unix 64-bit"), cppcheck::Platform::Type::Unix64);
    add(tr("Windows 32-bit ANSI"), cppcheck::Platform::Type::Win32A);
    add(tr("Windows 32-bit Unicode"), cppcheck::Platform::Type::Win32W);
    add(tr("Windows 64-bit"), cppcheck::Platform::Type::Win64);
}

int Platforms::getCount() const
{
    return mPlatforms.count();
}

Platform& Platforms::get(cppcheck::Platform::Type platform)
{
    QList<Platform>::iterator iter = mPlatforms.begin();
    while (iter != mPlatforms.end()) {
        if ((*iter).mType == platform) {
            return *iter;
        }
        ++iter;
    }
    return mPlatforms.first();
}
