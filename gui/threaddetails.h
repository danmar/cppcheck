#ifndef THREADDETAILS_H
#define THREADDETAILS_H

#include <QWidget>

namespace Ui {
    class ThreadDetails;
}

class ThreadDetails : public QWidget
{
    Q_OBJECT

public:
    explicit ThreadDetails(QWidget *parent = nullptr);
    ~ThreadDetails() override;

public slots:
    void threadDetailsUpdated(const QString &text);

private:
    Ui::ThreadDetails *mUi;
};

#endif // THREADDETAILS_H
