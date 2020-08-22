#include "variablecontractsdialog.h"
#include "ui_variablecontractsdialog.h"

#include <QRegExpValidator>

VariableContractsDialog::VariableContractsDialog(QWidget *parent, QString var) :
    QDialog(parent),
    ui(new Ui::VariableContractsDialog)
{
    ui->setupUi(this);

    mVarName = var.indexOf(" ") < 0 ? var : var.mid(0, var.indexOf(" "));

    this->setWindowTitle(mVarName);

    auto getMinMax = [](QString var, QString minmax) {
        if (var.indexOf(" " + minmax + ":") < 0)
            return QString();
        int pos1 = var.indexOf(" " + minmax + ":") + 2 + minmax.length();
        int pos2 = var.indexOf(" ", pos1);
        if (pos2 < 0)
            return var.mid(pos1);
        return var.mid(pos1, pos2-pos1);
    };

    ui->mMinValue->setText(getMinMax(var, "min"));
    ui->mMaxValue->setText(getMinMax(var, "max"));

    ui->mMinValue->setValidator(new QRegExpValidator(QRegExp("-?[0-9]*")));
    ui->mMaxValue->setValidator(new QRegExpValidator(QRegExp("-?[0-9]*")));
}

VariableContractsDialog::~VariableContractsDialog()
{
    delete ui;
}

QString VariableContractsDialog::getVarname() const
{
    return mVarName;
}
QString VariableContractsDialog::getMin() const
{
    return ui->mMinValue->text();
}
QString VariableContractsDialog::getMax() const
{
    return ui->mMaxValue->text();
}

