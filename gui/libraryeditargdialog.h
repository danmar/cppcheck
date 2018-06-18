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
    LibraryEditArgDialog(const LibraryEditArgDialog &) = delete;
    ~LibraryEditArgDialog();
    LibraryEditArgDialog &operator=(const LibraryEditArgDialog &) = delete;

    CppcheckLibraryData::Function::Arg getArg() const;

private slots:
    void minsizeChanged(int);

private:
    Ui::LibraryEditArgDialog *mUi;

    QList<CppcheckLibraryData::Function::Arg::MinSize> mMinSizes;
};

#endif // LIBRARYEDITARGDIALOG_H
