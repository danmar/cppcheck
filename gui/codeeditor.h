#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QSyntaxHighlighter>
#include <QPlainTextEdit>
#include <QObject>
#include <QRegularExpression>
#include "codeeditorstyle.h"

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;

class LineNumberArea;


class Highlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit Highlighter(QTextDocument *parent,
                         CodeEditorStyle *widgetStyle);

    void setSymbols(const QStringList &symbols);

    void setStyle(const CodeEditorStyle &newStyle);

protected:
    void highlightBlock(const QString &text) override;

private:
    enum RuleRole {
        Keyword = 1,
        Class   = 2,
        Comment = 3,
        Quote   = 4,
        Symbol  = 5
    };
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
        RuleRole ruleRole;
    };

    void applyFormat(HighlightingRule &rule);

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

    CodeEditorStyle *mWidgetStyle;
};

class CodeEditor : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent);
    CodeEditor(const CodeEditor &) = delete;
    CodeEditor &operator=(const CodeEditor &) = delete;
    ~CodeEditor();

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    void setStyle(const CodeEditorStyle& newStyle);

    /**
     * Set source code to show, goto error line and highlight that line.
     * \param code         The source code.
     * \param errorLine    line number
     * \param symbols      the related symbols, these are marked
     */
    void setError(const QString &code, int errorLine, const QStringList &symbols);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightErrorLine();
    void updateLineNumberArea(const QRect &, int);

private:
    QString generateStyleString();

private:
    QWidget *mLineNumberArea;
    Highlighter *mHighlighter;
    CodeEditorStyle *mWidgetStyle;
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
