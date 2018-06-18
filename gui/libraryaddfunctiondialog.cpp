#include "libraryaddfunctiondialog.h"
#include "ui_libraryaddfunctiondialog.h"

#include <QRegExp>
#include <QValidator>

LibraryAddFunctionDialog::LibraryAddFunctionDialog(QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::LibraryAddFunctionDialog)
{
    mUi->setupUi(this);
    QRegExp rx(NAMES);
    QValidator *validator = new QRegExpValidator(rx, this);
    mUi->functionName->setValidator(validator);
}

LibraryAddFunctionDialog::~LibraryAddFunctionDialog()
{
    delete mUi;
}

QString LibraryAddFunctionDialog::functionName() const
{
    return mUi->functionName->text();
}

int LibraryAddFunctionDialog::numberOfArguments() const
{
    return mUi->numberOfArguments->value();
}

