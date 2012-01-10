
#include "selectfilesdialog.h"

#include <QTreeView>
#include <QVBoxLayout>
#include <QFileSystemModel>
#include <QList>
#include <QString>
#include <QPushButton>

class SelectFilesModel : public QFileSystemModel {
private:
    QList<QString> selected;
    QList<QString> unselected;

    int getindex(const QList<QString> &s, const QString &filepath) const {
        for (int i = 0; i < s.size(); ++i) {
            if (filepath.startsWith(s[i]))
                return i;
        }
        return -1;
    }

public:
    SelectFilesModel() : QFileSystemModel() {
        QStringList f;
        f << "*.cpp";
        setNameFilters(f);
        setNameFilterDisables(false);
    }

    Qt::ItemFlags flags(const QModelIndex& index) const {
        if (index.column() == 0)
            return QFileSystemModel::flags(index) | Qt::ItemIsUserCheckable;
        return QFileSystemModel::flags(index);
    }

    QVariant data(const QModelIndex& index, int role=Qt::DisplayRole) const {
        if (role == Qt::CheckStateRole) {
            const QString filepath = filePath(index);
            if (getindex(selected,filepath) >= 0 &&
                getindex(unselected,filepath) == -1)
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
                // remove checked path
                selected.removeAll(filepath);

                // remove child unchecked paths
                int i;
                while ((i = getindex(unselected,filepath)) != -1) {
                    unselected.removeAt(i);
                }
            } else {
                if (getindex(selected,filepath) == -1)
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




SelectFilesDialog::SelectFilesDialog() : QDialog()
{
    setModal(true);

    resize(300,400);

    QTreeView *treeView = new QTreeView(this);
    treeView->setModel(new SelectFilesModel);
    for (int i = 1; i < 4; ++i)
        treeView->setColumnHidden(i, true);

    QPushButton *cancel = new QPushButton("Cancel", this);
    connect(cancel,SIGNAL(clicked()),this,SLOT(accept()));

    setLayout(new QVBoxLayout(this));
    layout()->addWidget(treeView);
    layout()->addWidget(cancel);
}


