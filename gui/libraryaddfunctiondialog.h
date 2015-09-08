#ifndef LIBRARYADDFUNCTIONDIALOG_H
#define LIBRARYADDFUNCTIONDIALOG_H

#include <QDialog>

#define SIMPLENAME     "[_a-zA-Z][_a-zA-Z0-9]*"            // just a name
#define SCOPENAME      SIMPLENAME "(::" SIMPLENAME ")*"    // names with optional scope
#define NAMES          SCOPENAME "(," SCOPENAME ")*"       // names can be separated by comma

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
