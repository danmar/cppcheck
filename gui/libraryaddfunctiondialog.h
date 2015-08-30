#ifndef LIBRARYADDFUNCTIONDIALOG_H
#define LIBRARYADDFUNCTIONDIALOG_H

#include <QDialog>

namespace Ui {
    class LibraryAddFunctionDialog;
}

class LibraryAddFunctionDialog : public QDialog {
    Q_OBJECT

public:
    explicit LibraryAddFunctionDialog(QWidget *parent = 0);
    ~LibraryAddFunctionDialog();

    QString functionName() const;
    int     numberOfArguments() const;

private:
    Ui::LibraryAddFunctionDialog *ui;
};

#endif // LIBRARYADDFUNCTIONDIALOG_H
