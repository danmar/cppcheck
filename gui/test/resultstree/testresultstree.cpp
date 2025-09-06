#include "testresultstree.h"

#include <iostream>

#include <QStandardItemModel>
#include <QtTest>

class QCustomItem : public QStandardItem
{
public:
    explicit QCustomItem(QString str)
        : mStr(std::move(str))
    {}
    ~QCustomItem() override {
        std::cout << "~QCustomItem(" << mStr.toStdString() << ")" << std::endl;
    }
private:
    QString mStr;
};

void TestResultsTree::test1() const
{
    auto* model = new QStandardItemModel;
    auto *item = new QCustomItem("0");
    {
        QMap<QString, QVariant> itemdata;
        itemdata["file"] = "file1";
        item->setData(itemdata);
        itemdata["file"] = "file2";
        item->setData(itemdata); // crash
    }
    model/*->invisibleRootItem()*/->appendRow(item);
    model->clear();
    delete model;
}

QTEST_MAIN(TestResultsTree)
