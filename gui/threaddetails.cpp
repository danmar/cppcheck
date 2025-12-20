#include "threaddetails.h"
#include "ui_threaddetails.h"

ThreadDetails::ThreadDetails(QWidget *parent)
    : QWidget(parent)
    , mUi(new Ui::ThreadDetails)
{
    mUi->setupUi(this);
}

ThreadDetails::~ThreadDetails()
{
    delete mUi;
}
