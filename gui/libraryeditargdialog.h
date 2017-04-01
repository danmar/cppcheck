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
    LibraryEditArgDialog(QWidget *parent, const CppcheckLibraryData::Function::Arg &arg);
    ~LibraryEditArgDialog();

    CppcheckLibraryData::Function::Arg getArg() const;

private slots:
    void minsizeChanged(int);

private:
    Ui::LibraryEditArgDialog *ui;

    QList<CppcheckLibraryData::Function::Arg::MinSize> minsizes;
};

#endif // LIBRARYEDITARGDIALOG_H
