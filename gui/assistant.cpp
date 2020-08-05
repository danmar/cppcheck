#include "assistant.h"

#include <QByteArray>
#include <QDir>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QProcess>

Assistant::Assistant()
    : mProc(nullptr)
{
}

Assistant::~Assistant()
{
    if (mProc && mProc->state() == QProcess::Running) {
        mProc->terminate();
        mProc->waitForFinished(3000);
    }
    delete mProc;
}

void Assistant::showDocumentation(const QString &page)
{
    if (!startAssistant())
        return;

    QByteArray ba("SetSource ");
    ba.append("qthelp://cppcheck.sourceforge.net/doc/");

    mProc->write(ba + page.toLocal8Bit() + '\n');
}

bool Assistant::startAssistant()
{
    if (!mProc)
        mProc = new QProcess();

    if (mProc->state() != QProcess::Running) {
        QString app = QLibraryInfo::location(QLibraryInfo::BinariesPath) + QDir::separator();
#if !defined(Q_OS_MAC)
        app += QLatin1String("assistant");
#else
        app += QLatin1String("Assistant.app/Contents/MacOS/Assistant");
#endif

        QStringList args;
        args << QLatin1String("-collectionFile")
             << QLatin1String("online-help.qhc")
             << QLatin1String("-enableRemoteControl");

        mProc->start(app, args);

        if (!mProc->waitForStarted()) {
            QMessageBox::critical(nullptr,
                                  tr("Cppcheck"),
                                  tr("Unable to launch Qt Assistant (%1)").arg(app));
            return false;
        }
    }
    return true;
}
