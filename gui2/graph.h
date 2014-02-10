#ifndef GRAPH_H
#define GRAPH_H

#include <QPaintEvent>
#include <QString>
#include <QVector>
#include <QWidget>

class Graph : public QWidget {
    Q_OBJECT
public:
    explicit Graph(QWidget *parent = 0);
    void trend(const QString &projectName);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QVector<int> values;
    QVector<QString> datetimes;

};

#endif // GRAPH_H
