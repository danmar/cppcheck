#include "configureprojects.h"
#include "ui_configureprojects.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

ConfigureProjects::ConfigureProjects(QWidget *parent, const ProjectList &projectlist_) :
    QDialog(parent),
    projectList(projectlist_),
    ui(new Ui::ConfigureProjects)
{
    ui->setupUi(this);
    ui->projects->addItems(projectList.projectNames());
    ui->projects->setCurrentRow(0);
}

ConfigureProjects::~ConfigureProjects()
{
    delete ui;
}

void ConfigureProjects::selectProject()
{
    const QString projectName = ui->projects->currentItem()->text();
    const ProjectList::Project *project = projectList.getproject(projectName);

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
        foreach(const ProjectList::Project *p, projectList.projects) {
            if (p->name == projectName) {
                exists = true;
                break;
            }
        }

        if (!exists)
            break;
    }
    ProjectList::Project *project = new ProjectList::Project;
    project->name = projectName;
    projectList.projects.append(project);
    ui->projects->addItem(projectName);
    ui->projects->sortItems();
}

void ConfigureProjects::deleteProject()
{
    const QString currentString = ui->projects->currentItem()->text();
    for (int i = 0; i < projectList.projects.size(); ++i) {
        if (projectList.projects[i]->name == currentString) {
            delete projectList.projects[i];
            projectList.projects.removeAt(i);
        }
    }
    delete ui->projects->currentItem();
}

void ConfigureProjects::nameChanged(QString name)
{
    if (name.isEmpty())
        ui->nameStatus->setText(tr("Empty name"));
    else if (name != ui->projects->currentItem()->text() && projectList.getproject(name))
        ui->nameStatus->setText(tr("Duplicate name"));
    else
        ui->nameStatus->setText("OK");
}

void ConfigureProjects::pathBrowse()
{
    const QString dir = QFileDialog::getExistingDirectory(this, tr("Select path to scan"));
    if (!dir.isEmpty()) {
        ui->path->setText(dir);
        apply();
    }
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

    ProjectList::Project *project = projectList.getproject(oldName);
    if (!newName.isEmpty() && !projectList.getproject(newName))
        project->name = newName;
    project->path    = ui->path->text();
    project->defines = ui->defines->text().split("[; ]");
    project->includes.clear();
    foreach(const QListWidgetItem *item, ui->includes->findItems(".*",Qt::MatchRegExp)) {
        project->includes << item->text();
    }

    const QStringList s = projectList.projectNames();
    ui->projects->clear();
    ui->projects->addItems(s);
    ui->projects->setCurrentRow(s.indexOf(project->name));
}

void ConfigureProjects::saveCompilerSettings()
{
    ProjectList::Project *project = projectList.getproject(ui->projects->currentItem()->text());
    project->clang.compiler       = ui->clangCompiler->isChecked();
    project->clang.analyser       = ui->clangAnalyser->isChecked();
    project->cppcheck.enabled     = ui->cppcheckEnabled->isChecked();
    project->gcc.enabled          = ui->gccEnabled->isChecked();
}
