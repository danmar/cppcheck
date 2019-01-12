#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = Q_NULLPTR);
    MainWindow(const MainWindow &) = delete;
    ~MainWindow();
    MainWindow &operator=(const MainWindow &) = delete;

public slots:
    void loadFile();
    void showResult(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;

    bool runProcess(const QString &programName, const QStringList & arguments);
    bool wget(const QString url);
    bool unpackArchive(const QString archiveName);
};

#endif // MAINWINDOW_H
