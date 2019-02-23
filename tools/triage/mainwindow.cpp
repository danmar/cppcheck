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
    std::srand(static_cast<unsigned int>(std::time(Q_NULLPTR)));
    QDir workFolder(WORK_FOLDER);
    if (!workFolder.exists()) {
        workFolder.mkdir(WORK_FOLDER);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadFile()
{
    ui->statusBar->clearMessage();
    const QString fileName = QFileDialog::getOpenFileName(this, tr("daca results file"), WORK_FOLDER, tr("Text files (*.txt);;All (*.*)"));
    if (fileName.isEmpty())
        return;
    QFile file(fileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream textStream(&file);
    load(textStream);
}

void MainWindow::loadFromClipboard()
{
    ui->statusBar->clearMessage();
    QString clipboardContent = QApplication::clipboard()->text();
    QTextStream textStream(&clipboardContent);
    load(textStream);
}

void MainWindow::load(QTextStream &textStream)
{
    QString url;
    QString errorMessage;
    QStringList allErrors;
    ui->results->clear();
    while (true) {
        QString line = textStream.readLine();
        if (line.isNull())
            break;
        if (line.startsWith("ftp://")) {
            url = line;
            if (!errorMessage.isEmpty())
                allErrors << errorMessage;
            errorMessage.clear();
        } else if (!url.isEmpty() && QRegExp(".*: (error|warning|style|note):.*").exactMatch(line)) {
            if (QRegExp("^(head|1.[0-9][0-9]) .*").exactMatch(line))
                line = line.mid(5);
            if (line.indexOf(": note:") > 0)
                errorMessage += '\n' + line;
            else if (errorMessage.isEmpty()) {
                errorMessage = url + '\n' + line;
            } else {
                allErrors << errorMessage;
                errorMessage = url + '\n' + line;
            }
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

bool MainWindow::runProcess(const QString &programName, const QStringList &arguments)
{
    QProcess process;
    process.setWorkingDirectory(WORK_FOLDER);
    process.start(programName, arguments);
    bool success = process.waitForFinished(-1);
    if (!success) {
        QString errorstr(programName);
        errorstr.append(": ");
        errorstr.append(process.errorString());
        ui->statusBar->showMessage(errorstr);
    } else {
        int exitCode = process.exitCode();
        if (exitCode != 0) {
            success = false;
            const QByteArray stderrOutput = process.readAllStandardError();
            QString errorstr(programName);
            errorstr.append(QString(": exited with %1: ").arg(exitCode));
            errorstr.append(stderrOutput);
            ui->statusBar->showMessage(errorstr);
        }
    }
    return success;
}

bool MainWindow::wget(const QString &url)
{
    return runProcess("wget", QStringList() << url);
}

bool MainWindow::unpackArchive(const QString &archiveName)
{
    // Unpack archive
    QStringList args;
#ifdef Q_OS_WIN
    /* On Windows --force-local is necessary because tar wants to connect to a remote system
     * when a colon is found in the archiveName. So "C:/Users/blah/triage/package" would not work
     * without it. */
    args << "--force-local";
#endif
    if (archiveName.endsWith(".tar.gz"))
        args << "-xzvf";
    else if (archiveName.endsWith(".tar.bz2"))
        args << "-xjvf";
    else if (archiveName.endsWith(".tar.xz"))
        args << "-xJvf";
    else {
        // Try to automatically find an (un)compressor for this archive
        args << "-xavf";
    }
    args << archiveName;

    return runProcess("tar", args);
}

void MainWindow::showResult(QListWidgetItem *item)
{
    ui->statusBar->clearMessage();
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
    const QString fileName = WORK_FOLDER + '/' + msg.left(msg.indexOf(":"));
    const int lineNumber = msg.midRef(pos1+1,pos2-pos1-1).toInt();

    if (!QFileInfo::exists(fileName)) {
        if (QFileInfo::exists(DACA2_PACKAGES + '/' + archiveName.mid(0,archiveName.indexOf(".tar.")) + ".tar.xz")) {
            if (!unpackArchive(DACA2_PACKAGES + '/' + archiveName.mid(0,archiveName.indexOf(".tar.")) + ".tar.xz"))
                return;
        } else {
            if (!QFileInfo::exists(WORK_FOLDER + '/' + archiveName)) {
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
    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString errorMsg =
            QString("Opening file %1 failed: %2").arg(f.fileName(), f.errorString());
        ui->statusBar->showMessage(errorMsg);
    } else {
        QTextStream textStream(&f);
        const QString fileData = textStream.readAll();
        ui->code->setError(fileData, lineNumber, QStringList());

        ui->edit1->setText(url);
        ui->edit2->setText(fileName);
        f.close();
    }
}
