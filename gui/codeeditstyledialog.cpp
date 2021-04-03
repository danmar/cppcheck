/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "codeeditstyledialog.h"

#include "codeeditor.h"
#include "codeeditstylecontrols.h"
#include <QFormLayout>
#include <QDialogButtonBox>

const QString StyleEditDialog::mSampleDocument(
    "/*****\n"
    "* Multiline Comment\n"
    "*****/\n"
    "#include <QApplication>\n"
    "#include <iostream>\n"
    "\n"
    "class fwdClass;\n"
    "\n"
    "int main(int argc, char *argv[])\n"
    "{\n"
    "    QApplication a(argc, argv);\n"
    "    int nLife = 42;\n"
    "    w.show();\n"
    "    // single line comment\n"
    "    // line below is highlighted\n"
    "    fwdClass( nLife );\n"
    "    return a.exec();\n"
    "}\n"
    "\n"
    "void class fwdClass( double dValue ) {\n"
    "    std::cout << \"Ipsum Lorem: \"\n"
    "              << nValue\n"
    "              << std::endl;\n"
    "}\n");

const QStringList StyleEditDialog::mErrSymbolsList = (
            QStringList(QStringList()
                        << "nLife"
                        << "dValue"
                        << "nValue"));
const int StyleEditDialog::mErrLineNum = 16;

