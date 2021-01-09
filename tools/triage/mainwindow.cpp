#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QClipboard>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFileDialog>
#include <QMimeDatabase>
#include <ctime>
#include <cstdlib>

const QString WORK_FOLDER(QDir::homePath() + "/triage");
const QString DACA2_PACKAGES(QDir::homePath() + "/daca2-packages");

const int MAX_ERRORS = 100;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mVersionRe("^(master|main|your|head|[12].[0-9][0-9]?) (.*)"),
    hFiles{"*.hpp", "*.h", "*.hxx", "*.hh", "*.tpp", "*.txx"},
    srcFiles{"*.cpp", "*.cxx", "*.cc", "*.c++", "*.C", "*.c", "*.cl"}
{
    ui->setupUi(this);
    std::srand(static_cast<unsigned int>(std::time(Q_NULLPTR)));
    QDir workFolder(WORK_FOLDER);
    if (!workFolder.exists()) {
        workFolder.mkdir(WORK_FOLDER);
    }

    mFSmodel.setRootPath(WORK_FOLDER);
    mFSmodel.setReadOnly(true);
    mFSmodel.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    ui->directoryTree->setModel(&mFSmodel);
    QHeaderView * header =  ui->directoryTree->header();
    for (int i = 1; i < header->length(); ++i)  // hide all except [0]
        header->hideSection(i);
    ui->directoryTree->setRootIndex(mFSmodel.index(WORK_FOLDER));

    ui->hFilesFilter->setToolTip(hFiles.join(','));
    ui->srcFilesFilter->setToolTip(srcFiles.join(','));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadFile()
{
    ui->statusBar->clearMessage();
    const QString fileName = QFileDialog::getOpenFileName(this, tr("daca results file"), WORK_FOLDER, tr("Text files (*.txt *.log);;All (*.*)"));
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
    QStringList versions;
    mAllErrors.clear();
    while (true) {
        QString line = textStream.readLine();
        if (line.isNull())
            break;
        if (line.startsWith("ftp://")) {
            url = line;
            if (!errorMessage.isEmpty())
                mAllErrors << errorMessage;
            errorMessage.clear();
        } else if (!url.isEmpty() && QRegExp(".*: (error|warning|style|note):.*").exactMatch(line)) {
            if (mVersionRe.exactMatch(line)) {
                const QString version = mVersionRe.cap(1);
                if (versions.indexOf(version) < 0)
                    versions << version;
            }
            if (line.indexOf(": note:") > 0)
                errorMessage += '\n' + line;
            else if (errorMessage.isEmpty()) {
                errorMessage = url + '\n' + line;
            } else {
                mAllErrors << errorMessage;
                errorMessage = url + '\n' + line;
            }
        }
    }
    if (!errorMessage.isEmpty())
        mAllErrors << errorMessage;

    ui->version->clear();
    if (versions.size() > 1)
        ui->version->addItem("");
    ui->version->addItems(versions);

    filter("");
}

void MainWindow::refreshResults()
{
    filter(ui->version->currentText());
}

void MainWindow::filter(QString filter)
{
    QStringList allErrors;

    for (const QString &errorItem : mAllErrors) {
        if (filter.isEmpty()) {
            allErrors << errorItem;
            continue;
        }

        const QStringList lines = errorItem.split("\n");
        if (lines.size() < 2)
            continue;

        if (lines[1].startsWith(filter))
            allErrors << errorItem;
    }

    ui->results->clear();

    if (ui->random100->isChecked() && allErrors.size() > MAX_ERRORS) {
        // remove items in /test/
        for (int i = allErrors.size() - 1; i >= 0 && allErrors.size() > MAX_ERRORS; --i) {
            if (allErrors[i].indexOf("test") > 0)
                allErrors.removeAt(i);
        }
        std::random_shuffle(allErrors.begin(), allErrors.end());
        ui->results->addItems(allErrors.mid(0, MAX_ERRORS));
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
    return runProcess("wget", QStringList{url});
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
    if (mVersionRe.exactMatch(msg))
        msg = mVersionRe.cap(2);
    const QString archiveName = url.mid(url.lastIndexOf("/") + 1);
    const int pos1 = msg.indexOf(":");
    const int pos2 = msg.indexOf(":", pos1+1);
    const QString fileName = WORK_FOLDER + '/' + msg.left(msg.indexOf(":"));
    const int lineNumber = msg.midRef(pos1+1, pos2-pos1-1).toInt();

    if (!QFileInfo::exists(fileName)) {
        const QString daca2archiveFile {DACA2_PACKAGES + '/' + archiveName.mid(0,archiveName.indexOf(".tar.")) + ".tar.xz"};
        if (QFileInfo::exists(daca2archiveFile)) {
            if (!unpackArchive(daca2archiveFile))
                return;
        } else {
            const QString archiveFullPath {WORK_FOLDER + '/' + archiveName};
            if (!QFileInfo::exists(archiveFullPath)) {
                // Download archive
                if (!wget(url))
                    return;
            }
            if (!unpackArchive(archiveFullPath))
                return;
        }
    }
    showSrcFile(fileName, url, lineNumber);
}

void MainWindow::showSrcFile(const QString &fileName, const QString &url, const int lineNumber)
{
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
        f.close();

        ui->urlEdit->setText(url);
        ui->fileEdit->setText(fileName);
        ui->directoryTree->setCurrentIndex(mFSmodel.index(fileName));
    }
}

void MainWindow::fileTreeFilter(QString str)
{
    mFSmodel.setNameFilters(QStringList{"*" + str + "*"});
    mFSmodel.setNameFilterDisables(false);
}

void MainWindow::findInFilesClicked()
{
    ui->tabWidget->setCurrentIndex(1);
    ui->inFilesResult->clear();
    const QString text = ui->filterEdit->text();

    QStringList filter;
    if (ui->hFilesFilter->isChecked())
        filter.append(hFiles);
    if (ui->srcFilesFilter->isChecked())
        filter.append(srcFiles);

    QMimeDatabase mimeDatabase;
    QDirIterator it(WORK_FOLDER, filter, QDir::AllEntries | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

    const auto common_path_len = WORK_FOLDER.length() + 1;  // let's remove common part of path improve UI

    while (it.hasNext()) {
        const QString fileName = it.next();
        const QMimeType mimeType = mimeDatabase.mimeTypeForFile(fileName);

        if (mimeType.isValid() && !mimeType.inherits(QStringLiteral("text/plain"))) {
            continue;
        }

        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            QString line;
            int lineN = 0;
            QTextStream in(&file);
            while (!in.atEnd()) {
                ++lineN;
                line = in.readLine();
                if (line.contains(text, Qt::CaseInsensitive)) {
                    ui->inFilesResult->addItem(fileName.mid(common_path_len) + QString{":"} + QString::number(lineN));
                }
            }
        }
    }
}

void MainWindow::directorytreeDoubleClick()
{
    showSrcFile(mFSmodel.filePath(ui->directoryTree->currentIndex()), "", 1);
}

void MainWindow::searchResultsDoubleClick()
{
    QString filename = ui->inFilesResult->currentItem()->text();
    const auto idx = filename.lastIndexOf(':');
    const int line = filename.midRef(idx + 1).toInt();
    showSrcFile(WORK_FOLDER + QString{"/"} + filename.left(idx), "", line);
}
