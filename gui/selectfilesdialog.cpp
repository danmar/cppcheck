/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2012 Daniel Marjamäki and Cppcheck team.
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

#include "selectfilesdialog.h"
#include "ui_selectfilesdialog.h"
#include "filelist.h"

#include <QTreeView>
#include <QVBoxLayout>
#include <QFileSystemModel>
#include <QStringList>
#include <QPushButton>

class SelectFilesModel : public QFileSystemModel {
private:
    /**
     * paths that are user-checked. on the screen all children
     * for these paths will appear to be checked too unless
     * they are "unchecked".
     */
    QStringList checked;

    /**
     * paths that are user-unchecked.
     */
    QStringList unchecked;

    /**
     * Get index in stringlist where start of string matches. If
     * many strings in the stringlist match then return the index
     * for the longest string.
     * \param paths stringlist with filepaths
     * \param filepath the filepath that is matched against the stringlist
     */
    int getindex(const QStringList &paths, const QString &filepath) const {
        int matchlen = 0;
        int matchindex = -1;
        for (int i = 0; i < paths.size(); ++i) {
            if (filepath.startsWith(paths[i])) {
                // not a real match of paths..
                if (paths[i].size() < filepath.size() && filepath[paths[i].size()] != '/')
                    continue;

                // paths match. the return value is the index for the
                // longest match
                if (paths[i].size() > matchlen)
                    matchindex = i;
            }
        }
        return matchindex;
    }

    /**
     * Is filepath partially checked?
     * \param filepath the filepath to investigate
     * \param checkindex result from getindex(checked,filepath). If not given the getindex will be called.
     * \return true if filepath is partially checked
     */
    bool partiallyChecked(const QString &filepath, int checkindex = -2) const {
        const QString filepath2 = filepath.endsWith("/") ? filepath : (filepath + "/");

        for (int i = 0; i < unchecked.size(); ++i) {
            if (unchecked[i].startsWith(filepath2)) {
                return true;
            }
        }

        if (checkindex == -2)
            checkindex = getindex(checked, filepath);


        if (checkindex == -1) {
            for (int i = 0; i < checked.size(); ++i) {
                if (checked[i].startsWith(filepath2)) {
                    return true;
                }
            }
        }

        return false;
    }

public:
    SelectFilesModel() : QFileSystemModel() {
        class FileLister : private FileList {
        public:
            static QStringList filters() {
                return GetDefaultFilters();
            }
        };
        setNameFilters(FileLister::filters());
        setNameFilterDisables(false);
        setRootPath("/");
    }

    Qt::ItemFlags flags(const QModelIndex& index) const {
        if (index.column() == 0)
            return QFileSystemModel::flags(index) | Qt::ItemIsUserCheckable;
        return QFileSystemModel::flags(index);
    }

    QVariant data(const QModelIndex& index, int role=Qt::DisplayRole) const {
        if (role == Qt::CheckStateRole) {
            const QString filepath = filePath(index);
            const int checkindex = getindex(checked, filepath);
            const int uncheckindex = getindex(unchecked, filepath);

            // If some children are not checked then this item should be partially checked..
            if (partiallyChecked(filepath, checkindex))
                return Qt::PartiallyChecked;

            // Is item selected but not unselected?
            if (checkindex >= 0 && uncheckindex == -1)
                return Qt::Checked;
            if (checkindex >= 0 && uncheckindex >= 0 &&
                checked[checkindex].size() > unchecked[uncheckindex].size())
                return Qt::Checked;

            // Item is either not selected at all or else it is unselected
            return Qt::Unchecked;
        }
        return QFileSystemModel::data(index, role);
    }

    bool setData(const QModelIndex& index, const QVariant& value, int role) {
        if (role == Qt::CheckStateRole) {
            const QString filepath = filePath(index);

            bool partiallychecked = partiallyChecked(filepath);

            if (unchecked.indexOf(filepath) != -1) {
                // remove unchecked path
                unchecked.removeAll(filepath);
            } else if (partiallychecked || checked.indexOf(filepath) != -1) {
                // remove child selected paths
                for (int i = checked.size() - 1; i >= 0; --i) {
                    if (checked[i].startsWith(filepath))
                        checked.removeAt(i);
                }

                // remove child unselected paths
                for (int i = unchecked.size() - 1; i >= 0; --i) {
                    if (unchecked[i].startsWith(filepath))
                        unchecked.removeAt(i);
                }

                // If partialChecked then select this item
                if (partiallychecked)
                    checked.append(filepath);
            } else {
                const int checkindex = getindex(checked, filepath);
                const int uncheckindex = getindex(unchecked, filepath);
                if (checkindex == -1)
                    checked.append(filepath);
                else if (uncheckindex >= 0 && checked[checkindex].size() < unchecked[uncheckindex].size())
                    checked.append(filepath);
                else
                    unchecked.append(filepath);
            }

            if (rowCount(index) > 0)
                emit(dataChanged(index, index.child(rowCount(index)-1,0)));

            // update parents
            QModelIndex parent = index.parent();
            while (parent != QModelIndex()) {
                emit(dataChanged(parent,parent));
                parent = parent.parent();
            }

            return true;
        }
        return QFileSystemModel::setData(index, value, role);
    }

    QStringList getFiles() const {
        QStringList ret;

        // List all files in "checked" folders..
        FileList fileLister;
        fileLister.AddPathList(checked);
        ret = fileLister.GetFileList();

        // Remove all items from ret that are unchecked but not checked..
        for (int i = ret.size() - 1; i >= 0; i--) {
            int uncheckindex = getindex(unchecked, ret[i]);
            if (uncheckindex == -1)
                continue;

            // both checked and unchecked, check which to rely on
            int checkindex = getindex(checked, ret[i]);
            if (checked[checkindex].size() < unchecked[uncheckindex].size())
                ret.removeAt(i);
        }

        return ret;
    }
};



SelectFilesDialog::SelectFilesDialog(QWidget *w) :
    QDialog(w),
    ui(new Ui::SelectFilesDialog)
{
    ui->setupUi(this);

    selectfilesmodel = new SelectFilesModel;

    ui->treeView->setModel(selectfilesmodel);
    for (int i = 1; i < 4; ++i)
        ui->treeView->setColumnHidden(i, true);

    // Change text of "OK" button to "Check"
    QPushButton *okbutton = ui->buttonBox->button(QDialogButtonBox::Ok);
    if (okbutton)
        okbutton->setText(tr("Check"));
}

SelectFilesDialog::~SelectFilesDialog()
{
    delete ui;
}

QStringList SelectFilesDialog::getFiles() const
{
    return selectfilesmodel->getFiles();
}
