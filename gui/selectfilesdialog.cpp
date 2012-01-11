
#include "selectfilesdialog.h"
#include "ui_selectfilesdialog.h"

#include <QTreeView>
#include <QVBoxLayout>
#include <QFileSystemModel>
#include <QStringList>
#include <QPushButton>

class SelectFilesModel : public QFileSystemModel {
private:
    QStringList selected;
    QStringList unselected;

    /**
     * Get index in stringlist where start of string matches. If
     * many strings in the stringlist match then return the index
     * for the longest string.
     * \param s stringlist with filepaths
     * \param filepath the filepath that is matched against the stringlist
     */
    int getindex(const QStringList &s, const QString &filepath) const {
        int matchlen = 0;
        int matchindex = -1;
        for (int i = 0; i < s.size(); ++i) {
            if (filepath.startsWith(s[i])) {
                if (s[i].size() > matchlen)
                    matchindex = i;
            }
        }
        return matchindex;
    }

public:
    SelectFilesModel() : QFileSystemModel() {
        QStringList f;
        f << "*.cpp";
        setNameFilters(f);
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
            int selindex = getindex(selected, filepath);
            int unselindex = getindex(unselected, filepath);
            if (selindex >= 0 && unselindex == -1)
                return Qt::Checked;
            if (selindex >= 0 && unselindex >= 0 &&
                    selected[selindex].size() > unselected[unselindex].size())
                return Qt::Checked;

            return Qt::Unchecked;
        }
        return QFileSystemModel::data(index, role);
    }

    bool setData(const QModelIndex& index, const QVariant& value, int role) {
        if (role == Qt::CheckStateRole) {
            const QString filepath = filePath(index);
            if (unselected.indexOf(filepath) != -1) {
                // remove unchecked path
                unselected.removeAll(filepath);
            } else if (selected.indexOf(filepath) != -1) {
                // remove child selected paths
                for (int i = selected.size() - 1; i >= 0; --i) {
                    if (selected[i].startsWith(filepath))
                        selected.removeAt(i);
                }

                // remove child unselected paths
                for (int i = unselected.size() - 1; i >= 0; --i) {
                    if (unselected[i].startsWith(filepath))
                        unselected.removeAt(i);
                }
            } else {
                int selindex = getindex(selected, filepath);
                int unselindex = getindex(unselected, filepath);
                if (selindex == -1)
                    selected.append(filepath);
                else if (unselindex >= 0 && selected[selindex].size() < unselected[unselindex].size())
                    selected.append(filepath);
                else
                    unselected.append(filepath);
            }

            if (rowCount(index) > 0)
                emit(dataChanged(index, index.child(rowCount(index)-1,0)));

            return true;
        }
        return QFileSystemModel::setData(index, value, role);
    }
};

SelectFilesDialog::~SelectFilesDialog() {
    delete ui;
}


SelectFilesDialog::SelectFilesDialog(QWidget *w) :
    QDialog(w),
    ui(new Ui::SelectFilesDialog)
{
    ui->setupUi(this);

    ui->treeView->setModel(new SelectFilesModel);
    for (int i = 1; i < 4; ++i)
        ui->treeView->setColumnHidden(i, true);
}