StyleEditDialog::StyleEditDialog(const CodeEditorStyle& newStyle,
                                 QWidget *parent /*= nullptr*/) :
    QDialog(parent),
    mStyleIncoming(newStyle),
    mStyleOutgoing(newStyle)
{
    QVBoxLayout *vboxMain = new QVBoxLayout(this);
    QHBoxLayout *hboxEdit = new QHBoxLayout();
    // Color/Weight controls
    QFormLayout *flEditControls = new QFormLayout();
    mBtnWidgetColorFG = new SelectColorButton(this);
    flEditControls->addRow(QObject::tr("Editor Foreground Color"),
                           mBtnWidgetColorFG);
    mBtnWidgetColorBG = new SelectColorButton(this);
    flEditControls->addRow(QObject::tr("Editor Background Color"),
                           mBtnWidgetColorBG);
    mBtnHighlightBG = new SelectColorButton(this);
    flEditControls->addRow(QObject::tr("Highlight Background Color"),
                           mBtnHighlightBG);
    mBtnLineNumFG = new SelectColorButton(this);
    flEditControls->addRow(QObject::tr("Line Number Foreground Color"),
                           mBtnLineNumFG);
    mBtnLineNumBG = new SelectColorButton(this);
    flEditControls->addRow(QObject::tr("Line Number Background Color"),
                           mBtnLineNumBG);
    mBtnKeywordFG = new SelectColorButton(this);
    flEditControls->addRow(QObject::tr("Keyword Foreground Color"),
                           mBtnKeywordFG);
    mCBKeywordWeight = new SelectFontWeightCombo(this);
    flEditControls->addRow(QObject::tr("Keyword Font Weight"),
                           mCBKeywordWeight);
    mBtnClassFG = new SelectColorButton(this);
    flEditControls->addRow(QObject::tr("Class Foreground Color"),
                           mBtnClassFG);
    mCBClassWeight = new SelectFontWeightCombo(this);
    flEditControls->addRow(QObject::tr("Class Font Weight"),
                           mCBClassWeight);
    mBtnQuoteFG = new SelectColorButton(this);
    flEditControls->addRow(QObject::tr("Quote Foreground Color"),
                           mBtnQuoteFG);
    mCBQuoteWeight = new SelectFontWeightCombo(this);
    flEditControls->addRow(QObject::tr("Quote Font Weight"),
                           mCBQuoteWeight);
    mBtnCommentFG = new SelectColorButton(this);
    flEditControls->addRow(QObject::tr("Comment Foreground Color"),
                           mBtnCommentFG);
    mCBCommentWeight = new SelectFontWeightCombo(this);
    flEditControls->addRow(QObject::tr("Comment Font Weight"),
                           mCBCommentWeight);
    mBtnSymbolFG = new SelectColorButton(this);
    flEditControls->addRow(QObject::tr("Symbol Foreground Color"),
                           mBtnSymbolFG);
    mBtnSymbolBG = new SelectColorButton(this);
    flEditControls->addRow(QObject::tr("Symbol Background Color"),
                           mBtnSymbolBG);
    mCBSymbolWeight = new SelectFontWeightCombo(this);
    flEditControls->addRow(QObject::tr("Symbol Font Weight"),
                           mCBSymbolWeight);
    hboxEdit->addLayout(flEditControls);
    // CodeEditor to display Style
    mSampleEditor = new CodeEditor(this);
    QFont sampleFont("Monospace");
    QFontMetrics fm(sampleFont);
    mSampleEditor->setMinimumWidth(fm.width(QString(40, 'W')));
    // designate highlight, errors, and symbols
    mSampleEditor->setError(mSampleDocument, mErrLineNum, mErrSymbolsList);
    // End Controls
    hboxEdit->addWidget(mSampleEditor);
    vboxMain->addLayout(hboxEdit);

    // Default Controls
    QHBoxLayout *hboxDefaultControls = new QHBoxLayout();
    mBtnDefaultLight = new QPushButton(QObject::tr("Set to Default Light"),
                                       this);
    mBtnDefaultDark  = new QPushButton(QObject::tr("Set to Default Dark"),
                                       this);
    hboxDefaultControls->addStretch(1);
    hboxDefaultControls->addWidget(mBtnDefaultLight);
    hboxDefaultControls->addWidget(mBtnDefaultDark);
    hboxDefaultControls->addStretch(1);
    vboxMain->addLayout(hboxDefaultControls);
    vboxMain->addStretch(2);
    // dialog controls
    QDialogButtonBox *dBtnBox = new QDialogButtonBox(
        QDialogButtonBox::Cancel |
        QDialogButtonBox::Ok |
        QDialogButtonBox::Reset);
    vboxMain->addStretch(1);
    vboxMain->addWidget(dBtnBox);

    // setup values for style controls
    updateControls();
    updateStyle();

    connect(dBtnBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(dBtnBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(dBtnBox->button(QDialogButtonBox::Reset), SIGNAL(clicked()),
            this, SLOT(resetStyle()));
    connect(mBtnDefaultLight, SIGNAL(clicked()),
            this, SLOT(setStyleDefaultLight()));
    connect(mBtnDefaultDark, SIGNAL(clicked()),
            this, SLOT(setStyleDefaultDark()));
    connect(mBtnWidgetColorFG, SIGNAL(colorChanged(const QColor&)),
            this, SLOT(colorChangedWidgetFG(const QColor&)));
    connect(mBtnWidgetColorBG, SIGNAL(colorChanged(const QColor&)),
            this, SLOT(colorChangedWidgetBG(const QColor&)));
    connect(mBtnHighlightBG, SIGNAL(colorChanged(const QColor&)),
            this, SLOT(colorChangedHighlightBG(const QColor&)));
    connect(mBtnLineNumFG, SIGNAL(colorChanged(const QColor&)),
            this, SLOT(colorChangedLineNumFG(const QColor&)));
    connect(mBtnLineNumBG, SIGNAL(colorChanged(const QColor&)),
            this, SLOT(colorChangedLineNumBG(const QColor&)));
    connect(mBtnKeywordFG, SIGNAL(colorChanged(const QColor&)),
            this, SLOT(colorChangedKeywordFG(const QColor&)));
    connect(mCBKeywordWeight, SIGNAL(weightChanged(const QFont::Weight&)),
            this, SLOT(weightChangedKeyword(const QFont::Weight&)));
    connect(mBtnClassFG, SIGNAL(colorChanged(const QColor&)),
            this, SLOT(colorChangedClassFG(const QColor&)));
    connect(mCBClassWeight, SIGNAL(weightChanged(const QFont::Weight&)),
            this, SLOT(weightChangedClass(const QFont::Weight&)));
    connect(mBtnQuoteFG, SIGNAL(colorChanged(const QColor&)),
            this, SLOT(colorChangedQuoteFG(const QColor&)));
    connect(mCBQuoteWeight, SIGNAL(weightChanged(const QFont::Weight&)),
            this, SLOT(weightChangedQuote(const QFont::Weight&)));
    connect(mBtnCommentFG, SIGNAL(colorChanged(const QColor&)),
            this, SLOT(colorChangedCommentFG(const QColor&)));
    connect(mCBCommentWeight, SIGNAL(weightChanged(const QFont::Weight&)),
            this, SLOT(weightChangedComment(const QFont::Weight&)));
    connect(mBtnSymbolFG, SIGNAL(colorChanged(const QColor&)),
            this, SLOT(colorChangedSymbolFG(const QColor&)));
    connect(mBtnSymbolBG, SIGNAL(colorChanged(const QColor&)),
            this, SLOT(colorChangedSymbolBG(const QColor&)));
    connect(mCBSymbolWeight, SIGNAL(weightChanged(const QFont::Weight&)),
            this, SLOT(weightChangedSymbol(const QFont::Weight&)));
}

void StyleEditDialog::updateControls()
{
    mBtnWidgetColorFG->setColor(mStyleOutgoing.widgetFGColor);
    mBtnWidgetColorBG->setColor(mStyleOutgoing.widgetBGColor);
    mBtnHighlightBG->setColor(mStyleOutgoing.highlightBGColor);
    mBtnLineNumFG->setColor(mStyleOutgoing.lineNumFGColor);
    mBtnLineNumBG->setColor(mStyleOutgoing.lineNumBGColor);
    mBtnKeywordFG->setColor(mStyleOutgoing.keywordColor);
    mCBKeywordWeight->setWeight(mStyleOutgoing.keywordWeight);
    mBtnClassFG->setColor(mStyleOutgoing.classColor);
    mCBClassWeight->setWeight(mStyleOutgoing.classWeight);
    mBtnQuoteFG->setColor(mStyleOutgoing.quoteColor);
    mCBQuoteWeight->setWeight(mStyleOutgoing.quoteWeight);
    mBtnCommentFG->setColor(mStyleOutgoing.commentColor);
    mCBCommentWeight->setWeight(mStyleOutgoing.commentWeight);
    mBtnSymbolFG->setColor(mStyleOutgoing.symbolFGColor);
    mBtnSymbolBG->setColor(mStyleOutgoing.symbolBGColor);
    mCBSymbolWeight->setWeight(mStyleOutgoing.symbolWeight);
}

void StyleEditDialog::updateStyle()
{
    mBtnDefaultLight->setEnabled(mStyleOutgoing != defaultStyleLight);
    mBtnDefaultDark->setEnabled(mStyleOutgoing != defaultStyleDark);
    // set Editor Styling
    mSampleEditor->setStyle(mStyleOutgoing);
}

CodeEditorStyle StyleEditDialog::getStyle()
{
    return mStyleOutgoing;
}

void StyleEditDialog::resetStyle()
{
    mStyleOutgoing = mStyleIncoming;
    updateControls();
    updateStyle();
}

void StyleEditDialog::setStyleDefaultLight()
{
    mStyleOutgoing = defaultStyleLight;
    updateControls();
    updateStyle();
}

void StyleEditDialog::setStyleDefaultDark()
{
    mStyleOutgoing = defaultStyleDark;
    updateControls();
    updateStyle();
}

void StyleEditDialog::colorChangedWidgetFG(const QColor& newColor)
{
    mStyleOutgoing.widgetFGColor = newColor;
    updateStyle();
}

void StyleEditDialog::colorChangedWidgetBG(const QColor& newColor)
{
    mStyleOutgoing.widgetBGColor = newColor;
    updateStyle();
}

void StyleEditDialog::colorChangedHighlightBG(const QColor& newColor)
{
    mStyleOutgoing.highlightBGColor = newColor;
    updateStyle();
}

void StyleEditDialog::colorChangedLineNumFG(const QColor& newColor)
{
    mStyleOutgoing.lineNumFGColor = newColor;
    updateStyle();
}

void StyleEditDialog::colorChangedLineNumBG(const QColor& newColor)
{
    mStyleOutgoing.lineNumBGColor = newColor;
    updateStyle();
}

void StyleEditDialog::colorChangedKeywordFG(const QColor& newColor)
{
    mStyleOutgoing.keywordColor = newColor;
    updateStyle();
}

void StyleEditDialog::weightChangedKeyword(const QFont::Weight& newWeight)
{
    mStyleOutgoing.keywordWeight = newWeight;
    updateStyle();
}

void StyleEditDialog::colorChangedClassFG(const QColor& newColor)
{
    mStyleOutgoing.classColor = newColor;
    updateStyle();
}

void StyleEditDialog::weightChangedClass(const QFont::Weight& newWeight)
{
    mStyleOutgoing.classWeight = newWeight;
    updateStyle();
}

void StyleEditDialog::colorChangedQuoteFG(const QColor& newColor)
{
    mStyleOutgoing.quoteColor = newColor;
    updateStyle();
}

void StyleEditDialog::weightChangedQuote(const QFont::Weight& newWeight)
{
    mStyleOutgoing.quoteWeight = newWeight;
    updateStyle();
}

void StyleEditDialog::colorChangedCommentFG(const QColor& newColor)
{
    mStyleOutgoing.commentColor = newColor;
    updateStyle();
}

void StyleEditDialog::weightChangedComment(const QFont::Weight& newWeight)
{
    mStyleOutgoing.commentWeight = newWeight;
    updateStyle();
}

void StyleEditDialog::colorChangedSymbolFG(const QColor& newColor)
{
    mStyleOutgoing.symbolFGColor = newColor;
    updateStyle();
}

void StyleEditDialog::colorChangedSymbolBG(const QColor& newColor)
{
    mStyleOutgoing.symbolBGColor = newColor;
    updateStyle();
}

void StyleEditDialog::weightChangedSymbol(const QFont::Weight& newWeight)
{
    mStyleOutgoing.symbolWeight = newWeight;
    updateStyle();
}
