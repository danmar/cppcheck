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

    ui->minsize1type->setEnabled(true);
    ui->minsize1arg->setEnabled(arg.minsizes.count() >= 1);
    ui->minsize1arg2->setEnabled(arg.minsizes.count() >= 1 && arg.minsizes[0].type == "mul");
    ui->minsize2type->setEnabled(arg.minsizes.count() >= 1);
    ui->minsize2arg->setEnabled(arg.minsizes.count() >= 2);
    ui->minsize2arg2->setEnabled(arg.minsizes.count() >= 2 && arg.minsizes[1].type == "mul");

    QStringList items;
    items << "None" << "argvalue" << "constant" << "mul" << "strlen";

    ui->minsize1type->clear();
    ui->minsize1type->addItems(items);
    if (arg.minsizes.count() >= 1) {
        ui->minsize1type->setCurrentIndex(items.indexOf(minsizes[0].type));
        ui->minsize1arg->setValue(minsizes[0].arg.toInt());
        if (arg.minsizes[0].type == "mul")
            ui->minsize1arg2->setValue(minsizes[0].arg2.toInt());
        else
            ui->minsize1arg2->setValue(0);
    } else {
        ui->minsize1type->setCurrentIndex(0);
        ui->minsize1arg->setValue(0);
        ui->minsize1arg2->setValue(0);
    }

    ui->minsize2type->clear();
    ui->minsize2type->addItems(items);
    if (arg.minsizes.count() >= 2) {
        ui->minsize2type->setCurrentIndex(items.indexOf(minsizes[1].type));
        ui->minsize2arg->setValue(minsizes[1].arg.toInt());
        if (arg.minsizes[1].type == "mul")
            ui->minsize2arg2->setValue(minsizes[1].arg2.toInt());
        else
            ui->minsize2arg2->setValue(0);
    } else {
        ui->minsize2type->setCurrentIndex(0);
        ui->minsize2arg->setValue(0);
        ui->minsize2arg2->setValue(0);
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
    if (ui->minsize1type->currentIndex() != 0) {
        CppcheckLibraryData::Function::Arg::MinSize minsize1;
        minsize1.type = ui->minsize1type->currentText();
        minsize1.arg  = QString::number(ui->minsize1arg->value());
        if (minsize1.type == "mul")
            minsize1.arg2 = QString::number(ui->minsize1arg2->value());
        ret.minsizes.append(minsize1);

        if (ui->minsize2type->currentIndex() != 0) {
            CppcheckLibraryData::Function::Arg::MinSize minsize2;
            minsize2.type = ui->minsize2type->currentText();
            minsize2.arg  = QString::number(ui->minsize2arg->value());
            if (minsize2.type == "mul")
                minsize2.arg2 = QString::number(ui->minsize2arg2->value());
            ret.minsizes.append(minsize2);
        }
    }
    ret.valid     = ui->valid->text();
    return ret;
}

void LibraryEditArgDialog::minsizeChanged(int)
{
    ui->minsize1arg->setEnabled(ui->minsize1type->currentIndex() != 0);
    ui->minsize1arg2->setEnabled(ui->minsize1type->currentText() == "mul");
    ui->minsize2type->setEnabled(ui->minsize1type->currentIndex() != 0);
    ui->minsize2arg->setEnabled(ui->minsize2type->currentIndex() != 0);
    ui->minsize2arg2->setEnabled(ui->minsize2type->currentText() == "mul");
}
