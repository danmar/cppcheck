#ifndef VARIABLECONSTRAINTSDIALOG_H
#define VARIABLECONSTRAINTSDIALOG_H

#include <QDialog>

namespace Ui {
    class VariableContractsDialog;
}

class VariableContractsDialog : public QDialog {
    Q_OBJECT

public:
    explicit VariableContractsDialog(QWidget *parent, QString var);
    ~VariableContractsDialog();

    QString getVarname() const;
    QString getMin() const;
    QString getMax() const;

private:
    Ui::VariableContractsDialog *mUI;
    QString mVarName;
};

#endif // VARIABLECONSTRAINTSDIALOG_H
