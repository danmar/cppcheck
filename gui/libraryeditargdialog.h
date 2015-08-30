#ifndef LIBRARYEDITARGDIALOG_H
#define LIBRARYEDITARGDIALOG_H

#include <QDialog>
#include "cppchecklibrarydata.h"

namespace Ui {
    class LibraryEditArgDialog;
}

class LibraryEditArgDialog : public QDialog {
    Q_OBJECT

public:
    LibraryEditArgDialog(QWidget *parent, const CppcheckLibraryData::Function::Arg &a);
    ~LibraryEditArgDialog();

    CppcheckLibraryData::Function::Arg getArg() const;

private:
    Ui::LibraryEditArgDialog *ui;

    CppcheckLibraryData::Function::Arg arg;
};

#endif // LIBRARYEDITARGDIALOG_H
