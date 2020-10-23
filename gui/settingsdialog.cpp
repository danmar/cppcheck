/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <QDialog>
#include <QWidget>
#include <QList>
#include <QListWidgetItem>
#include <QSettings>
#include <QFileDialog>
#include <QFileInfo>
#include <QThread>
#include "settingsdialog.h"
#include "applicationdialog.h"
#include "applicationlist.h"
#include "translationhandler.h"
#include "codeeditorstyle.h"
#include "codeeditstyledialog.h"
#include "common.h"

SettingsDialog::SettingsDialog(ApplicationList *list,
                               TranslationHandler *translator,
                               QWidget *parent) :
    QDialog(parent),
    mApplications(list),
    mTempApplications(new ApplicationList(this)),
    mTranslator(translator)
{
    mUI.setupUi(this);
    mUI.mPythonPathWarning->setStyleSheet("color: red");
    QSettings settings;
    mTempApplications->copy(list);

    mUI.mJobs->setText(settings.value(SETTINGS_CHECK_THREADS, 1).toString());
    mUI.mForce->setCheckState(boolToCheckState(settings.value(SETTINGS_CHECK_FORCE, false).toBool()));
    mUI.mShowFullPath->setCheckState(boolToCheckState(settings.value(SETTINGS_SHOW_FULL_PATH, false).toBool()));
    mUI.mShowNoErrorsMessage->setCheckState(boolToCheckState(settings.value(SETTINGS_SHOW_NO_ERRORS, false).toBool()));
    mUI.mShowDebugWarnings->setCheckState(boolToCheckState(settings.value(SETTINGS_SHOW_DEBUG_WARNINGS, false).toBool()));
    mUI.mSaveAllErrors->setCheckState(boolToCheckState(settings.value(SETTINGS_SAVE_ALL_ERRORS, false).toBool()));
    mUI.mSaveFullPath->setCheckState(boolToCheckState(settings.value(SETTINGS_SAVE_FULL_PATH, false).toBool()));
    mUI.mInlineSuppressions->setCheckState(boolToCheckState(settings.value(SETTINGS_INLINE_SUPPRESSIONS, false).toBool()));
    mUI.mEnableInconclusive->setCheckState(boolToCheckState(settings.value(SETTINGS_INCONCLUSIVE_ERRORS, false).toBool()));
    mUI.mShowStatistics->setCheckState(boolToCheckState(settings.value(SETTINGS_SHOW_STATISTICS, false).toBool()));
    mUI.mShowErrorId->setCheckState(boolToCheckState(settings.value(SETTINGS_SHOW_ERROR_ID, false).toBool()));
    mUI.mEditPythonPath->setText(settings.value(SETTINGS_PYTHON_PATH, QString()).toString());
    validateEditPythonPath();
    mUI.mEditMisraFile->setText(settings.value(SETTINGS_MISRA_FILE, QString()).toString());

#ifdef Q_OS_WIN
    //mUI.mTabClang->setVisible(true);
    mUI.mEditClangPath->setText(settings.value(SETTINGS_CLANG_PATH, QString()).toString());
    mUI.mEditVsIncludePaths->setText(settings.value(SETTINGS_VS_INCLUDE_PATHS, QString()).toString());
    connect(mUI.mBtnBrowseClangPath, &QPushButton::released, this, &SettingsDialog::browseClangPath);
#else
    mUI.mTabClang->setVisible(false);
#endif
    mCurrentStyle = new CodeEditorStyle(CodeEditorStyle::loadSettings(&settings));
    manageStyleControls();

    connect(mUI.mEditPythonPath, SIGNAL(textEdited(const QString &)),
            this, SLOT(validateEditPythonPath()));

    connect(mUI.mButtons, &QDialogButtonBox::accepted, this, &SettingsDialog::ok);
    connect(mUI.mButtons, &QDialogButtonBox::rejected, this, &SettingsDialog::reject);
    connect(mUI.mBtnAddApplication, SIGNAL(clicked()),
            this, SLOT(addApplication()));
    connect(mUI.mBtnRemoveApplication, SIGNAL(clicked()),
            this, SLOT(removeApplication()));
    connect(mUI.mBtnEditApplication, SIGNAL(clicked()),
            this, SLOT(editApplication()));
    connect(mUI.mBtnDefaultApplication, SIGNAL(clicked()),
            this, SLOT(defaultApplication()));
    connect(mUI.mListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
            this, SLOT(editApplication()));

    connect(mUI.mBtnBrowsePythonPath, &QPushButton::clicked, this, &SettingsDialog::browsePythonPath);
    connect(mUI.mBtnBrowseMisraFile, &QPushButton::clicked, this, &SettingsDialog::browseMisraFile);
    connect(mUI.mBtnEditTheme, SIGNAL(clicked()), this, SLOT(editCodeEditorStyle()));
    connect(mUI.mThemeSystem, SIGNAL(released()), this, SLOT(setCodeEditorStyleDefault()));
    connect(mUI.mThemeDark, SIGNAL(released()), this, SLOT(setCodeEditorStyleDefault()));
    connect(mUI.mThemeLight, SIGNAL(released()), this, SLOT(setCodeEditorStyleDefault()));
    connect(mUI.mThemeCustom, SIGNAL(toggled(bool)), mUI.mBtnEditTheme, SLOT(setEnabled(bool)));

    mUI.mListWidget->setSortingEnabled(false);
    populateApplicationList();

    const int count = QThread::idealThreadCount();
    if (count != -1)
        mUI.mLblIdealThreads->setText(QString::number(count));
    else
        mUI.mLblIdealThreads->setText(tr("N/A"));

    loadSettings();
    initTranslationsList();
}

