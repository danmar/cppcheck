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
    solution.load(QDir::homePath() + "/.staticanalysis.xml");

    connect(ui->projectwidget, SIGNAL(scan()), this, SLOT(scan()));

    // TODO: right now we don't show anything on the status bar
    this->statusBar()->hide();

    ApplicationSettings settings;
    if (settings.currentProject.isEmpty() && !solution.projects.isEmpty()) {
        settings.currentProject = solution.projectNames().first();
        settings.save();
    }
    ui->projectwidget->setProject(settings.currentProject);

    ui->projects->addItems(solution.projectNames());
    ui->projects->setCurrentIndex(solution.projectNames().indexOf(settings.currentProject));

    QHBoxLayout *layout = new QHBoxLayout;
    ui->workarea->setLayout(layout);

    resultsForm = new ResultsForm(this);
    resultsForm->hide();
    layout->addWidget(resultsForm);
}

MainWindow::~MainWindow()
{
    solution.save(QDir::homePath() + "/.staticanalysis.xml");
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
    ConfigureProjects *dialog = new ConfigureProjects(this,solution);
    ApplicationSettings settings;
    settings.currentProject = ui->projects->currentText();
    if (dialog->exec() == QDialog::Accepted) {
        solution.swap(dialog->solution);
        ui->projects->clear();
        const QStringList s = solution.projectNames();
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
    resultsForm->show();

    const Solution::Project *project = solution.getproject(ui->projectwidget->getProjectName());
    if (project) {
        resultsForm->scan(*project);
    }
}

void MainWindow::log()
{
    resultsForm->show();
    resultsForm->showResults(ui->projectwidget->getProjectName());
}
