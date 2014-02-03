#ifndef RESULTSFORM_H
#define RESULTSFORM_H

#include "solution.h"
#include <QProcess>
#include <QWidget>

namespace Ui {
    class ResultsForm;
}

class ResultsModel;

class ResultsForm : public QWidget {
    Q_OBJECT

public:
    explicit ResultsForm(QWidget *parent = 0);
    ~ResultsForm();

    void scan(const Solution::Project &project);
    void showResults(const QString &projectName);

private slots:
    void scanAddResult();
    void scanFinished();

private:
    Ui::ResultsForm *ui;
    ResultsModel *resultsmodel;

    struct ScanData {
        QProcess          *process;
        Solution::Project  project;
        QStringList        files;
        int                filenum;
        int                analyser;
    } currentScan;

};

#endif // RESULTSFORM_H
