#ifndef COMPLIANCEREPORTDIALOG_H
#define COMPLIANCEREPORTDIALOG_H

#include <QDialog>
#include <QObject>
#include <QString>

namespace Ui {
    class ComplianceReportDialog;
}

class ProjectFile;
class QAbstractButton;

class ComplianceReportDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit ComplianceReportDialog(ProjectFile* projectFile, QString resultsFile);
    ~ComplianceReportDialog() final;

private slots:
    void buttonClicked(QAbstractButton* button);

private:
    void save();

    Ui::ComplianceReportDialog *mUI;
    ProjectFile* mProjectFile;
    QString mResultsFile;
};

#endif // COMPLIANCEREPORTDIALOG_H
