/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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
#include <QThread>
#include "settingsdialog.h"
#include "applicationdialog.h"
#include "applicationlist.h"
#include "translationhandler.h"
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
    QSettings settings;
    mTempApplications->Copy(list);

    mUI.mJobs->setText(settings.value(SETTINGS_CHECK_THREADS, 1).toString());
    mUI.mForce->setCheckState(BoolToCheckState(settings.value(SETTINGS_CHECK_FORCE, false).toBool()));
    mUI.mShowFullPath->setCheckState(BoolToCheckState(settings.value(SETTINGS_SHOW_FULL_PATH, false).toBool()));
    mUI.mShowNoErrorsMessage->setCheckState(BoolToCheckState(settings.value(SETTINGS_SHOW_NO_ERRORS, false).toBool()));
    mUI.mShowDebugWarnings->setCheckState(BoolToCheckState(settings.value(SETTINGS_SHOW_DEBUG_WARNINGS, false).toBool()));
    mUI.mSaveAllErrors->setCheckState(BoolToCheckState(settings.value(SETTINGS_SAVE_ALL_ERRORS, false).toBool()));
    mUI.mSaveFullPath->setCheckState(BoolToCheckState(settings.value(SETTINGS_SAVE_FULL_PATH, false).toBool()));
    mUI.mInlineSuppressions->setCheckState(BoolToCheckState(settings.value(SETTINGS_INLINE_SUPPRESSIONS, false).toBool()));
    mUI.mEnableInconclusive->setCheckState(BoolToCheckState(settings.value(SETTINGS_INCONCLUSIVE_ERRORS, false).toBool()));
    mUI.mShowErrorId->setCheckState(BoolToCheckState(settings.value(SETTINGS_SHOW_ERROR_ID, false).toBool()));

    connect(mUI.mButtons, SIGNAL(accepted()), this, SLOT(Ok()));
    connect(mUI.mButtons, SIGNAL(rejected()), this, SLOT(reject()));
    connect(mUI.mBtnAddApplication, SIGNAL(clicked()),
            this, SLOT(AddApplication()));
    connect(mUI.mBtnRemoveApplication, SIGNAL(clicked()),
            this, SLOT(RemoveApplication()));
    connect(mUI.mBtnEditApplication, SIGNAL(clicked()),
            this, SLOT(EditApplication()));
    connect(mUI.mBtnDefaultApplication, SIGNAL(clicked()),
            this, SLOT(DefaultApplication()));
    connect(mUI.mListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
            this, SLOT(EditApplication()));
    connect(mUI.mBtnAddIncludePath, SIGNAL(clicked()),
            this, SLOT(AddIncludePath()));
    connect(mUI.mBtnRemoveIncludePath, SIGNAL(clicked()),
            this, SLOT(RemoveIncludePath()));
    connect(mUI.mBtnEditIncludePath, SIGNAL(clicked()),
            this, SLOT(EditIncludePath()));

    mUI.mListWidget->setSortingEnabled(false);
    PopulateApplicationList();

    const int count = QThread::idealThreadCount();
    if (count != -1)
        mUI.mLblIdealThreads->setText(QString::number(count));
    else
        mUI.mLblIdealThreads->setText(tr("N/A"));

    LoadSettings();
    InitTranslationsList();
    InitIncludepathsList();
}

SettingsDialog::~SettingsDialog()
{
    SaveSettings();
}

void SettingsDialog::AddIncludePath(const QString &path)
{
    if (path.isNull() || path.isEmpty())
        return;

    QListWidgetItem *item = new QListWidgetItem(path);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    mUI.mListIncludePaths->addItem(item);
}

void SettingsDialog::InitIncludepathsList()
{
    QSettings settings;
    const QString allPaths = settings.value(SETTINGS_GLOBAL_INCLUDE_PATHS).toString();
    const QStringList paths = allPaths.split(";", QString::SkipEmptyParts);
    foreach(QString path, paths) {
        AddIncludePath(path);
    }
}

void SettingsDialog::InitTranslationsList()
{
    const QString current = mTranslator->GetCurrentLanguage();
    QList<TranslationInfo> translations = mTranslator->GetTranslations();
    foreach(TranslationInfo translation, translations) {
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(translation.mName);
        item->setData(LangCodeRole, QVariant(translation.mCode));
        mUI.mListLanguages->addItem(item);
        if (translation.mCode == current || translation.mCode == current.mid(0, 2))
            mUI.mListLanguages->setCurrentItem(item);
    }
}

Qt::CheckState SettingsDialog::BoolToCheckState(bool yes)
{
    if (yes) {
        return Qt::Checked;
    }
    return Qt::Unchecked;
}

bool SettingsDialog::CheckStateToBool(Qt::CheckState state)
{
    if (state == Qt::Checked) {
        return true;
    }
    return false;
}


void SettingsDialog::LoadSettings()
{
    QSettings settings;
    resize(settings.value(SETTINGS_CHECK_DIALOG_WIDTH, 800).toInt(),
           settings.value(SETTINGS_CHECK_DIALOG_HEIGHT, 600).toInt());
}

void SettingsDialog::SaveSettings() const
{
    QSettings settings;
    settings.setValue(SETTINGS_CHECK_DIALOG_WIDTH, size().width());
    settings.setValue(SETTINGS_CHECK_DIALOG_HEIGHT, size().height());
}

