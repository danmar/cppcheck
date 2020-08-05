#ifndef ASSISTANT_H
#define ASSISTANT_H

#include <QCoreApplication>
#include <QString>

class QProcess;

class Assistant {
    Q_DECLARE_TR_FUNCTIONS(Assistant)

public:
    Assistant();
    ~Assistant();
    void showDocumentation(const QString &file);

private:
    bool startAssistant();
    QProcess *mProc;
};

#endif // ASSISTANT_H
