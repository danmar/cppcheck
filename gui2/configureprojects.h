#ifndef ConfigureProjects_H
#define ConfigureProjects_H

#include "solution.h"
#include <QDialog>

namespace Ui {
    class ConfigureProjects;
}

class ConfigureProjects : public QDialog {
    Q_OBJECT

public:
    explicit ConfigureProjects(QWidget *parent, const Solution &solution_);
    ~ConfigureProjects();

    Solution solution;

private slots:
    void selectProject();
    void newProject();
    void deleteProject();
    void pathBrowse();
    void newInclude();
    void deleteInclude();
    void apply();
    void saveCompilerSettings();

private:
    Ui::ConfigureProjects *ui;
};

#endif // ConfigureProjects_H
