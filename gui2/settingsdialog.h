#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

static const char SETTING_CLANG_CMD[]    = "clang";
static const char SETTING_CPPCHECK_CMD[] = "cppcheck";
static const char SETTING_GCC_CMD[]      = "gcc";

namespace Ui {
    class SettingsDialog;
}

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

private slots:
    void saveSettings();

    void resultsFolderBrowse();
    void clangBrowse();
    void cppcheckBrowse();
    void gccBrowse();

private:
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
