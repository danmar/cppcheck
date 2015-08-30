#include "libraryeditargdialog.h"
#include "ui_libraryeditargdialog.h"

LibraryEditArgDialog::LibraryEditArgDialog(QWidget *parent, const CppcheckLibraryData::Function::Arg &a) :
    QDialog(parent),
    ui(new Ui::LibraryEditArgDialog),
    arg(a)
{
    ui->setupUi(this);
    ui->notbool->setChecked(arg.notbool);
    ui->notnull->setChecked(arg.notnull);
    ui->notuninit->setChecked(arg.notuninit);
    ui->strz->setChecked(arg.strz);
    ui->formatStr->setChecked(arg.formatstr);
    ui->formatStrSafe->setVisible(false);
    ui->formatStrType->setVisible(false);
    ui->valid->setText(arg.valid);
}

LibraryEditArgDialog::~LibraryEditArgDialog()
{
    delete ui;
}

CppcheckLibraryData::Function::Arg LibraryEditArgDialog::getArg() const
{
    CppcheckLibraryData::Function::Arg ret;
    ret.notbool   = ui->notbool->isChecked();
    ret.notnull   = ui->notnull->isChecked();
    ret.notuninit = ui->notuninit->isChecked();
    ret.strz      = ui->strz->isChecked();
    ret.formatstr = ui->formatStr->isChecked();
    ret.minsizes  = arg.minsizes;  // TODO : read from GUI
    ret.valid     = ui->valid->text();
    return ret;
}
