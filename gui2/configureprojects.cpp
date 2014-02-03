#include "configureprojects.h"
#include "ui_configureprojects.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

ConfigureProjects::ConfigureProjects(QWidget *parent, const Solution &solution_) :
    QDialog(parent),
    solution(solution_),
    ui(new Ui::ConfigureProjects)
{
    ui->setupUi(this);
    ui->projects->addItems(solution.projectNames());
    ui->projects->setCurrentRow(0);
}

ConfigureProjects::~ConfigureProjects()
{
    delete ui;
}

void ConfigureProjects::selectProject()
{
    const QString projectName = ui->projects->currentItem()->text();
    const Solution::Project *project = solution.getproject(projectName);

    ui->name->setText(project ? project->name : QString());
    ui->path->setText(project ? project->path : QString());
    ui->defines->clear();
    ui->includes->clear();
    if (project != 0) {
        QString defines;
        foreach(const QString def, project->defines) {
            defines += " " + def;
        }
        ui->defines->setText(defines);

        ui->includes->addItems(project->includes);

        ui->clangCompiler->setChecked(project->clang.compiler);
        ui->clangAnalyser->setChecked(project->clang.analyser);
        ui->cppcheckEnabled->setChecked(project->cppcheck.enabled);
        ui->gccEnabled->setChecked(project->gcc.enabled);
    }
}

void ConfigureProjects::newProject()
{
    QString projectName;
    for (int i = 1; i < 1000; i++) {
        projectName = "Project" + QString().sprintf("%i",i);
        bool exists = false;
        foreach(const Solution::Project *p, solution.projects) {
            if (p->name == projectName) {
                exists = true;
                break;
            }
        }

        if (!exists)
            break;
    }
    Solution::Project *project = new Solution::Project();
    project->name = projectName;
    solution.projects.append(project);
    ui->projects->addItem(projectName);
    ui->projects->sortItems();
}

void ConfigureProjects::deleteProject()
{
    const QString currentString = ui->projects->currentItem()->text();
    for (int i = 0; i < solution.projects.size(); ++i) {
        if (solution.projects[i]->name == currentString) {
            delete solution.projects[i];
            solution.projects.removeAt(i);
        }
    }
    delete ui->projects->currentItem();
}

void ConfigureProjects::pathBrowse()
{
    const QString dir = QFileDialog::getExistingDirectory(this, tr("Select path to scan"));
    if (!dir.isEmpty())
        ui->path->setText(dir);
}

void ConfigureProjects::newInclude()
{
    const QString dir = QFileDialog::getExistingDirectory(this,
                        tr("Select directory to include"));
    if (!dir.isEmpty()) {
        ui->includes->addItem(dir);
    }
}

void ConfigureProjects::deleteInclude()
{
    ui->includes->removeItemWidget(ui->includes->currentItem());
}

void ConfigureProjects::apply()
{
    const QString oldName = ui->projects->currentItem()->text();
    const QString newName = ui->name->text();
    if (oldName != newName && solution.getproject(newName)) {
        QMessageBox::warning(this, QObject::tr("Results"), QObject::tr("Failed to apply changes - project name already exists"));
        return;
    }

    Solution::Project *project = solution.getproject(oldName);
    project->name    = newName;
    project->path    = ui->path->text();
    project->defines = ui->defines->text().split("[; ]");
    project->includes.clear();
    foreach(const QListWidgetItem *item, ui->includes->findItems(".*",Qt::MatchRegExp)) {
        project->includes << item->text();
    }

    const QStringList s = solution.projectNames();
    ui->projects->clear();
    ui->projects->addItems(s);
    ui->projects->setCurrentRow(s.indexOf(newName));
}

void ConfigureProjects::saveCompilerSettings()
{
    Solution::Project *project = solution.getproject(ui->projects->currentItem()->text());
    project->clang.compiler    = ui->clangCompiler->isChecked();
    project->clang.analyser    = ui->clangAnalyser->isChecked();
    project->cppcheck.enabled  = ui->cppcheckEnabled->isChecked();
    project->gcc.enabled       = ui->gccEnabled->isChecked();
}
