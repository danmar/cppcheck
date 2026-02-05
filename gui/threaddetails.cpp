#include "threaddetails.h"
#include "ui_threaddetails.h"

ThreadDetails::ThreadDetails(QWidget *parent)
    : QWidget(parent)
    , mUi(new Ui::ThreadDetails)
{
    setWindowFlags(Qt::Window);
    mUi->setupUi(this);
    mUi->detailsBox->setReadOnly(true);
}

ThreadDetails::~ThreadDetails()
{
    delete mUi;
}

void ThreadDetails::threadDetailsUpdated(const QString &text)
{
    mUi->detailsBox->setText(text);
}
