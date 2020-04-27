#include "functioncontractdialog.h"
#include "ui_functioncontractdialog.h"

static QString formatFunctionName(QString f)
{
    if (f.endsWith("()"))
        return f;
    f.replace("(", "(\n    ");
    f.replace(",", ",\n    ");
    return f;
}

FunctionContractDialog::FunctionContractDialog(QWidget *parent, QString name, QString expects) :
    QDialog(parent),
    ui(new Ui::FunctionContractDialog)
{
    ui->setupUi(this);
    ui->functionName->setText(formatFunctionName(name));
    ui->expects->setPlainText(expects);
}

FunctionContractDialog::~FunctionContractDialog()
{
    delete ui;
}

QString FunctionContractDialog::getExpects() const
{
    return ui->expects->toPlainText();
}

