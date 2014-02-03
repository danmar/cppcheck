#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "projectlist.h"

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class ResultsForm;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void settings();
    void configureProjects();
    void selectProject(QString projectName);

    // Project widget..
    void scan();
    void log();

private:
    Ui::MainWindow *ui;
    ProjectList projectList;
    ResultsForm *resultsForm;
};

#endif // MAINWINDOW_H
