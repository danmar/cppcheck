#include "libraryeditargdialog.h"
#include "ui_libraryeditargdialog.h"

LibraryEditArgDialog::LibraryEditArgDialog(QWidget *parent, const CppcheckLibraryData::Function::Arg &arg) :
    QDialog(parent),
    ui(new Ui::LibraryEditArgDialog),
    minsizes(arg.minsizes)
{
    ui->setupUi(this);
    ui->notbool->setChecked(arg.notbool);
    ui->notnull->setChecked(arg.notnull);
    ui->notuninit->setChecked(arg.notuninit);
    ui->strz->setChecked(arg.strz);
    ui->formatstr->setChecked(arg.formatstr);
    ui->valid->setText(arg.valid);
    foreach(const CppcheckLibraryData::Function::Arg::MinSize &minsize, arg.minsizes) {
        if (ui->minsizes->count() > 0)
            ui->minsizes->addItem("and");
        if (minsize.type == "argvalue")
            ui->minsizes->addItem("Buffer size must be at least as many bytes as given by argument " + minsize.arg);
        else if (minsize.type == "constant")
            ui->minsizes->addItem("Buffer size must be at least " + minsize.arg + " bytes");
        else if (minsize.type == "mul")
            ui->minsizes->addItem("Buffer size must be at least as many bytes as multiplication result of argument " + minsize.arg + " and " + minsize.arg2);
        else if (minsize.type == "strlen")
            ui->minsizes->addItem("Buffer size must be at least as big as the string in argument " + minsize.arg + "");
        else
            ui->minsizes->addItem("unhandled type: " + minsize.type);
    }
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
    ret.formatstr = ui->formatstr->isChecked();
    ret.minsizes  = minsizes;
    ret.valid     = ui->valid->text();
    return ret;
}
