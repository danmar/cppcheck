#ifndef FUNCTIONCONTRACTDIALOG_H
#define FUNCTIONCONTRACTDIALOG_H

#include <QDialog>

namespace Ui {
    class FunctionContractDialog;
}

class FunctionContractDialog : public QDialog {
    Q_OBJECT

public:
    explicit FunctionContractDialog(QWidget *parent, QString name, QString expects);
    ~FunctionContractDialog();
    QString getExpects() const;
private:
    Ui::FunctionContractDialog *ui;
};

#endif // FUNCTIONCONTRACTDIALOG_H
