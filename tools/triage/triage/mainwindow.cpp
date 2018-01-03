#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QClipboard>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>

const QString WORK_FOLDER(QDir::homePath() + "/triage");

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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
    while (true) {
        const QString line = textStream.readLine();
        if (line.isNull())
            break;
        if (line.startsWith("ftp://"))
            url = line;
        else if (!url.isEmpty()) {
            ui->results->addItem(url + "\n" + line);
            url.clear();
        }
    }
}

void MainWindow::showResult(QListWidgetItem *item)
{
    if (!item->text().startsWith("ftp://"))
        return;
    const QStringList lines = item->text().split("\n");
    if (lines.size() != 2)
        return;
    const QString url = lines[0];
    const QString msg = lines[1];

    const QString archiveName = url.mid(url.lastIndexOf("/") + 1);
    const int pos1 = msg.indexOf(":");
    const int pos2 = msg.indexOf(":", pos1+1);
    const QString fileName = msg.left(msg.indexOf(":"));
    const int lineNumber = msg.mid(pos1+1,pos2-pos1-1).toInt();

    QProcess process;
    process.setWorkingDirectory(WORK_FOLDER);

    if (!QFileInfo(WORK_FOLDER + '/' + archiveName).exists()) {
        // Download archive
        process.start("wget", QStringList()<<url);
        if (!process.waitForFinished(-1))
            return;

        // Unpack archive
        QStringList args;
        if (url.endsWith(".tar.gz"))
            args << "xzvf";
        else if (url.endsWith(".tar.bz2"))
            args << "xjvf";
        else if (url.endsWith(".tar.xz"))
            args << "xJvf";
        args << archiveName;
        process.start("tar", args);
        if (!process.waitForFinished(-1))
            return;
    }

    // Open file
    ui->code->setFocus();
    QFile f(WORK_FOLDER + '/' + fileName);
    f.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream textStream(&f);
    const QString fileData = textStream.readAll();
    ui->code->setPlainText(fileData);
    for (int pos = 0, line = 1; pos < fileData.size(); ++pos) {
        if (fileData[pos] == '\n') {
            ++line;
            if (line == lineNumber) {
                QTextCursor textCursor = ui->code->textCursor();
                textCursor.setPosition(pos+1);
                ui->code->setTextCursor(textCursor);
                ui->code->centerCursor();
                break;
            }
        }
    }
}
