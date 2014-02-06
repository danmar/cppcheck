#ifndef CODEBROWSER_H
#define CODEBROWSER_H

#include <QPlainTextEdit>

class CodeBrowser : public QPlainTextEdit {
    Q_OBJECT

public:
    CodeBrowser(QWidget *parent = 0);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

    void setLine(int line);

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &, int);

private:
    QWidget *lineNumberArea;
};

class LineNumberArea : public QWidget {
public:
    LineNumberArea(CodeBrowser *browser) : QWidget(browser) {
        codeBrowser = browser;
    }

    QSize sizeHint() const {
        return QSize(codeBrowser->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) {
        codeBrowser->lineNumberAreaPaintEvent(event);
    }

private:
    CodeBrowser *codeBrowser;
};

#endif // CODEBROWSER_H