void SettingsDialog::SaveSettingValues() const
{
    int jobs = mUI.mJobs->text().toInt();
    if (jobs <= 0) {
        jobs = 1;
    }

    QSettings settings;
    settings.setValue(SETTINGS_CHECK_THREADS, jobs);
    SaveCheckboxValue(&settings, mUI.mForce, SETTINGS_CHECK_FORCE);
    SaveCheckboxValue(&settings, mUI.mSaveAllErrors, SETTINGS_SAVE_ALL_ERRORS);
    SaveCheckboxValue(&settings, mUI.mSaveFullPath, SETTINGS_SAVE_FULL_PATH);
    SaveCheckboxValue(&settings, mUI.mShowFullPath, SETTINGS_SHOW_FULL_PATH);
    SaveCheckboxValue(&settings, mUI.mShowNoErrorsMessage, SETTINGS_SHOW_NO_ERRORS);
    SaveCheckboxValue(&settings, mUI.mShowDebugWarnings, SETTINGS_SHOW_DEBUG_WARNINGS);
    SaveCheckboxValue(&settings, mUI.mInlineSuppressions, SETTINGS_INLINE_SUPPRESSIONS);
    SaveCheckboxValue(&settings, mUI.mEnableInconclusive, SETTINGS_INCONCLUSIVE_ERRORS);
    SaveCheckboxValue(&settings, mUI.mShowErrorId, SETTINGS_SHOW_ERROR_ID);

    const QListWidgetItem *currentLang = mUI.mListLanguages->currentItem();
    if (currentLang) {
        const QString langcode = currentLang->data(LangCodeRole).toString();
        settings.setValue(SETTINGS_LANGUAGE, langcode);
    }

    const int count = mUI.mListIncludePaths->count();
    QString includePaths;
    for (int i = 0; i < count; i++) {
        QListWidgetItem *item = mUI.mListIncludePaths->item(i);
        includePaths += item->text();
        includePaths += ";";
    }
    settings.setValue(SETTINGS_GLOBAL_INCLUDE_PATHS, includePaths);
}

void SettingsDialog::SaveCheckboxValue(QSettings *settings, QCheckBox *box,
                                       const QString &name)
{
    settings->setValue(name, CheckStateToBool(box->checkState()));
}

void SettingsDialog::AddApplication()
{
    Application app;
    ApplicationDialog dialog(tr("Add a new application"), app, this);

    if (dialog.exec() == QDialog::Accepted) {
        mTempApplications->AddApplication(app);
        mUI.mListWidget->addItem(app.getName());
    }
}

void SettingsDialog::RemoveApplication()
{
    QList<QListWidgetItem *> selected = mUI.mListWidget->selectedItems();
    foreach(QListWidgetItem *item, selected) {
        const int removeIndex = mUI.mListWidget->row(item);
        const int currentDefault = mTempApplications->GetDefaultApplication();
        mTempApplications->RemoveApplication(removeIndex);
        if (removeIndex == currentDefault)
            // If default app is removed set default to unknown
            mTempApplications->SetDefault(-1);
        else if (removeIndex < currentDefault)
            // Move default app one up if earlier app was removed
            mTempApplications->SetDefault(currentDefault - 1);
    }
    mUI.mListWidget->clear();
    PopulateApplicationList();
}

void SettingsDialog::EditApplication()
{
    QList<QListWidgetItem *> selected = mUI.mListWidget->selectedItems();
    QListWidgetItem *item = 0;
    foreach(item, selected) {
        int row = mUI.mListWidget->row(item);
        Application& app = mTempApplications->GetApplication(row);
        ApplicationDialog dialog(tr("Modify an application"), app, this);

        if (dialog.exec() == QDialog::Accepted) {
            item->setText(app.getName());
        }
    }
}

void SettingsDialog::DefaultApplication()
{
    QList<QListWidgetItem *> selected = mUI.mListWidget->selectedItems();
    if (!selected.isEmpty()) {
        int index = mUI.mListWidget->row(selected[0]);
        mTempApplications->SetDefault(index);
        mUI.mListWidget->clear();
        PopulateApplicationList();
    }
}

void SettingsDialog::PopulateApplicationList()
{
    const int defapp = mTempApplications->GetDefaultApplication();
    for (int i = 0; i < mTempApplications->GetApplicationCount(); i++) {
        const Application& app = mTempApplications->GetApplication(i);
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
        if (mTempApplications->GetApplicationCount() > defapp)
            mUI.mListWidget->setCurrentRow(defapp);
        else
            mUI.mListWidget->setCurrentRow(0);
    }
}

void SettingsDialog::Ok()
{
    mApplications->Copy(mTempApplications);
    accept();
}

bool SettingsDialog::ShowFullPath() const
{
    return CheckStateToBool(mUI.mShowFullPath->checkState());
}

bool SettingsDialog::SaveFullPath() const
{
    return CheckStateToBool(mUI.mSaveFullPath->checkState());
}

bool SettingsDialog::SaveAllErrors() const
{
    return CheckStateToBool(mUI.mSaveAllErrors->checkState());
}

bool SettingsDialog::ShowNoErrorsMessage() const
{
    return CheckStateToBool(mUI.mShowNoErrorsMessage->checkState());
}

bool SettingsDialog::ShowErrorId() const
{
    return CheckStateToBool(mUI.mShowErrorId->checkState());
}

void SettingsDialog::AddIncludePath()
{
    QString selectedDir = QFileDialog::getExistingDirectory(this,
                          tr("Select include directory"),
                          GetPath(SETTINGS_LAST_INCLUDE_PATH));

    if (!selectedDir.isEmpty()) {
        AddIncludePath(selectedDir);
        SetPath(SETTINGS_LAST_INCLUDE_PATH, selectedDir);
    }
}

void SettingsDialog::RemoveIncludePath()
{
    const int row = mUI.mListIncludePaths->currentRow();
    QListWidgetItem *item = mUI.mListIncludePaths->takeItem(row);
    delete item;
}

void SettingsDialog::EditIncludePath()
{
    QListWidgetItem *item = mUI.mListIncludePaths->currentItem();
    mUI.mListIncludePaths->editItem(item);
}
