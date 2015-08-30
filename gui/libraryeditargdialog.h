#ifndef LIBRARYEDITARGDIALOG_H
#define LIBRARYEDITARGDIALOG_H

#include <QDialog>
#include "librarydata.h"

namespace Ui {
    class LibraryEditArgDialog;
}

class LibraryEditArgDialog : public QDialog {
    Q_OBJECT

public:
    LibraryEditArgDialog(QWidget *parent, const LibraryData::Function::Arg &a);
    ~LibraryEditArgDialog();

    LibraryData::Function::Arg getArg() const;

private:
    Ui::LibraryEditArgDialog *ui;

    LibraryData::Function::Arg arg;
};

#endif // LIBRARYEDITARGDIALOG_H
