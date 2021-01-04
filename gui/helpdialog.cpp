#include "helpdialog.h"
#include "ui_helpdialog.h"
#include "common.h"

#include <QFileInfo>
#include <QHelpEngine>
#include <QHelpContentWidget>
#include <QHelpIndexWidget>
#include <QMessageBox>

void HelpBrowser::setHelpEngine(QHelpEngine *helpEngine)
{
    mHelpEngine = helpEngine;
}

QVariant HelpBrowser::loadResource(int type, const QUrl &name)
{
    if (name.scheme() == "qthelp") {
        QString url(name.toString());
        while (url.indexOf("/./") > 0)
            url.remove(url.indexOf("/./"), 2);
        return QVariant(mHelpEngine->fileData(QUrl(url)));
    }
    return QTextBrowser::loadResource(type, name);
}

static QString getHelpFile()
{
    const QString datadir = getDataDir();

    QStringList paths;
    paths << (datadir + "/help")
          << datadir
          << (QApplication::applicationDirPath() + "/help")
          << QApplication::applicationDirPath();
#ifdef FILESDIR
    const QString filesdir = FILESDIR;
    paths << (filesdir + "/help")
          << filesdir;
#endif
    for (QString p: paths) {
        QString filename = p + "/online-help.qhc";
        if (QFileInfo(filename).exists())
            return filename;
    }
    return QString();
}

HelpDialog::HelpDialog(QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::HelpDialog)
{
    mUi->setupUi(this);

    QString helpFile = getHelpFile();
    if (helpFile.isEmpty()) {
        const QString msg = tr("Helpfile '%1' was not found").arg("online-help.qhc");
        QMessageBox msgBox(QMessageBox::Warning,
                           tr("Cppcheck"),
                           msg,
                           QMessageBox::Ok,
                           this);
        msgBox.exec();
        mHelpEngine = nullptr;
        return;
    }

    mHelpEngine = new QHelpEngine(helpFile);
    // Disable the timestamp check of online-help.qhc by setting _q_readonly
    mHelpEngine->setProperty("_q_readonly", QVariant::fromValue<bool>(true));
    mHelpEngine->setupData();

    mUi->contents->addWidget(mHelpEngine->contentWidget());
    mUi->index->addWidget(mHelpEngine->indexWidget());

    mUi->textBrowser->setHelpEngine(mHelpEngine);

    mUi->textBrowser->setSource(QUrl("qthelp://cppcheck.sourceforge.net/doc/index.html"));
    connect(mHelpEngine->contentWidget(),
            SIGNAL(linkActivated(QUrl)),
            mUi->textBrowser,
            SLOT(setSource(QUrl)));

    connect(mHelpEngine->indexWidget(),
            SIGNAL(linkActivated(QUrl, QString)),
            mUi->textBrowser,
            SLOT(setSource(QUrl)));
}

HelpDialog::~HelpDialog()
{
    delete mUi;
}
