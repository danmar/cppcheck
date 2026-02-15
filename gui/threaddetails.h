#ifndef THREADDETAILS_H
#define THREADDETAILS_H

#include <QWidget>
#include <QMap>
#include "checkthread.h"

namespace Ui {
    class ThreadDetails;
}

class ThreadDetails : public QWidget
{
    Q_OBJECT

public:
    explicit ThreadDetails(QWidget *parent = nullptr);
    ~ThreadDetails() override;

    static ThreadDetails* instance() {
        return mInstance;
    }

public slots:
    void threadDetailsUpdated(const QMap<int, CheckThread::Details>& threadDetails);

private:
    static ThreadDetails* mInstance;
    Ui::ThreadDetails *mUi;
};

#endif // THREADDETAILS_H
