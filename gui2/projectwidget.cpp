#include "projectwidget.h"
#include <QDate>
#include <QMouseEvent>
#include <QPainter>

static const int buttonSize = 40;
static const int distance   = 4;

ProjectWidget::ProjectWidget(QWidget *parent) :
    QWidget(parent)
{
    projectName = "MyProject";
    lastResultsDate = QDate::currentDate().toString("yyyy-MM-dd");
}

void ProjectWidget::setProject(const QString &projectName_)
{
    projectName = projectName_;
    update();
}

static void drawLineGraph(QPainter *painter, int left, int top, int width, int height, const int values[], int numberOfValues)
{
    int maxvalue = 0;
    for (int i = 0; i < numberOfValues; i++) {
        if (values[i] > maxvalue)
            maxvalue = values[i];
    }

    painter->setPen(Qt::gray);
    painter->setBrush(Qt::white);
    painter->drawRect(left,top,width,height);

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


static void drawBarGraph(QPainter *painter, int left, int top, int width, int height, const int values[], int numberOfValues)
{
    int maxvalue = 0;
    for (int i = 0; i < numberOfValues; i++) {
        if (values[i] > maxvalue)
            maxvalue = values[i];
    }

    painter->setPen(Qt::gray);
    painter->setBrush(Qt::white);
    painter->drawRect(left,top,width,height);

    const int w = width - 2;
    const int h = height - 2;

    for (int i = 0; i < numberOfValues; i++) {
        if (values[i] <= 0)
            continue;

        int barwidth = w / numberOfValues - 2;
        int barheight = (h * values[i]) / maxvalue;

        painter->fillRect(left + 1 + i * w / numberOfValues,
                          top + height - 1 - barheight,
                          barwidth,
                          barheight,
                          Qt::red);
    }
}


void ProjectWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.fillRect(rect(),Qt::white);

    // title
    QFont titleFont("verdana", 18);
    painter.setFont(titleFont);
    const int titleTextHeight = painter.fontMetrics().height();
    painter.drawText(10, titleTextHeight, projectName);

    // last results
    QFont normalFont("verdana");
    painter.setFont(normalFont);
    const int normalTextHeight = painter.fontMetrics().height();
    painter.drawText(10, 4 + titleTextHeight + normalTextHeight, "Last results: " + lastResultsDate);

    int buttonx = width() - distance - buttonSize;

    painter.setRenderHint(QPainter::Antialiasing,true);

    // trend
    static const int results[] = {10,15,2,5,4};
    drawLineGraph(&painter, buttonx, distance, buttonSize, buttonSize, results, sizeof(results) / sizeof(results[0]));
    painter.setPen(Qt::black);
    painter.drawText(buttonx, distance + buttonSize + normalTextHeight, "Trend");
    buttonx -= distance + buttonSize;

    // new
    int diff[sizeof(results) / sizeof(results[0])] = {0};
    for (unsigned int i = 1; i < sizeof(results) / sizeof(results[0]); ++i)
        diff[i] = std::max(0, results[i] - results[i-1]);
    drawBarGraph(&painter, buttonx, distance, buttonSize, buttonSize, diff, sizeof(diff) / sizeof(diff[0]));
    painter.setPen(Qt::black);
    painter.drawText(buttonx, distance + buttonSize + normalTextHeight, "New");
    buttonx -= distance + buttonSize;

    // Log
    painter.setPen(Qt::gray);
    painter.setBrush(Qt::white);
    painter.drawRect(buttonx,distance,buttonSize,buttonSize);
    painter.setPen(Qt::black);
    for (int y = 3*distance; y < buttonSize-distance; y+=4)
        painter.drawLine(buttonx+2*distance, y, buttonx+buttonSize-2*distance, y);
    painter.drawText(buttonx, distance + buttonSize + normalTextHeight, "Log");
    buttonx -= distance + buttonSize;

    // Start scan
    painter.setPen(Qt::gray);
    painter.setBrush(Qt::white);
    painter.drawRect(buttonx,distance,buttonSize,buttonSize);
    QPoint darkpoints[3] = {
        QPoint(buttonx+buttonSize/2-10, distance+buttonSize/2-10),
        QPoint(buttonx+buttonSize/2+10, distance+buttonSize/2),
        QPoint(buttonx+buttonSize/2-10, distance+buttonSize/2+10),
    };
    painter.setBrush(Qt::darkGreen);
    painter.setPen(Qt::white);
    painter.drawPolygon(darkpoints, 3);
    painter.setPen(Qt::black);
    painter.drawText(buttonx, distance + buttonSize + normalTextHeight, "Scan");
    buttonx -= distance + buttonSize;
}

void ProjectWidget::mousePressEvent(QMouseEvent *event)
{
    int button = (width() - event->pos().x()) / (distance + buttonSize);

    if (button == 2)
        emit(log());

    if (button == 3)
        emit(scan());
}
