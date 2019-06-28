#include "networkinfo.h"
#include <QNetworkReply>
#include <QNetworkRequest>

NetworkInfo::NetworkInfo(QObject *parent) : QObject(parent)
{
    mManager = new QNetworkAccessManager;
    connect(mManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(managerFinished(QNetworkReply*)));
}

NetworkInfo::~NetworkInfo()
{
    delete mManager;
}

void NetworkInfo::start()
{
    //QNetworkRequest request;
    request.setUrl(QUrl("http://cppcheck.sourceforge.net/version.txt"));
    mManager->get(request);
}

void NetworkInfo::managerFinished(QNetworkReply *reply) {
    if (reply->error()) {
        qDebug() << reply->errorString();
        return;
    }

    emit cppcheckVersion(reply->readAll().trimmed());
}
