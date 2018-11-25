#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QClipboard>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <ctime>

const QString WORK_FOLDER(QDir::homePath() + "/triage");
const QString DACA2_PACKAGES(QDir::homePath() + "/daca2-packages");

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    std::srand(std::time(0));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadFile()
{
    const QString fileName = QFileDialog::getOpenFileName(this, tr("daca results file"), WORK_FOLDER, tr("Text files (*.txt)"));
    if (fileName.isEmpty())
        return;
    ui->results->clear();
    QFile file(fileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream textStream(&file);
    QString url;
    QString errorMessage;
    QStringList allErrors;
    while (true) {
        const QString line = textStream.readLine();
        if (line.isNull())
            break;
        if (line.startsWith("ftp://")) {
            if (!url.isEmpty() && !errorMessage.isEmpty())
                allErrors << (url + "\n" + errorMessage);
            url = line;
            errorMessage.clear();
        } else if (!url.isEmpty() && QRegExp(".*: (error|warning|style|note):.*").exactMatch(line)) {
            if (line.indexOf(": note:") > 0)
                errorMessage += '\n' + line;
            else if (errorMessage.isEmpty()) {
                errorMessage = url + '\n' + line;
            } else {
                allErrors << errorMessage;
                errorMessage = url + '\n' + line;
            }
        } else if (!url.isEmpty() && QRegExp("^(head|1.[0-9][0-9]) .*:[0-9]+:.*\\]").exactMatch(line)) {
            allErrors << (url + '\n' + line);
        }
    }
    if (!errorMessage.isEmpty())
        allErrors << errorMessage;
    if (allErrors.size() > 100) {
        // remove items in /test/
        for (int i = allErrors.size()-1; i >= 0 && allErrors.size() > 100; --i) {
            if (allErrors[i].indexOf("test") > 0)
                allErrors.removeAt(i);
        }
        std::random_shuffle(allErrors.begin(), allErrors.end());
        ui->results->addItems(allErrors.mid(0,100));
        ui->results->sortItems();
    } else {
        ui->results->addItems(allErrors);
    }
}

static bool wget(const QString url)
{
    QProcess process;
    process.setWorkingDirectory(WORK_FOLDER);
    process.start("wget", QStringList() << url);
    return process.waitForFinished(-1);
}

static bool unpackArchive(const QString archiveName)
{
    // Unpack archive
    QStringList args;
    if (archiveName.endsWith(".tar.gz"))
        args << "xzvf";
    else if (archiveName.endsWith(".tar.bz2"))
        args << "xjvf";
    else if (archiveName.endsWith(".tar.xz"))
        args << "xJvf";
    args << archiveName;

    QProcess process;
    process.setWorkingDirectory(WORK_FOLDER);
    process.start("tar", args);
    return process.waitForFinished(-1);
}

void MainWindow::showResult(QListWidgetItem *item)
{
    if (!item->text().startsWith("ftp://"))
        return;
    const QStringList lines = item->text().split("\n");
    if (lines.size() < 2)
        return;
    const QString url = lines[0];
    QString msg = lines[1];
    if (QRegExp("^(head|1.[0-9][0-9]) .*").exactMatch(msg))
        msg = msg.mid(5);
    const QString archiveName = url.mid(url.lastIndexOf("/") + 1);
    const int pos1 = msg.indexOf(":");
    const int pos2 = msg.indexOf(":", pos1+1);
    const QString fileName = msg.left(msg.indexOf(":"));
    const int lineNumber = msg.mid(pos1+1,pos2-pos1-1).toInt();

    if (!QFileInfo(fileName).exists()) {
        if (QFileInfo(DACA2_PACKAGES + '/' + archiveName.mid(0,archiveName.indexOf(".tar.")) + ".tar.xz").exists()) {
            if (!unpackArchive(DACA2_PACKAGES + '/' + archiveName.mid(0,archiveName.indexOf(".tar.")) + ".tar.xz"))
                return;
        } else {
            if (!QFileInfo(WORK_FOLDER + '/' + archiveName).exists()) {
                // Download archive
                if (!wget(url))
                    return;
            }
            if (!unpackArchive(WORK_FOLDER + '/' + archiveName))
                return;
        }
    }

    // Open file
    ui->code->setFocus();
    QFile f(WORK_FOLDER + '/' + fileName);
    f.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream textStream(&f);
    const QString fileData = textStream.readAll();
    ui->code->setError(fileData, lineNumber, QStringList());

    ui->edit1->setText(url);
    ui->edit2->setText(WORK_FOLDER + '/' + fileName);
}
