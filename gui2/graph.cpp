#include "graph.h"
#include "applicationsettings.h"
#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <QPainter>

Graph::Graph(QWidget *parent) :
    QWidget(parent)
{
}


static QString getstr(const QDomElement element, const QString &tagName)
{
    const QDomElement child = element.firstChildElement(tagName);
    return child.isNull() ? QString() : child.text();
}

void Graph::diff(const QString &projectName)
{
    values.clear();
    datetimes.clear();

    ApplicationSettings settings;
    QDir dir(settings.resultsFolder);
    dir.setSorting(QDir::Name);
    dir.setNameFilters(QStringList() << "*.xml");
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

    QSet<QString> previousResults;
    bool first = true;

    foreach(const QFileInfo fileinfo, dir.entryInfoList()) {
        const QString filename = fileinfo.canonicalFilePath();
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;

        QDomDocument doc;
        if (!doc.setContent(&file))
            continue;

        const QDomElement rootElement = doc.documentElement();
        if (rootElement.tagName() != "results")
            continue;

        const QDomElement metaElement = rootElement.firstChildElement("meta");
        if (metaElement.isNull())
            continue;

        const QDomElement projectElement = metaElement.firstChildElement("project");
        if (projectElement.text() != projectName)
            continue;

        int added=0;
        QSet<QString> results;
        for (QDomElement child = rootElement.firstChildElement("results"); !child.isNull(); child = child.nextSiblingElement()) {
            const QString str = getstr(child,"id") + "," + getstr(child,"line") + "," + getstr(child,"file");
            results.insert(str);
            if (!first) {
                if (!previousResults.contains(str))
                    added++;
            }
        }
        previousResults = results;
        values.append(added);
        datetimes.append(fileinfo.baseName().mid(0,10));
        first = false;
    }

    if (isVisible())
        update();
}

void Graph::trend(const QString &projectName)
{
    values.clear();
    datetimes.clear();

    ApplicationSettings settings;
    QDir dir(settings.resultsFolder);
    dir.setSorting(QDir::Name);
    dir.setNameFilters(QStringList() << "*.xml");
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    foreach(const QFileInfo fileinfo, dir.entryInfoList()) {
        const QString filename = fileinfo.canonicalFilePath();
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;

        QDomDocument doc;
        if (!doc.setContent(&file))
            continue;

        const QDomElement rootElement = doc.documentElement();
        if (rootElement.tagName() != "results")
            continue;

        const QDomElement metaElement = rootElement.firstChildElement("meta");
        if (metaElement.isNull())
            continue;

        const QDomElement projectElement = metaElement.firstChildElement("project");
        if (projectElement.text() != projectName)
            continue;

        int results = 0;
        for (QDomElement child = rootElement.firstChildElement("results"); !child.isNull(); child = child.nextSiblingElement())
            ++results;
        values.append(results);
        datetimes.append(fileinfo.baseName().mid(0,10));
    }

    if (isVisible())
        update();
}


static void drawLineGraph(QPainter *painter, int left, int top, int width, int height, const QVector<int> &values)
{
    const int numberOfValues = values.size();

    int maxvalue = 0;
    for (int i = 0; i < numberOfValues; i++) {
        if (values[i] > maxvalue)
            maxvalue = values[i];
    }

    painter->setPen(Qt::gray);
    painter->setBrush(Qt::white);
    painter->drawRect(left,top,width,height);

    if (maxvalue == 0)
        return;

    const int bottom = top + height;
    const int w = width - 2;
    const int h = height - 2;

    painter->setPen(Qt::red);
    for (int i = 1; i < numberOfValues; i++) {
        painter->drawLine(left   + 1 + (i-1) * w / (numberOfValues - 1),
                          bottom - 1 - (h * values[i-1]) / maxvalue,
                          left   + 1 + i * w / (numberOfValues - 1),
                          bottom - 1 - (h * values[i]) / maxvalue);
    }
}


void Graph::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.fillRect(rect(),Qt::white);
    drawLineGraph(&painter,50,50,width()-100,height()-100,values);
    painter.setPen(Qt::black);
    painter.drawText(QRect(50,height()-40,100,20), Qt::AlignLeft, datetimes.front());
    painter.drawText(QRect(width()-150,height()-40,100,20), Qt::AlignRight, datetimes.back());
}

