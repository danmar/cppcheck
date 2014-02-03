#ifndef PROJECTWIDGET_H
#define PROJECTWIDGET_H

#include <QString>
#include <QWidget>

class ProjectWidget : public QWidget {
    Q_OBJECT
public:
    explicit ProjectWidget(QWidget *parent = 0);
    void setProject(const QString &projectName_);
    void getProjectName() const { return projectName; }

signals:
    void scan();
    void log();

private:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);

    QString projectName;
    QString lastResultsDate;
};

#endif // PROJECTWIDGET_H
