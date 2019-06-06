#ifndef CODEEDITORSTYLE_H
#define CODEEDITORSTYLE_H

#include <QString>
#include <QColor>
#include <QFont>

class CodeEditorStyle {
public:
    CodeEditorStyle(
        const QColor& CtrlFGColor, const QColor& CtrlBGColor,
        const QColor& HiLiBGColor,
        const QColor& LnNumFGColor, const QColor& LnNumBGColor,
        const QColor& KeyWdFGColor, const QFont::Weight& KeyWdWeight,
        const QColor& ClsFGColor, const QFont::Weight& ClsWeight,
        const QColor& QteFGColor, const QFont::Weight& QteWeight,
        const QColor& CmtFGColor, const QFont::Weight& CmtWeight,
        const QColor& SymbFGColor, const QColor& SymbBGColor,
        const QFont::Weight& SymbWeight ) :
    widgetFGColor( CtrlFGColor ),
    widgetBGColor( CtrlBGColor ),
    highlightBGColor( HiLiBGColor ),
    lineNumFGColor( LnNumFGColor ),
    lineNumBGColor( LnNumBGColor ),
    keywordColor( KeyWdFGColor ),
    keywordWeight( KeyWdWeight ),
    classColor( ClsFGColor ),
    classWeight( ClsWeight ),
    quoteColor( QteFGColor ),
    quoteWeight( QteWeight ),
    commentColor( CmtFGColor ),
    commentWeight( CmtWeight ),
    symbolFGColor( SymbFGColor ),
    symbolBGColor( SymbBGColor ),
    symbolWeight( SymbWeight )
    {}

    // delete empty constructor
    CodeEditorStyle() = delete;
    // default copy constructor and copy operator=
    CodeEditorStyle( const CodeEditorStyle& rhs ) = default;
    CodeEditorStyle& operator=( const CodeEditorStyle& rhs ) = default;
    // default move constructor and move operator=
    CodeEditorStyle( CodeEditorStyle&& rhs ) = default;
    CodeEditorStyle& operator=( CodeEditorStyle&& rhs ) = default;
    // default destructor
    ~CodeEditorStyle() = default;

public:
    QColor          widgetFGColor;
    QColor          widgetBGColor;
    QColor          highlightBGColor;
    QColor          lineNumFGColor;
    QColor          lineNumBGColor;
    QColor          keywordColor;
    QFont::Weight   keywordWeight;
    QColor          classColor;
    QFont::Weight   classWeight;
    QColor          quoteColor;
    QFont::Weight   quoteWeight;
    QColor          commentColor;
    QFont::Weight   commentWeight;
    QColor          symbolFGColor;
    QColor          symbolBGColor;
    QFont::Weight   symbolWeight;
};

static const CodeEditorStyle defaultStyle({
/* editor FG/BG */          Qt::black, QColor( 240, 240, 240 ),
/* highlight BG */          QColor( 255, 220, 220 ),
/* line number FG/BG */     Qt::black, QColor( 240, 240, 240 ),
/* keyword FG/Weight */     Qt::darkBlue, QFont::Bold,
/* class FG/Weight */       Qt::darkMagenta, QFont::Bold,
/* quote FG/Weight */       Qt::darkGreen, QFont::Normal,
/* comment FG/Weight */     Qt::gray, QFont::Normal,
/* Symbol FG/BG/Weight */   Qt::red, QColor( 220, 220, 255 ), QFont::Normal });

#endif /* CODEEDITORSTYLE_H */
