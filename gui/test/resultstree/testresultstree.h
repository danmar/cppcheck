#include <QObject>

class TestResultsTree : public QObject {
    Q_OBJECT

private slots:
    void test1() const;
};
