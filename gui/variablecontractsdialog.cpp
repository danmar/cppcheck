#include "variablecontractsdialog.h"
#include "ui_variablecontractsdialog.h"

#include <QRegExpValidator>

VariableContractsDialog::VariableContractsDialog(QWidget *parent, QString var) :
    QDialog(parent),
    mUI(new Ui::VariableContractsDialog)
{
    mUI->setupUi(this);

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

    mUI->mMinValue->setText(getMinMax(var, "min"));
    mUI->mMaxValue->setText(getMinMax(var, "max"));

    mUI->mMinValue->setValidator(new QRegExpValidator(QRegExp("-?[0-9]*")));
    mUI->mMaxValue->setValidator(new QRegExpValidator(QRegExp("-?[0-9]*")));
}

VariableContractsDialog::~VariableContractsDialog()
{
    delete mUI;
}

QString VariableContractsDialog::getVarname() const
{
    return mVarName;
}
QString VariableContractsDialog::getMin() const
{
    return mUI->mMinValue->text();
}
QString VariableContractsDialog::getMax() const
{
    return mUI->mMaxValue->text();
}
