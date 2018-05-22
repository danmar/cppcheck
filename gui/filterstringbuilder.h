/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2018 Cppcheck team.
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

#ifndef FILTERSTRINGBUILDER_H
#define FILTERSTRINGBUILDER_H

#include <QMap>
#include <QString>

/// @addtogroup GUI
/// @{

/**
 * @brief A helper class to create filter strings for QFileDialog operations.
 *
 * Intended usage is as follows:
 *
 * @code
 * const QString filter = FilterStringBuilder()
 *     .add(tr("Supported images"), "*.bmp *.jpg *.png")
 *     .add(tr("Plain text"), "*.txt")
 *     .addAllSupported()
 *     .toFilterString();
 *
 * // filter will be "All supported files (*.txt *.bmp *.jpg *.png);;Plain text (*.txt);; Supported images (*.bmp *.jpg *.png)"
 * @endcode
 */
class FilterStringBuilder
{
   Q_DECLARE_TR_FUNCTIONS(FilterStringBuilder)
   
   QMap<QString,QString> mFilters;
   bool mDisplayAll = false;
   bool mDisplayAllSupported = false;
      
public:
   /**
    * @brief Adds a new entry to the filter string list.
    *
    * When generating the filter string, the entries will be sorted by
    * description, so it doesn't matter in which order you add them.
    *
    * Provide a short description (which might be localized) and one or
    * multiple file name patterns like this:
    *
    * @code
    * FilterStringBuilder()
    *     .add(tr("Supported images"), "*.bmp *.jpg *.png")
    *     .add(tr("Plain text"), "*.txt")
    *
    * // results in two filter strings like this:
    * //
    * // Plain text (*.txt)
    * // Supported images (*.bmp *.jpg *.png)
    * @endcode
    */
   FilterStringBuilder& add(const QString& desc, const QString& patterns);

   /**
    * @brief Enables automatic generation of a filter entry which matches all
    * file types: All files (*.*)
    *
    * This entry will be placed before entries added via the add() method,
    * ignoring the sort order.
    */
   FilterStringBuilder& addAll();

   /**
    * @brief Enables automatic generation of a filter entry which matches all
    * supported file types: All supported files (*.bmp *.jpg *.png *.txt ...)
    *
    * Supported file types are those you're adding via the add() method. It
    * doesn't matter whether you call this method before adding supported
    * file types or after adding them.
    *
    * If enabled, this entry will be placed before all other filter string
    * entries, making it the default filter.
    */
   FilterStringBuilder& addAllSupported();

   /**
    * @brief Creates a string suitable for passing as the filter argument to
    * methods like QFileDialog::getOpenFileName.
    *
    * If the instance is configured to generate a filter for all supported
    * file types, that one will be the first in the filter list. It will be
    * used as the default filter, unless a matching "selectedFilter"
    * parameter is passed as well.
    *
    * If the instance is configured to generate a filter for all file types,
    * it will be generated as the second entry, making it the default if all
    * supported file types are not being generated.
    *
    * The remaining filter entries will be added in alphabetic order.
    */
   QString toFilterString() const;
};


/// @}
#endif