SettingsDialog::~SettingsDialog()
{
    saveSettings();
}

void SettingsDialog::initTranslationsList()
{
    const QString current = mTranslator->getCurrentLanguage();
    QList<TranslationInfo> translations = mTranslator->getTranslations();
    foreach (TranslationInfo translation, translations) {
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(translation.mName);
        item->setData(mLangCodeRole, QVariant(translation.mCode));
        mUI.mListLanguages->addItem(item);
        if (translation.mCode == current || translation.mCode == current.mid(0, 2))
            mUI.mListLanguages->setCurrentItem(item);
    }
}

Qt::CheckState SettingsDialog::boolToCheckState(bool yes)
{
    if (yes) {
        return Qt::Checked;
    }
    return Qt::Unchecked;
}

bool SettingsDialog::checkStateToBool(Qt::CheckState state)
{
    if (state == Qt::Checked) {
        return true;
    }
    return false;
}


void SettingsDialog::loadSettings()
{
    QSettings settings;
    resize(settings.value(SETTINGS_CHECK_DIALOG_WIDTH, 800).toInt(),
           settings.value(SETTINGS_CHECK_DIALOG_HEIGHT, 600).toInt());
}

void SettingsDialog::saveSettings() const
{
    QSettings settings;
    settings.setValue(SETTINGS_CHECK_DIALOG_WIDTH, size().width());
    settings.setValue(SETTINGS_CHECK_DIALOG_HEIGHT, size().height());
}

