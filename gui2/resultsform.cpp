#include "resultsform.h"
#include "applicationsettings.h"
#include "resultsmodel.h"
#include "ui_resultsform.h"
#include <QDate>
#include <QDebug>
#include <QDir>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QTextCodec>

ResultsForm::ResultsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ResultsForm)
{
    ui->setupUi(this);
    resultsmodel = new ResultsModel(this);
    ui->results->setModel(resultsmodel);

    currentScan.analyser = 0;
    currentScan.filenum  = 0;
    currentScan.files.clear();
    currentScan.process  = 0;
    currentScan.project  = ProjectList::Project();
}

ResultsForm::~ResultsForm()
{
    delete ui;
}

void ResultsForm::contextMenu(QPoint pos)
{
    const int row = ui->results->currentIndex().row();
    QMenu contextMenu(tr("Context menu"), this);
    QAction *hideId = new QAction(tr("Hide id"), &contextMenu);
    contextMenu.addAction(hideId);
    QAction *hideAllOtherId = new QAction(tr("Hide all other id"), &contextMenu);
    contextMenu.addAction(hideAllOtherId);
    QAction *showAll = new QAction(tr("Show all"), &contextMenu);
    contextMenu.addAction(showAll);
    const QAction *a = contextMenu.exec(mapToGlobal(pos));
    if (a==hideId)
        resultsmodel->hideId(row);
    else if (a==hideAllOtherId)
        resultsmodel->hideAllOtherId(row);
    else if (a==showAll)
        resultsmodel->showAll();
}


static QStringList filelist(const QString &path)
{
    QStringList files;
    QDir dir(path);
    dir.setSorting(QDir::Name);

    dir.setNameFilters(QStringList() << "*");
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach(const QString dirname, dir.entryList()) {
        files += filelist(path + "/" + dirname);
    }

    dir.setNameFilters(QStringList() << "*.cpp" << "*.c");
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    foreach(const QString file, dir.entryList()) {
        files << path + "/" + file;
    }

    return files;
}

void ResultsForm::scan(const ProjectList::Project &project)
{
    qDebug() << "ResultsForm::scan()";
    if (currentScan.process)
        return;

    currentScan.process = new QProcess(this);
    currentScan.project = project;
    connect(currentScan.process, SIGNAL(readyReadStandardError()), this, SLOT(scanAddResult()));
    connect(currentScan.process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(scanFinished()));
    resultsmodel->clear();
    ui->progressBar->setVisible(true);
    ui->progressBar->setEnabled(true);
    ui->progressBar->setValue(0);

    currentScan.files = filelist(project.path);
    currentScan.filenum = 0;
    currentScan.analyser = 0;
    scanFinished();
}

void ResultsForm::scanAddResult()
{
    const QString err = QTextCodec::codecForMib(106)->toUnicode(currentScan.process->readAllStandardError());
    if (!err.isEmpty()) {
        const QStringList err2(err.split(QRegExp("[\r\n]")));
        const QString &path = currentScan.project.path;
        foreach(QString errmsg, err2) {
            if (errmsg.startsWith(path) && (errmsg[path.size()]=='\\' || errmsg[path.size()]=='/'))
                errmsg = errmsg.mid(path.size()+1);
            resultsmodel->addresult(errmsg);
        }
    }
}

void ResultsForm::scanFinished()
{
    ApplicationSettings settings;
    const QString commands[] = { QString(),
                                 settings.clang,
                                 settings.cppcheck,
                                 settings.gcc
                               };

    const bool enabled[] = { false,
                             currentScan.project.clang.compiler,
                             currentScan.project.cppcheck.enabled,
                             currentScan.project.gcc.enabled
                           };

    // Scanning of a file was finished. Scan next file..
    QString cmd;
    while (cmd.isEmpty() && currentScan.filenum < currentScan.files.size()) {
        currentScan.analyser = (currentScan.analyser + 1) & 3;
        if (currentScan.analyser == 0)
            ++currentScan.filenum;
        if (!enabled[currentScan.analyser])
            continue;
        cmd = commands[currentScan.analyser];
        if (cmd.indexOf("/")>=0 && !QFileInfo(cmd).exists())
            cmd.clear();
    }

    QStringList args;
    if (cmd == settings.gcc) {
        args << "-fsyntax-only";
        args << "--std=c++11";
        args << "-Wall";
        args << "-Wextra";
        args << "-pedantic";
        args << "-Wabi";
        args << "-Wcast-qual";
        args << "-Wconversion";
        args << "-Wfloat-equal";
        args << "-Winline";
        args << "-Wlogical-op";
        args << "-Wmissing-declarations";
        args << "-Wmissing-format-attribute";
        args << "-Wno-long-long";
        args << "-Woverloaded-virtual";
        args << "-Wpacked";
        args << "-Wredundant-decls";
        args << "-Wshadow";
        args << "-Wsign-conversion";
        args << "-Wsign-promo";
        args << "-Wunreachable-code";
        args << "-Wno-sign-compare";
    } else if (cmd == settings.clang) {
        args << "-fsyntax-only";
        args << "--std=c++11";
        args << "-Weverything";
    } else if (cmd == settings.cppcheck) {
        args << "--template={file}:{line}:{severity}:{message} [{id}]" << "--enable=style" << "--inconclusive";
    }

    if (currentScan.filenum >= currentScan.files.size()) {
        ui->progressBar->setValue(0);
        ui->progressBar->setEnabled(false);

        // Save results to file so they can be viewed later
        QString filename;
        for (int i = 1; i < 10000; i++) {
            filename = settings.resultsFolder + "/"
                       + QDate::currentDate().toString("yyyy-MM-dd-")
                       + QString().sprintf("%i",i)
                       + ".xml";
            if (!QFileInfo(filename).exists())
                break;
        }
        if (!resultsmodel->save(filename, currentScan.project.name))
            QMessageBox::warning(this, QObject::tr("Results"), QObject::tr("Failed to save results"));

        // Clear "currentScan"
        currentScan.process  = NULL;
        currentScan.analyser = 0;
        currentScan.filenum  = 0;
        currentScan.files.clear();
        currentScan.project  = ProjectList::Project();
    } else {
        currentScan.process->start(cmd, args << currentScan.files[currentScan.filenum]);
        ui->progressBar->setValue(100 * currentScan.filenum / currentScan.files.size());
    }
}

static QString lastResultFile(const QString &projectName)
{
    QString ret;

    ApplicationSettings settings;
    QDir dir(settings.resultsFolder);
    dir.setSorting(QDir::Name);
    dir.setNameFilters(QStringList() << "*.xml");
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    foreach(const QFileInfo fileinfo, dir.entryInfoList()) {
        const QString filename = fileinfo.canonicalFilePath();
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;

        QDomDocument doc;
        if (!doc.setContent(&file))
            continue;

        const QDomElement rootElement = doc.documentElement();
        if (rootElement.tagName() != "results")
            continue;

        const QDomElement metaElement = rootElement.firstChildElement("meta");
        if (metaElement.isNull())
            continue;

        const QDomElement projectElement = metaElement.firstChildElement("project");
        if (projectElement.text() == projectName)
            ret = filename;
    }

    return ret;
}

void ResultsForm::showResults(const QString & projectName)
{
    if (currentScan.process == 0) {
        ui->progressBar->setVisible(false);
        resultsmodel->load(lastResultFile(projectName));
    }
}


