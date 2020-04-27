#ifndef FUNCTIONCONTRACTDIALOG_H
#define FUNCTIONCONTRACTDIALOG_H

#include <QDialog>

namespace Ui {
    class FunctionContractDialog;
}

class FunctionContractDialog : public QDialog {
    Q_OBJECT

public:
    explicit FunctionContractDialog(QWidget *parent, const QString &name, const QString &expects);
    ~FunctionContractDialog();
    QString getExpects() const;
private:
    Ui::FunctionContractDialog *mUi;
};

#endif // FUNCTIONCONTRACTDIALOG_H
