#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("Cppcheck");
    QCoreApplication::setOrganizationDomain("cppcheck.sf.net");
    QCoreApplication::setApplicationName("Cppcheck GUI 2");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
