#ifndef NETWORKINFO_H
#define NETWORKINFO_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QString>

/// @addtogroup GUI
/// @{


/**
 * Network communications with cppcheck website to get current version etc
 */
class NetworkInfo : public QObject
{
    Q_OBJECT
public:
    NetworkInfo(QObject *parent);
    ~NetworkInfo();

    void start();
signals:
    void cppcheckVersion(QString version);
private slots:
    void managerFinished(QNetworkReply *reply);
private:
    QNetworkAccessManager *mManager;
    QNetworkRequest request;
};
/// @}
#endif // NETWORKINFO_H
