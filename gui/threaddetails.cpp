#include "threaddetails.h"
#include "ui_threaddetails.h"

ThreadDetails* ThreadDetails::mInstance;

ThreadDetails::ThreadDetails(QWidget *parent)
    : QWidget(parent)
    , mUi(new Ui::ThreadDetails)
{
    mInstance = this;
    setWindowFlags(Qt::Window);
    mUi->setupUi(this);
}

ThreadDetails::~ThreadDetails()
{
    mInstance = nullptr;
    delete mUi;
}

void ThreadDetails::threadDetailsUpdated(QMap<int, CheckThread::Details> threadDetails)
{
    mUi->tableWidget->setRowCount(threadDetails.size());
    int row = 0;
    for (const auto& td: threadDetails) {
        mUi->tableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(td.threadIndex)));
        mUi->tableWidget->setItem(row, 1, new QTableWidgetItem(QString(td.startTime.toString(Qt::TextDate))));
        mUi->tableWidget->setItem(row, 2, new QTableWidgetItem(td.file));
        ++row;
    }
}
