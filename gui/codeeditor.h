#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QSyntaxHighlighter>
#include <QPlainTextEdit>
#include <QObject>
#include <QRegularExpression>

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;

class LineNumberArea;


class Highlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit Highlighter(QTextDocument *parent);

    void setSymbols(const QStringList &symbols);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> mHighlightingRules;
    QVector<HighlightingRule> mHighlightingRulesWithSymbols;

    QRegularExpression mCommentStartExpression;
    QRegularExpression mCommentEndExpression;

    QTextCharFormat mKeywordFormat;
    QTextCharFormat mClassFormat;
    QTextCharFormat mSingleLineCommentFormat;
    QTextCharFormat mMultiLineCommentFormat;
    QTextCharFormat mQuotationFormat;
    QTextCharFormat mSymbolFormat;
};

class CodeEditor : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent);
    CodeEditor(const CodeEditor &) = delete;
    CodeEditor &operator=(const CodeEditor &) = delete;

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

    void setError(const QString &code, int errorLine, const QStringList &symbols);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightErrorLine();
    void updateLineNumberArea(const QRect &, int);

private:
    QWidget *mLineNumberArea;
    Highlighter *mHighlighter;
    int mErrorPosition;
};


class LineNumberArea : public QWidget {
public:
    explicit LineNumberArea(CodeEditor *editor) : QWidget(editor) {
        mCodeEditor = editor;
    }

    QSize sizeHint() const override {
        return QSize(mCodeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        mCodeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor *mCodeEditor;
};

#endif // CODEEDITOR_H