void SettingsDialog::saveSettingValues() const
{
    int jobs = mUI.mJobs->text().toInt();
    if (jobs <= 0) {
        jobs = 1;
    }

    QSettings settings;
    settings.setValue(SETTINGS_CHECK_THREADS, jobs);
    saveCheckboxValue(&settings, mUI.mForce, SETTINGS_CHECK_FORCE);
    saveCheckboxValue(&settings, mUI.mSaveAllErrors, SETTINGS_SAVE_ALL_ERRORS);
    saveCheckboxValue(&settings, mUI.mSaveFullPath, SETTINGS_SAVE_FULL_PATH);
    saveCheckboxValue(&settings, mUI.mShowFullPath, SETTINGS_SHOW_FULL_PATH);
    saveCheckboxValue(&settings, mUI.mShowNoErrorsMessage, SETTINGS_SHOW_NO_ERRORS);
    saveCheckboxValue(&settings, mUI.mShowDebugWarnings, SETTINGS_SHOW_DEBUG_WARNINGS);
    saveCheckboxValue(&settings, mUI.mInlineSuppressions, SETTINGS_INLINE_SUPPRESSIONS);
    saveCheckboxValue(&settings, mUI.mEnableInconclusive, SETTINGS_INCONCLUSIVE_ERRORS);
    saveCheckboxValue(&settings, mUI.mShowStatistics, SETTINGS_SHOW_STATISTICS);
    saveCheckboxValue(&settings, mUI.mShowErrorId, SETTINGS_SHOW_ERROR_ID);
    settings.setValue(SETTINGS_PYTHON_PATH, mUI.mEditPythonPath->text());
    settings.setValue(SETTINGS_MISRA_FILE, mUI.mEditMisraFile->text());

#ifdef Q_OS_WIN
    settings.setValue(SETTINGS_CLANG_PATH, mUI.mEditClangPath->text());
    settings.setValue(SETTINGS_VS_INCLUDE_PATHS, mUI.mEditVsIncludePaths->text());
#endif

    const QListWidgetItem *currentLang = mUI.mListLanguages->currentItem();
    if (currentLang) {
        const QString langcode = currentLang->data(mLangCodeRole).toString();
        settings.setValue(SETTINGS_LANGUAGE, langcode);
    }
    CodeEditorStyle::saveSettings(&settings, *mCurrentStyle);
}

void SettingsDialog::saveCheckboxValue(QSettings *settings, QCheckBox *box,
                                       const QString &name)
{
    settings->setValue(name, checkStateToBool(box->checkState()));
}

void SettingsDialog::validateEditPythonPath()
{
    const auto pythonPath = mUI.mEditPythonPath->text();
    if (pythonPath.isEmpty()) {
        mUI.mEditPythonPath->setStyleSheet("");
        mUI.mPythonPathWarning->hide();
        return;
    }

    QFileInfo pythonPathInfo(pythonPath);
    if (!pythonPathInfo.exists() ||
        !pythonPathInfo.isFile() ||
        !pythonPathInfo.isExecutable()) {
        mUI.mEditPythonPath->setStyleSheet("QLineEdit {border: 1px solid red}");
        mUI.mPythonPathWarning->setText(tr("The executable file \"%1\" is not available").arg(pythonPath));
        mUI.mPythonPathWarning->show();
    } else {
        mUI.mEditPythonPath->setStyleSheet("");
        mUI.mPythonPathWarning->hide();
    }
}

void SettingsDialog::addApplication()
{
    Application app;
    ApplicationDialog dialog(tr("Add a new application"), app, this);

    if (dialog.exec() == QDialog::Accepted) {
        mTempApplications->addApplication(app);
        mUI.mListWidget->addItem(app.getName());
    }
}

void SettingsDialog::removeApplication()
{
    QList<QListWidgetItem *> selected = mUI.mListWidget->selectedItems();
    foreach (QListWidgetItem *item, selected) {
        const int removeIndex = mUI.mListWidget->row(item);
        const int currentDefault = mTempApplications->getDefaultApplication();
        mTempApplications->removeApplication(removeIndex);
        if (removeIndex == currentDefault)
            // If default app is removed set default to unknown
            mTempApplications->setDefault(-1);
        else if (removeIndex < currentDefault)
            // Move default app one up if earlier app was removed
            mTempApplications->setDefault(currentDefault - 1);
    }
    mUI.mListWidget->clear();
    populateApplicationList();
}

void SettingsDialog::editApplication()
{
    QList<QListWidgetItem *> selected = mUI.mListWidget->selectedItems();
    QListWidgetItem *item = nullptr;
    foreach (item, selected) {
        int row = mUI.mListWidget->row(item);
        Application& app = mTempApplications->getApplication(row);
        ApplicationDialog dialog(tr("Modify an application"), app, this);

        if (dialog.exec() == QDialog::Accepted) {
            QString name = app.getName();
            if (mTempApplications->getDefaultApplication() == row)
                name += tr(" [Default]");
            item->setText(name);
        }
    }
}

void SettingsDialog::defaultApplication()
{
    QList<QListWidgetItem *> selected = mUI.mListWidget->selectedItems();
    if (!selected.isEmpty()) {
        int index = mUI.mListWidget->row(selected[0]);
        mTempApplications->setDefault(index);
        mUI.mListWidget->clear();
        populateApplicationList();
    }
}

