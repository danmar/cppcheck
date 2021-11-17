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

FunctionContractDialog::FunctionContractDialog(QWidget *parent, const QString &name, const QString &expects) :
    QDialog(parent),
    mUi(new Ui::FunctionContractDialog)
{
    mUi->setupUi(this);
    mUi->functionName->setText(formatFunctionName(name));
    mUi->expects->setPlainText(expects);
}

FunctionContractDialog::~FunctionContractDialog()
{
    delete mUi;
}

QString FunctionContractDialog::getExpects() const
{
    return mUi->expects->toPlainText();
}

