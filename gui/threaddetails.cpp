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

void ThreadDetails::threadDetailsUpdated(const QMap<int, CheckThread::Details>& threadDetails)
{
    QString text("Thread\tStart time\tFile\n");
    for (const auto& td: threadDetails) {
        text += QString("%1\t%2\t%3\n").arg(td.threadIndex).arg(td.startTime.toString(Qt::TextDate)).arg(td.file);
    }
    mUi->plainTextEdit->setPlainText(text);
}
