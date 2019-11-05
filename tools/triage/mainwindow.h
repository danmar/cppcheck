#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QString>
#include <QTextStream>
#include <QFileSystemModel>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = Q_NULLPTR);
    MainWindow(const MainWindow &) = delete;
    MainWindow &operator=(const MainWindow &) = delete;
    ~MainWindow();

public slots:
    void loadFile();
    void loadFromClipboard();
    void filter(QString filter);
    void showResult(QListWidgetItem *item);
    void refreshResults();
    void fileTreeFilter(QString str);
    void findInFilesClicked();
    void directorytreeDoubleClick();
    void searchResultsDoubleClick();

private:
    Ui::MainWindow *ui;

    void load(QTextStream &textStream);
    bool runProcess(const QString &programName, const QStringList & arguments);
    bool wget(const QString &url);
    bool unpackArchive(const QString &archiveName);
    void showSrcFile(const QString &fileName, const QString &url, const int lineNumber);

    QStringList mAllErrors;
    QFileSystemModel mFSmodel;
    const QRegExp mVersionRe;

    const QStringList hFiles;
    const QStringList srcFiles;
};

#endif // MAINWINDOW_H
