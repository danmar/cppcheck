#include "mainwindow.h"
#include "applicationsettings.h"
#include "configureprojects.h"
#include "resultsform.h"
#include "settingsdialog.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDir>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    projectList.load(QDir::homePath() + "/.cppcheck-gui-2.xml");

    connect(ui->projectwidget, SIGNAL(scan()), this, SLOT(scan()));
    connect(ui->projectwidget, SIGNAL(log()), this, SLOT(log()));

    // TODO: right now we don't show anything on the status bar
    this->statusBar()->hide();

    ApplicationSettings settings;
    if (settings.currentProject.isEmpty() && !projectList.projects.isEmpty()) {
        settings.currentProject = projectList.projectNames().first();
        settings.save();
    }
    ui->projectwidget->setProject(settings.currentProject);

    ui->projects->addItems(projectList.projectNames());
    ui->projects->setCurrentIndex(projectList.projectNames().indexOf(settings.currentProject));

    QHBoxLayout *layout = new QHBoxLayout;
    ui->workarea->setLayout(layout);

    resultsForm = new ResultsForm(this);
    resultsForm->hide();
    layout->addWidget(resultsForm);
}

MainWindow::~MainWindow()
{
    projectList.save(QDir::homePath() + "/.cppcheck-gui-2.xml");
    delete ui;
}

void MainWindow::settings()
{
    qDebug() << "MainWindow::settings";
    SettingsDialog *dialog = new SettingsDialog(this);
    dialog->show();
}

void MainWindow::configureProjects()
{
    qDebug() << "MainWindow::configureProjects";
    ConfigureProjects *dialog = new ConfigureProjects(this,projectList);
    ApplicationSettings settings;
    settings.currentProject = ui->projects->currentText();
    if (dialog->exec() == QDialog::Accepted) {
        projectList.swap(dialog->projectList);
        ui->projects->clear();
        const QStringList s = projectList.projectNames();
        ui->projects->addItems(s);
        ui->projects->setCurrentIndex(s.indexOf(settings.currentProject));
    }
}

void MainWindow::selectProject(QString projectName)
{
    ApplicationSettings settings;
    settings.currentProject = projectName;
    settings.save();
    ui->projectwidget->setProject(projectName);
}

void MainWindow::scan()
{
    const ProjectList::Project *project = projectList.getproject(ui->projectwidget->getProjectName());
    if (project) {
        resultsForm->show();
        resultsForm->scan(*project);
    }
}

void MainWindow::log()
{
    const ProjectList::Project *project = projectList.getproject(ui->projectwidget->getProjectName());
    if (project) {
        resultsForm->show();
        resultsForm->showResults(*project);
    }
}
