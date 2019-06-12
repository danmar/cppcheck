#include <QtWidgets>

#include "codeeditor.h"


Highlighter::Highlighter(QTextDocument *parent,
                         CodeEditorStyle *widgetStyle) :
    QSyntaxHighlighter(parent),
    mWidgetStyle(widgetStyle)
{
    HighlightingRule rule;

    mKeywordFormat.setForeground(mWidgetStyle->keywordColor);
    mKeywordFormat.setFontWeight(mWidgetStyle->keywordWeight);
    QStringList keywordPatterns;
    keywordPatterns << "bool"
                    << "break"
                    << "case"
                    << "char"
                    << "class"
                    << "const"
                    << "continue"
                    << "default"
                    << "do"
                    << "double"
                    << "else"
                    << "enum"
                    << "explicit"
                    << "for"
                    << "friend"
                    << "if"
                    << "inline"
                    << "int"
                    << "long"
                    << "namespace"
                    << "operator"
                    << "private"
                    << "protected"
                    << "public"
                    << "return"
                    << "short"
                    << "signed"
                    << "static"
                    << "struct"
                    << "switch"
                    << "template"
                    << "throw"
                    << "typedef"
                    << "typename"
                    << "union"
                    << "unsigned"
                    << "virtual"
                    << "void"
                    << "volatile"
                    << "while";
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression("\\b" + pattern + "\\b");
        rule.format = mKeywordFormat;
        mHighlightingRules.append(rule);
    }

    mClassFormat.setForeground(mWidgetStyle->classColor);
    mClassFormat.setFontWeight(mWidgetStyle->classWeight);
    rule.pattern = QRegularExpression("\\bQ[A-Za-z]+\\b");
    rule.format = mClassFormat;
    mHighlightingRules.append(rule);

    mQuotationFormat.setForeground(mWidgetStyle->quoteColor);
    mQuotationFormat.setFontWeight(mWidgetStyle->quoteWeight);
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = mQuotationFormat;
    mHighlightingRules.append(rule);

    mSingleLineCommentFormat.setForeground(mWidgetStyle->commentColor);
    mSingleLineCommentFormat.setFontWeight(mWidgetStyle->commentWeight);
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = mSingleLineCommentFormat;
    mHighlightingRules.append(rule);

    mHighlightingRulesWithSymbols = mHighlightingRules;

    mMultiLineCommentFormat.setForeground(mWidgetStyle->commentColor);
    mMultiLineCommentFormat.setFontWeight(mWidgetStyle->commentWeight);

    mSymbolFormat.setForeground(mWidgetStyle->symbolFGColor);
    mSymbolFormat.setBackground(mWidgetStyle->symbolBGColor);
    mSymbolFormat.setFontWeight(mWidgetStyle->symbolWeight);

    mCommentStartExpression = QRegularExpression("/\\*");
    mCommentEndExpression = QRegularExpression("\\*/");
}

void Highlighter::setSymbols(const QStringList &symbols)
{
    mHighlightingRulesWithSymbols = mHighlightingRules;
    foreach (const QString &sym, symbols) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression("\\b" + sym + "\\b");
        rule.format = mSymbolFormat;
        mHighlightingRulesWithSymbols.append(rule);
    }
}

void Highlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, mHighlightingRulesWithSymbols) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(mCommentStartExpression);

    while (startIndex >= 0) {
        QRegularExpressionMatch match = mCommentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                            + match.capturedLength();
        }
        setFormat(startIndex, commentLength, mMultiLineCommentFormat);
        startIndex = text.indexOf(mCommentStartExpression, startIndex + commentLength);
    }
}


CodeEditor::CodeEditor(QWidget *parent,
                       CodeEditorStyle *widgetStyle /*= nullptr*/) :
    QPlainTextEdit(parent)
{
    if (widgetStyle) mWidgetStyle = widgetStyle;
    else mWidgetStyle = new CodeEditorStyle(defaultStyle);

    mLineNumberArea = new LineNumberArea(this);
    mHighlighter = new Highlighter(this->document(), mWidgetStyle);
    mErrorPosition = -1;

    // set widget coloring by overriding widget style sheet
    QString bgcolor = QString("background:rgb(%1,%2,%3);")
                      .arg(mWidgetStyle->widgetBGColor.red())
                      .arg(mWidgetStyle->widgetBGColor.green())
                      .arg(mWidgetStyle->widgetBGColor.blue());
    QString fgcolor = QString("color:rgb(%1,%2,%3);")
                      .arg(mWidgetStyle->widgetFGColor.red())
                      .arg(mWidgetStyle->widgetFGColor.green())
                      .arg(mWidgetStyle->widgetFGColor.blue());
    QString style = QString("%1 %2")
                    .arg(bgcolor)
                    .arg(fgcolor);
    setObjectName("CodeEditor");
    setStyleSheet(style);

    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));

    updateLineNumberAreaWidth(0);
}

static int getPos(const QString &fileData, int lineNumber)
{
    if (lineNumber <= 1)
        return 0;
    for (int pos = 0, line = 1; pos < fileData.size(); ++pos) {
        if (fileData[pos] != '\n')
            continue;
        ++line;
        if (line >= lineNumber)
            return pos + 1;
    }
    return fileData.size();
}

void CodeEditor::setError(const QString &code, int errorLine, const QStringList &symbols)
{
    mHighlighter->setSymbols(symbols);

    setPlainText(code);

    mErrorPosition = getPos(code, errorLine);
    QTextCursor tc = textCursor();
    tc.setPosition(mErrorPosition);
    setTextCursor(tc);
    centerCursor();

    highlightErrorLine();
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;
    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        mLineNumberArea->scroll(0, dy);
    else
        mLineNumberArea->update(0, rect.y(), mLineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    QRect cr = contentsRect();
    mLineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::highlightErrorLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    QTextEdit::ExtraSelection selection;

    selection.format.setBackground(mWidgetStyle->highlightBGColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = QTextCursor(document());
    selection.cursor.setPosition(mErrorPosition);
    selection.cursor.clearSelection();
    extraSelections.append(selection);

    setExtraSelections(extraSelections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(mLineNumberArea);
    painter.fillRect(event->rect(), mWidgetStyle->lineNumBGColor);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(mWidgetStyle->lineNumFGColor);
            painter.drawText(0, top, mLineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}
