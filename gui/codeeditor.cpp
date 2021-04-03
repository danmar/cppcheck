#include "codeeditor.h"

#include <QShortcut>
#include <QPainter>
#include "codeeditorstyle.h"


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
        rule.ruleRole = RuleRole::Keyword;
        mHighlightingRules.append(rule);
    }

    mClassFormat.setForeground(mWidgetStyle->classColor);
    mClassFormat.setFontWeight(mWidgetStyle->classWeight);
    rule.pattern = QRegularExpression("\\bQ[A-Za-z]+\\b");
    rule.format = mClassFormat;
    rule.ruleRole = RuleRole::Class;
    mHighlightingRules.append(rule);

    mQuotationFormat.setForeground(mWidgetStyle->quoteColor);
    mQuotationFormat.setFontWeight(mWidgetStyle->quoteWeight);
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = mQuotationFormat;
    rule.ruleRole = RuleRole::Quote;
    mHighlightingRules.append(rule);

    mSingleLineCommentFormat.setForeground(mWidgetStyle->commentColor);
    mSingleLineCommentFormat.setFontWeight(mWidgetStyle->commentWeight);
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = mSingleLineCommentFormat;
    rule.ruleRole = RuleRole::Comment;
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
        rule.ruleRole = RuleRole::Symbol;
        mHighlightingRulesWithSymbols.append(rule);
    }
}

void Highlighter::setStyle(const CodeEditorStyle &newStyle)
{
    mKeywordFormat.setForeground(newStyle.keywordColor);
    mKeywordFormat.setFontWeight(newStyle.keywordWeight);
    mClassFormat.setForeground(newStyle.classColor);
    mClassFormat.setFontWeight(newStyle.classWeight);
    mSingleLineCommentFormat.setForeground(newStyle.commentColor);
    mSingleLineCommentFormat.setFontWeight(newStyle.commentWeight);
    mMultiLineCommentFormat.setForeground(newStyle.commentColor);
    mMultiLineCommentFormat.setFontWeight(newStyle.commentWeight);
    mQuotationFormat.setForeground(newStyle.quoteColor);
    mQuotationFormat.setFontWeight(newStyle.quoteWeight);
    mSymbolFormat.setForeground(newStyle.symbolFGColor);
    mSymbolFormat.setBackground(newStyle.symbolBGColor);
    mSymbolFormat.setFontWeight(newStyle.symbolWeight);
    for (HighlightingRule& rule : mHighlightingRules) {
        applyFormat(rule);
    }

    for (HighlightingRule& rule : mHighlightingRulesWithSymbols) {
        applyFormat(rule);
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

void Highlighter::applyFormat(HighlightingRule &rule)
{
    switch (rule.ruleRole) {
    case RuleRole::Keyword:
        rule.format = mKeywordFormat;
        break;
    case RuleRole::Class:
        rule.format = mClassFormat;
        break;
    case RuleRole::Comment:
        rule.format = mSingleLineCommentFormat;
        break;
    case RuleRole::Quote:
        rule.format = mQuotationFormat;
        break;
    case RuleRole::Symbol:
        rule.format = mSymbolFormat;
        break;
    }
}

CodeEditor::CodeEditor(QWidget *parent) :
    QPlainTextEdit(parent),
    mWidgetStyle(new CodeEditorStyle(defaultStyleLight))
{
    mLineNumberArea = new LineNumberArea(this);
    mHighlighter = new Highlighter(document(), mWidgetStyle);
    mErrorPosition = -1;

    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    setFont(font);
    mLineNumberArea->setFont(font);

    // set widget coloring by overriding widget style sheet
    setObjectName("CodeEditor");
    setStyleSheet(generateStyleString());

    QShortcut *copyText = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_C),this);
    QShortcut *allText = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_A),this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(copyText, SIGNAL(activated()), this, SLOT(copy()));
    connect(allText, SIGNAL(activated()), this, SLOT(selectAll()));

    updateLineNumberAreaWidth(0);
}

CodeEditor::~CodeEditor()
{
    // NOTE: not a Qt Object - delete manually
    delete mWidgetStyle;
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

void CodeEditor::setStyle(const CodeEditorStyle& newStyle)
{
    *mWidgetStyle = newStyle;
    // apply new styling
    setStyleSheet(generateStyleString());
    mHighlighter->setStyle(newStyle);
    mHighlighter->rehighlight();
    highlightErrorLine();
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

void CodeEditor::setError(int errorLine, const QStringList &symbols)
{
    mHighlighter->setSymbols(symbols);

    mErrorPosition = getPos(toPlainText(), errorLine);
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
    if (mErrorPosition >= 0) {
        selection.cursor.setPosition(mErrorPosition);
    } else {
        selection.cursor.setPosition(0);
    }
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

QString CodeEditor::generateStyleString()
{
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
    return style;
}
