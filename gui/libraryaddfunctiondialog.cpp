#include "libraryaddfunctiondialog.h"
#include "ui_libraryaddfunctiondialog.h"

#include <QRegExp>
#include <QValidator>

LibraryAddFunctionDialog::LibraryAddFunctionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LibraryAddFunctionDialog)
{
    ui->setupUi(this);
    QRegExp rx(NAMES);
    QValidator *validator = new QRegExpValidator(rx, this);
    ui->functionName->setValidator(validator);
}

LibraryAddFunctionDialog::~LibraryAddFunctionDialog()
{
    delete ui;
}

QString LibraryAddFunctionDialog::functionName() const
{
    return ui->functionName->text();
}

int LibraryAddFunctionDialog::numberOfArguments() const
{
    return ui->numberOfArguments->value();
}

