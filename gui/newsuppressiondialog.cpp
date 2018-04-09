#include "newsuppressiondialog.h"
#include "ui_newsuppressiondialog.h"

NewSuppressionDialog::NewSuppressionDialog(QWidget *parent) :
    QDialog(parent),
    mUI(new Ui::NewSuppressionDialog)
{
    mUI->setupUi(this);
}

NewSuppressionDialog::~NewSuppressionDialog()
{
    delete mUI;
}

void NewSuppressionDialog::setErrorIds(const QStringList &errorIds)
{
    mUI->mComboErrorId->addItems(errorIds);
    mUI->mComboErrorId->setCurrentIndex(-1);
    mUI->mComboErrorId->setCurrentText("");
}

Suppressions::Suppression NewSuppressionDialog::getSuppression() const
{
    Suppressions::Suppression ret;
    ret.errorId = mUI->mComboErrorId->currentText().toStdString();
    ret.fileName = mUI->mTextFileName->text().toStdString();
    ret.lineNumber = mUI->mTextLineNumber->text().toInt();
    ret.symbolName = mUI->mTextSymbolName->text().toStdString();
    return ret;
}
