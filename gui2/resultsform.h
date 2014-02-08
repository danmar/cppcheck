#ifndef RESULTSFORM_H
#define RESULTSFORM_H

#include "projectlist.h"
#include <QModelIndex>
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

    void scan(const ProjectList::Project &project);
    void showResults(const ProjectList::Project &project);

private slots:
    void scanAddResult();
    void scanFinished();

    void contextMenu(QPoint pos);

    void triage(QModelIndex index);
    void falsePositive();
    void truePositive();

private:
    Ui::ResultsForm *ui;
    ResultsModel    *resultsmodel;
    QModelIndex      triageIndex;

    struct ScanData {
        QProcess             *process;
        ProjectList::Project  project;
        QStringList           files;
        int                   filenum;
        int                   analyser;
    } currentScan;

};

#endif // RESULTSFORM_H