void SettingsDialog::populateApplicationList()
{
    const int defapp = mTempApplications->getDefaultApplication();
    for (int i = 0; i < mTempApplications->getApplicationCount(); i++) {
        const Application& app = mTempApplications->getApplication(i);
        QString name = app.getName();
        if (i == defapp) {
            name += " ";
            name += tr("[Default]");
        }
        mUI.mListWidget->addItem(name);
    }

    // Select default application, or if there is no default app then the
    // first item.
    if (defapp == -1)
        mUI.mListWidget->setCurrentRow(0);
    else {
        if (mTempApplications->getApplicationCount() > defapp)
            mUI.mListWidget->setCurrentRow(defapp);
        else
            mUI.mListWidget->setCurrentRow(0);
    }
}

void SettingsDialog::ok()
{
    mApplications->copy(mTempApplications);
    accept();
}

bool SettingsDialog::showFullPath() const
{
    return checkStateToBool(mUI.mShowFullPath->checkState());
}

bool SettingsDialog::saveFullPath() const
{
    return checkStateToBool(mUI.mSaveFullPath->checkState());
}

bool SettingsDialog::saveAllErrors() const
{
    return checkStateToBool(mUI.mSaveAllErrors->checkState());
}

bool SettingsDialog::showNoErrorsMessage() const
{
    return checkStateToBool(mUI.mShowNoErrorsMessage->checkState());
}

bool SettingsDialog::showErrorId() const
{
    return checkStateToBool(mUI.mShowErrorId->checkState());
}

bool SettingsDialog::showInconclusive() const
{
    return checkStateToBool(mUI.mEnableInconclusive->checkState());
}

void SettingsDialog::browsePythonPath()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select python binary"), QDir::rootPath());
    if (fileName.contains("python", Qt::CaseInsensitive))
        mUI.mEditPythonPath->setText(fileName);
}

void SettingsDialog::browseMisraFile()
{
    const QString fileName = QFileDialog::getOpenFileName(this, tr("Select MISRA File"), QDir::homePath(), "Misra File (*.pdf *.txt)");
    if (!fileName.isEmpty())
        mUI.mEditMisraFile->setText(fileName);
}

// Slot to set default light style
void SettingsDialog::setCodeEditorStyleDefault()
{
    if (mUI.mThemeSystem->isChecked())
        *mCurrentStyle = CodeEditorStyle::getSystemTheme();
    if (mUI.mThemeLight->isChecked())
        *mCurrentStyle = defaultStyleLight;
    if (mUI.mThemeDark->isChecked())
        *mCurrentStyle = defaultStyleDark;
    manageStyleControls();
}

// Slot to edit custom style
void SettingsDialog::editCodeEditorStyle()
{
    StyleEditDialog dlg(*mCurrentStyle, this);
    int nResult = dlg.exec();
    if (nResult == QDialog::Accepted) {
        *mCurrentStyle = dlg.getStyle();
        manageStyleControls();
    }
}

void SettingsDialog::browseClangPath()
{
    QString selectedDir = QFileDialog::getExistingDirectory(this,
                          tr("Select clang path"),
                          QDir::rootPath());

    if (!selectedDir.isEmpty()) {
        mUI.mEditClangPath->setText(selectedDir);
    }
}

void SettingsDialog::manageStyleControls()
{
    bool isSystemTheme = mCurrentStyle->isSystemTheme();
    bool isDefaultLight = !isSystemTheme && *mCurrentStyle == defaultStyleLight;
    bool isDefaultDark =  !isSystemTheme && *mCurrentStyle == defaultStyleDark;
    mUI.mThemeSystem->setChecked(isSystemTheme);
    mUI.mThemeLight->setChecked(isDefaultLight && !isDefaultDark);
    mUI.mThemeDark->setChecked(!isDefaultLight && isDefaultDark);
    mUI.mThemeCustom->setChecked(!isSystemTheme && !isDefaultLight && !isDefaultDark);
    mUI.mBtnEditTheme->setEnabled(!isSystemTheme && !isDefaultLight && !isDefaultDark);
}

