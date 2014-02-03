#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "applicationsettings.h"
#include <QDir>
#include <QFileDialog>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    connect(this, SIGNAL(accepted()), this, SLOT(saveSettings()));

    ApplicationSettings settings;
    ui->resultsFolder->setText(settings.resultsFolder);
    ui->clang->setText(settings.clang);
    ui->cppcheck->setText(settings.cppcheck);
    ui->gcc->setText(settings.gcc);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::saveSettings()
{
    ApplicationSettings settings;
    settings.resultsFolder = ui->resultsFolder->text();
    settings.clang         = ui->clang->text();
    settings.cppcheck      = ui->cppcheck->text();
    settings.gcc           = ui->gcc->text();
    settings.save();
}

void SettingsDialog::resultsFolderBrowse()
{
    const QString dir = QFileDialog::getExistingDirectory(this, tr("Select folder where results will be saved"), QDir::homePath());
    if (!dir.isEmpty())
        ui->resultsFolder->setText(dir);
}

void SettingsDialog::clangBrowse()
{
    const QString clang = QFileDialog::getOpenFileName(this, tr("Select clang binary"));
    if (!clang.isEmpty())
        ui->clang->setText(clang);
}

void SettingsDialog::cppcheckBrowse()
{
    const QString cppcheck = QFileDialog::getOpenFileName(this, tr("Select cppcheck binary"));
    if (!cppcheck.isEmpty())
        ui->cppcheck->setText(cppcheck);
}

void SettingsDialog::gccBrowse()
{
    const QString gcc = QFileDialog::getOpenFileName(this, tr("Select gcc binary"));
    if (!gcc.isEmpty())
        ui->clang->setText(gcc);
}
