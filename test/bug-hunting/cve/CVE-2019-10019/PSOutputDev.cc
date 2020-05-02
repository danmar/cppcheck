//========================================================================
//
// PSOutputDev.cc
//
// Copyright 1996-2013 Glyph & Cog, LLC
//
//========================================================================

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <signal.h>
#include <math.h>
#include "gmempp.h"
#include "GString.h"
#include "GList.h"
#include "GHash.h"
#include "config.h"
#include "GlobalParams.h"
#include "Object.h"
#include "Error.h"
#include "Function.h"
#include "Gfx.h"
#include "GfxState.h"
#include "GfxFont.h"
#include "UnicodeMap.h"
#include "FoFiType1C.h"
#include "FoFiTrueType.h"
#include "Catalog.h"
#include "Page.h"
#include "Stream.h"
#include "Annot.h"
#include "PDFDoc.h"
#include "XRef.h"
#include "PreScanOutputDev.h"
#include "CharCodeToUnicode.h"
#include "Form.h"
#include "TextString.h"
#if HAVE_SPLASH
#  include "Splash.h"
#  include "SplashBitmap.h"
#  include "SplashOutputDev.h"
#endif
#include "PSOutputDev.h"

// the MSVC math.h doesn't define this
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//------------------------------------------------------------------------
// PostScript prolog and setup
//------------------------------------------------------------------------

// The '~' escapes mark prolog code that is emitted only in certain
// levels:
//
//   ~[123][ngs]
//      ^   ^----- n=psLevel_, g=psLevel_Gray, s=psLevel_Sep
//      +----- 1=psLevel1__, 2=psLevel2__, 3=psLevel3__

static const char *prolog[] = {
  "/xpdf 75 dict def xpdf begin",
  "% PDF special state",
  "/pdfDictSize 15 def",
  "~1ns",
  "/pdfStates 64 array def",
  "  0 1 63 {",
  "    pdfStates exch pdfDictSize dict",
  "    dup /pdfStateIdx 3 index put",
  "    put",
  "  } for",
  "~123ngs",
  "/pdfSetup {",
  "  /pdfDuplex exch def",
  "  /setpagedevice where {",
  "    pop 2 dict begin",
  "      /Policies 1 dict dup begin /PageSize 6 def end def",
  "      pdfDuplex { /Duplex true def } if",
  "    currentdict end setpagedevice",
  "  } if",
  "  /pdfPageW 0 def",
  "  /pdfPageH 0 def",
  "} def",
  "/pdfSetupPaper {",
  "  2 copy pdfPageH ne exch pdfPageW ne or {",
  "    /pdfPageH exch def",
  "    /pdfPageW exch def",
  "    /setpagedevice where {",
  "      pop 3 dict begin",
  "        /PageSize [pdfPageW pdfPageH] def",
  "        pdfDuplex { /Duplex true def } if",
  "        /ImagingBBox null def",
  "      currentdict end setpagedevice",
  "    } if",
  "  } {",
  "    pop pop",
  "  } ifelse",
  "} def",
  "~1ns",
  "/pdfOpNames [",
  "  /pdfFill /pdfStroke /pdfLastFill /pdfLastStroke",
  "  /pdfTextMat /pdfFontSize /pdfCharSpacing /pdfTextRender",
  "  /pdfTextRise /pdfWordSpacing /pdfHorizScaling /pdfTextClipPath",
  "] def",
  "~123ngs",
  "/pdfStartPage {",
  "~1ns",
  "  pdfStates 0 get begin",
  "~23ngs",
  "  pdfDictSize dict begin",
  "~23n",
  "  /pdfFillCS [] def",
  "  /pdfFillXform {} def",
  "  /pdfStrokeCS [] def",
  "  /pdfStrokeXform {} def",
  "~1n",
  "  /pdfFill 0 def",
  "  /pdfStroke 0 def",
  "~1s",
  "  /pdfFill [0 0 0 1] def",
  "  /pdfStroke [0 0 0 1] def",
  "~23g",
  "  /pdfFill 0 def",
  "  /pdfStroke 0 def",
  "~23ns",
  "  /pdfFill [0] def",
  "  /pdfStroke [0] def",
  "  /pdfFillOP false def",
  "  /pdfStrokeOP false def",
  "~123ngs",
  "  /pdfLastFill false def",
  "  /pdfLastStroke false def",
  "  /pdfTextMat [1 0 0 1 0 0] def",
  "  /pdfFontSize 0 def",
  "  /pdfCharSpacing 0 def",
  "  /pdfTextRender 0 def",
  "  /pdfTextRise 0 def",
  "  /pdfWordSpacing 0 def",
  "  /pdfHorizScaling 1 def",
  "  /pdfTextClipPath [] def",
  "} def",
  "/pdfEndPage { end } def",
  "~23s",
  "% separation convention operators",
  "/findcmykcustomcolor where {",
  "  pop",
  "}{",
  "  /findcmykcustomcolor { 5 array astore } def",
  "} ifelse",
  "/setcustomcolor where {",
  "  pop",
  "}{",
  "  /setcustomcolor {",
  "    exch",
  "    [ exch /Separation exch dup 4 get exch /DeviceCMYK exch",
  "      0 4 getinterval cvx",
  "      [ exch /dup load exch { mul exch dup } /forall load",
  "        /pop load dup ] cvx",
  "    ] setcolorspace setcolor",
  "  } def",
  "} ifelse",
  "/customcolorimage where {",
  "  pop",
  "}{",
  "  /customcolorimage {",
  "    gsave",
  "    [ exch /Separation exch dup 4 get exch /DeviceCMYK exch",
  "      0 4 getinterval",
  "      [ exch /dup load exch { mul exch dup } /forall load",
  "        /pop load dup ] cvx",
  "    ] setcolorspace",
  "    10 dict begin",
  "      /ImageType 1 def",
  "      /DataSource exch def",
  "      /ImageMatrix exch def",
  "      /BitsPerComponent exch def",
  "      /Height exch def",
  "      /Width exch def",
  "      /Decode [1 0] def",
  "    currentdict end",
  "    image",
  "    grestore",
  "  } def",
  "} ifelse",
  "~123ngs",
  "% PDF color state",
  "~1n",
  "/g { dup /pdfFill exch def setgray",
  "     /pdfLastFill true def /pdfLastStroke false def } def",
  "/G { dup /pdfStroke exch def setgray",
  "     /pdfLastStroke true def /pdfLastFill false def } def",
  "/fCol {",
  "  pdfLastFill not {",
  "    pdfFill setgray",
  "    /pdfLastFill true def /pdfLastStroke false def",
  "  } if",
  "} def",
  "/sCol {",
  "  pdfLastStroke not {",
  "    pdfStroke setgray",
  "    /pdfLastStroke true def /pdfLastFill false def",
  "  } if",
  "} def",
  "~1s",
  "/k { 4 copy 4 array astore /pdfFill exch def setcmykcolor",
  "     /pdfLastFill true def /pdfLastStroke false def } def",
  "/K { 4 copy 4 array astore /pdfStroke exch def setcmykcolor",
  "     /pdfLastStroke true def /pdfLastFill false def } def",
  "/fCol {",
  "  pdfLastFill not {",
  "    pdfFill aload pop setcmykcolor",
  "    /pdfLastFill true def /pdfLastStroke false def",
  "  } if",
  "} def",
  "/sCol {",
  "  pdfLastStroke not {",
  "    pdfStroke aload pop setcmykcolor",
  "    /pdfLastStroke true def /pdfLastFill false def",
  "  } if",
  "} def",
  "~23n",
  "/cs { /pdfFillXform exch def dup /pdfFillCS exch def",
  "      setcolorspace } def",
  "/CS { /pdfStrokeXform exch def dup /pdfStrokeCS exch def",
  "      setcolorspace } def",
  "/sc { pdfLastFill not {",
  "        pdfFillCS setcolorspace pdfFillOP setoverprint",
  "      } if",
  "      dup /pdfFill exch def aload pop pdfFillXform setcolor",
  "      /pdfLastFill true def /pdfLastStroke false def } def",
  "/SC { pdfLastStroke not {",
  "        pdfStrokeCS setcolorspace pdfStrokeOP setoverprint",
  "      } if",
  "      dup /pdfStroke exch def aload pop pdfStrokeXform setcolor",
  "      /pdfLastStroke true def /pdfLastFill false def } def",
  "/op { /pdfFillOP exch def",
  "      pdfLastFill { pdfFillOP setoverprint } if } def",
  "/OP { /pdfStrokeOP exch def",
  "      pdfLastStroke { pdfStrokeOP setoverprint } if } def",
  "/fCol {",
  "  pdfLastFill not {",
  "    pdfFillCS setcolorspace",
  "    pdfFill aload pop pdfFillXform setcolor",
  "    pdfFillOP setoverprint",
  "    /pdfLastFill true def /pdfLastStroke false def",
  "  } if",
  "} def",
  "/sCol {",
  "  pdfLastStroke not {",
  "    pdfStrokeCS setcolorspace",
  "    pdfStroke aload pop pdfStrokeXform setcolor",
  "    pdfStrokeOP setoverprint",
  "    /pdfLastStroke true def /pdfLastFill false def",
  "  } if",
  "} def",
  "~23g",
  "/g { dup /pdfFill exch def setgray",
  "     /pdfLastFill true def /pdfLastStroke false def } def",
  "/G { dup /pdfStroke exch def setgray",
  "     /pdfLastStroke true def /pdfLastFill false def } def",
  "/fCol {",
  "  pdfLastFill not {",
  "    pdfFill setgray",
  "    /pdfLastFill true def /pdfLastStroke false def",
  "  } if",
  "} def",
  "/sCol {",
  "  pdfLastStroke not {",
  "    pdfStroke setgray",
  "    /pdfLastStroke true def /pdfLastFill false def",
  "  } if",
  "} def",
  "~23s",
  "/k { 4 copy 4 array astore /pdfFill exch def setcmykcolor",
  "     pdfFillOP setoverprint",
  "     /pdfLastFill true def /pdfLastStroke false def } def",
  "/K { 4 copy 4 array astore /pdfStroke exch def setcmykcolor",
  "     pdfStrokeOP setoverprint",
  "     /pdfLastStroke true def /pdfLastFill false def } def",
  "/ck { 6 copy 6 array astore /pdfFill exch def",
  "      findcmykcustomcolor exch setcustomcolor",
  "      pdfFillOP setoverprint",
  "      /pdfLastFill true def /pdfLastStroke false def } def",
  "/CK { 6 copy 6 array astore /pdfStroke exch def",
  "      findcmykcustomcolor exch setcustomcolor",
  "      pdfStrokeOP setoverprint",
  "      /pdfLastStroke true def /pdfLastFill false def } def",
  "/op { /pdfFillOP exch def",
  "      pdfLastFill { pdfFillOP setoverprint } if } def",
  "/OP { /pdfStrokeOP exch def",
  "      pdfLastStroke { pdfStrokeOP setoverprint } if } def",
  "/fCol {",
  "  pdfLastFill not {",
  "    pdfFill aload length 4 eq {",
  "      setcmykcolor",
  "    }{",
  "      findcmykcustomcolor exch setcustomcolor",
  "    } ifelse",
  "    pdfFillOP setoverprint",
  "    /pdfLastFill true def /pdfLastStroke false def",
  "  } if",
  "} def",
  "/sCol {",
  "  pdfLastStroke not {",
  "    pdfStroke aload length 4 eq {",
  "      setcmykcolor",
  "    }{",
  "      findcmykcustomcolor exch setcustomcolor",
  "    } ifelse",
  "    pdfStrokeOP setoverprint",
  "    /pdfLastStroke true def /pdfLastFill false def",
  "  } if",
  "} def",
  "~3ns",
  "/opm {",
  "  /setoverprintmode where { pop setoverprintmode } { pop } ifelse",
  "} def",
  "~123ngs",
  "% build a font",
  "/pdfMakeFont {",
  "  4 3 roll findfont",
  "  4 2 roll matrix scale makefont",
  "  dup length dict begin",
  "    { 1 index /FID ne { def } { pop pop } ifelse } forall",
  "    /Encoding exch def",
  "    currentdict",
  "  end",
  "  definefont pop",
  "} def",
  "/pdfMakeFont16 {",
  "  exch findfont",
  "  dup length dict begin",
  "    { 1 index /FID ne { def } { pop pop } ifelse } forall",
  "    /WMode exch def",
  "    currentdict",
  "  end",
  "  definefont pop",
  "} def",
  "~3ngs",
  "/pdfMakeFont16L3 {",
  "  1 index /CIDFont resourcestatus {",
  "    pop pop 1 index /CIDFont findresource /CIDFontType known",
  "  } {",
  "    false",
  "  } ifelse",
  "  {",
  "    0 eq { /Identity-H } { /Identity-V } ifelse",
  "    exch 1 array astore composefont pop",
  "  } {",
  "    pdfMakeFont16",
  "  } ifelse",
  "} def",
  "~123ngs",
  "% graphics state operators",
  "~1ns",
  "/q {",
  "  gsave",
  "  pdfOpNames length 1 sub -1 0 { pdfOpNames exch get load } for",
  "  pdfStates pdfStateIdx 1 add get begin",
  "  pdfOpNames { exch def } forall",
  "} def",
  "/Q { end grestore } def",
  "~23ngs",
  "/q { gsave pdfDictSize dict begin } def",
  "/Q {",
  "  end grestore",
  "} def",
  "~123ngs",
  "/cm { concat } def",
  "/d { setdash } def",
  "/i { setflat } def",
  "/j { setlinejoin } def",
  "/J { setlinecap } def",
  "/M { setmiterlimit } def",
  "/w { setlinewidth } def",
  "% path segment operators",
  "/m { moveto } def",
  "/l { lineto } def",
  "/c { curveto } def",
  "/re { 4 2 roll moveto 1 index 0 rlineto 0 exch rlineto",
  "      neg 0 rlineto closepath } def",
  "/h { closepath } def",
  "% path painting operators",
  "/S { sCol stroke } def",
  "/Sf { fCol stroke } def",
  "/f { fCol fill } def",
  "/f* { fCol eofill } def",
  "% clipping operators",
  "/W { clip newpath } def",
  "/W* { eoclip newpath } def",
  "/Ws { strokepath clip newpath } def",
  "% text state operators",
  "/Tc { /pdfCharSpacing exch def } def",
  "/Tf { dup /pdfFontSize exch def",
  "      dup pdfHorizScaling mul exch matrix scale",
  "      pdfTextMat matrix concatmatrix dup 4 0 put dup 5 0 put",
  "      exch findfont exch makefont setfont } def",
  "/Tr { /pdfTextRender exch def } def",
  "/Ts { /pdfTextRise exch def } def",
  "/Tw { /pdfWordSpacing exch def } def",
  "/Tz { /pdfHorizScaling exch def } def",
  "% text positioning operators",
  "/Td { pdfTextMat transform moveto } def",
  "/Tm { /pdfTextMat exch def } def",
  "% text string operators",
  "/xyshow where {",
  "  pop",
  "  /xyshow2 {",
  "    dup length array",
  "    0 2 2 index length 1 sub {",
  "      2 index 1 index 2 copy get 3 1 roll 1 add get",
  "      pdfTextMat dtransform",
  "      4 2 roll 2 copy 6 5 roll put 1 add 3 1 roll dup 4 2 roll put",
  "    } for",
  "    exch pop",
  "    xyshow",
  "  } def",
  "}{",
  "  /xyshow2 {",
  "    currentfont /FontType get 0 eq {",
  "      0 2 3 index length 1 sub {",
  "        currentpoint 4 index 3 index 2 getinterval show moveto",
  "        2 copy get 2 index 3 2 roll 1 add get",
  "        pdfTextMat dtransform rmoveto",
  "      } for",
  "    } {",
  "      0 1 3 index length 1 sub {",
  "        currentpoint 4 index 3 index 1 getinterval show moveto",
  "        2 copy 2 mul get 2 index 3 2 roll 2 mul 1 add get",
  "        pdfTextMat dtransform rmoveto",
  "      } for",
  "    } ifelse",
  "    pop pop",
  "  } def",
  "} ifelse",
  "/cshow where {",
  "  pop",
  "  /xycp {", // xycharpath
  "    0 3 2 roll",
  "    {",
  "      pop pop currentpoint 3 2 roll",
  "      1 string dup 0 4 3 roll put false charpath moveto",
  "      2 copy get 2 index 2 index 1 add get",
  "      pdfTextMat dtransform rmoveto",
  "      2 add",
  "    } exch cshow",
  "    pop pop",
  "  } def",
  "}{",
  "  /xycp {", // xycharpath
  "    currentfont /FontType get 0 eq {",
  "      0 2 3 index length 1 sub {",
  "        currentpoint 4 index 3 index 2 getinterval false charpath moveto",
  "        2 copy get 2 index 3 2 roll 1 add get",
  "        pdfTextMat dtransform rmoveto",
  "      } for",
  "    } {",
  "      0 1 3 index length 1 sub {",
  "        currentpoint 4 index 3 index 1 getinterval false charpath moveto",
  "        2 copy 2 mul get 2 index 3 2 roll 2 mul 1 add get",
  "        pdfTextMat dtransform rmoveto",
  "      } for",
  "    } ifelse",
  "    pop pop",
  "  } def",
  "} ifelse",
  "/Tj {",
  "  fCol",  // because stringwidth has to draw Type 3 chars
  "  0 pdfTextRise pdfTextMat dtransform rmoveto",
  "  currentpoint 4 2 roll",
  "  pdfTextRender 1 and 0 eq {",
  "    2 copy xyshow2",
  "  } if",
  "  pdfTextRender 3 and dup 1 eq exch 2 eq or {",
  "    3 index 3 index moveto",
  "    2 copy",
  "    currentfont /FontType get 3 eq { fCol } { sCol } ifelse",
  "    xycp currentpoint stroke moveto",
  "  } if",
  "  pdfTextRender 4 and 0 ne {",
  "    4 2 roll moveto xycp",
  "    /pdfTextClipPath [ pdfTextClipPath aload pop",
  "      {/moveto cvx}",
  "      {/lineto cvx}",
  "      {/curveto cvx}",
  "      {/closepath cvx}",
  "    pathforall ] def",
  "    currentpoint newpath moveto",
  "  } {",
  "    pop pop pop pop",
  "  } ifelse",
  "  0 pdfTextRise neg pdfTextMat dtransform rmoveto",
  "} def",
  "/Tj3 {",
  "  pdfTextRender 3 and 3 ne {"
  "    fCol",  // because stringwidth has to draw Type 3 chars
  "    0 pdfTextRise pdfTextMat dtransform rmoveto",
  "    xyshow2",
  "    0 pdfTextRise neg pdfTextMat dtransform rmoveto",
  "  } {",
  "    pop pop",
  "  } ifelse",
  "} def",
  "/TJm { 0.001 mul pdfFontSize mul pdfHorizScaling mul neg 0",
  "       pdfTextMat dtransform rmoveto } def",
  "/TJmV { 0.001 mul pdfFontSize mul neg 0 exch",
  "        pdfTextMat dtransform rmoveto } def",
  "/Tclip { pdfTextClipPath cvx exec clip newpath",
  "         /pdfTextClipPath [] def } def",
  "~1ns",
  "% Level 1 image operators",
  "~1n",
  "/pdfIm1 {",
  "  /pdfImBuf1 4 index string def",
  "  { currentfile pdfImBuf1 readhexstring pop } image",
  "} def",
  "~1s",
  "/pdfIm1Sep {",
  "  /pdfImBuf1 4 index string def",
  "  /pdfImBuf2 4 index string def",
  "  /pdfImBuf3 4 index string def",
  "  /pdfImBuf4 4 index string def",
  "  { currentfile pdfImBuf1 readhexstring pop }",
  "  { currentfile pdfImBuf2 readhexstring pop }",
  "  { currentfile pdfImBuf3 readhexstring pop }",
  "  { currentfile pdfImBuf4 readhexstring pop }",
  "  true 4 colorimage",
  "} def",
  "~1ns",
  "/pdfImM1 {",
  "  fCol /pdfImBuf1 4 index 7 add 8 idiv string def",
  "  { currentfile pdfImBuf1 readhexstring pop } imagemask",
  "} def",
  "/pdfImStr {",
  "  2 copy exch length lt {",
  "    2 copy get exch 1 add exch",
  "  } {",
  "    ()",
  "  } ifelse",
  "} def",
  "/pdfImM1a {",
  "  { pdfImStr } imagemask",
  "  pop pop",
  "} def",
  "~23ngs",
  "% Level 2/3 image operators",
  "/pdfImBuf 100 string def",
  "/pdfImStr {",
  "  2 copy exch length lt {",
  "    2 copy get exch 1 add exch",
  "  } {",
  "    ()",
  "  } ifelse",
  "} def",
  "/skipEOD {",
  "  { currentfile pdfImBuf readline",
  "    not { pop exit } if",
  "    (%-EOD-) eq { exit } if } loop",
  "} def",
  "/pdfIm { image skipEOD } def",
  "~3ngs",
  "/pdfMask {",
  "  /ReusableStreamDecode filter",
  "  skipEOD",
  "  /maskStream exch def",
  "} def",
  "/pdfMaskEnd { maskStream closefile } def",
  "/pdfMaskInit {",
  "  /maskArray exch def",
  "  /maskIdx 0 def",
  "} def",
  "/pdfMaskSrc {",
  "  maskIdx maskArray length lt {",
  "    maskArray maskIdx get",
  "    /maskIdx maskIdx 1 add def",
  "  } {",
  "    ()",
  "  } ifelse",
  "} def",
  "~23s",
  "/pdfImSep {",
  "  findcmykcustomcolor exch",
  "  dup /Width get /pdfImBuf1 exch string def",
  "  dup /Decode get aload pop 1 index sub /pdfImDecodeRange exch def",
  "  /pdfImDecodeLow exch def",
  "  begin Width Height BitsPerComponent ImageMatrix DataSource end",
  "  /pdfImData exch def",
  "  { pdfImData pdfImBuf1 readstring pop",
  "    0 1 2 index length 1 sub {",
  "      1 index exch 2 copy get",
  "      pdfImDecodeRange mul 255 div pdfImDecodeLow add round cvi",
  "      255 exch sub put",
  "    } for }",
  "  6 5 roll customcolorimage",
  "  skipEOD",
  "} def",
  "~23ngs",
  "/pdfImM { fCol imagemask skipEOD } def",
  "/pr {",
  "  4 2 roll exch 5 index div exch 4 index div moveto",
  "  exch 3 index div dup 0 rlineto",
  "  exch 2 index div 0 exch rlineto",
  "  neg 0 rlineto",
  "  closepath",
  "} def",
  "/pdfImClip { gsave clip } def",
  "/pdfImClipEnd { grestore } def",
  "~23ns",
  "% shading operators",
  "/colordelta {",
  "  false 0 1 3 index length 1 sub {",
  "    dup 4 index exch get 3 index 3 2 roll get sub abs 0.004 gt {",
  "      pop true",
  "    } if",
  "  } for",
  "  exch pop exch pop",
  "} def",
  "/funcCol { func n array astore } def",
  "/funcSH {",
  "  dup 0 eq {",
  "    true",
  "  } {",
  "    dup 6 eq {",
  "      false",
  "    } {",
  "      4 index 4 index funcCol dup",
  "      6 index 4 index funcCol dup",
  "      3 1 roll colordelta 3 1 roll",
  "      5 index 5 index funcCol dup",
  "      3 1 roll colordelta 3 1 roll",
  "      6 index 8 index funcCol dup",
  "      3 1 roll colordelta 3 1 roll",
  "      colordelta or or or",
  "    } ifelse",
  "  } ifelse",
  "  {",
  "    1 add",
  "    4 index 3 index add 0.5 mul exch 4 index 3 index add 0.5 mul exch",
  "    6 index 6 index 4 index 4 index 4 index funcSH",
  "    2 index 6 index 6 index 4 index 4 index funcSH",
  "    6 index 2 index 4 index 6 index 4 index funcSH",
  "    5 3 roll 3 2 roll funcSH pop pop",
  "  } {",
  "    pop 3 index 2 index add 0.5 mul 3 index  2 index add 0.5 mul",
  "~23n",
  "    funcCol sc",
  "~23s",
  "    funcCol aload pop k",
  "~23ns",
  "    dup 4 index exch mat transform m",
  "    3 index 3 index mat transform l",
  "    1 index 3 index mat transform l",
  "    mat transform l pop pop h f*",
  "  } ifelse",
  "} def",
  "/axialCol {",
  "  dup 0 lt {",
  "    pop t0",
  "  } {",
  "    dup 1 gt {",
  "      pop t1",
  "    } {",
  "      dt mul t0 add",
  "    } ifelse",
  "  } ifelse",
  "  func n array astore",
  "} def",
  "/axialSH {",
  "  dup 2 lt {",
  "    true",
  "  } {",
  "    dup 8 eq {",
  "      false",
  "    } {",
  "      2 index axialCol 2 index axialCol colordelta",
  "    } ifelse",
  "  } ifelse",
  "  {",
  "    1 add 3 1 roll 2 copy add 0.5 mul",
  "    dup 4 3 roll exch 4 index axialSH",
  "    exch 3 2 roll axialSH",
  "  } {",
  "    pop 2 copy add 0.5 mul",
  "~23n",
  "    axialCol sc",
  "~23s",
  "    axialCol aload pop k",
  "~23ns",
  "    exch dup dx mul x0 add exch dy mul y0 add",
  "    3 2 roll dup dx mul x0 add exch dy mul y0 add",
  "    dx abs dy abs ge {",
  "      2 copy yMin sub dy mul dx div add yMin m",
  "      yMax sub dy mul dx div add yMax l",
  "      2 copy yMax sub dy mul dx div add yMax l",
  "      yMin sub dy mul dx div add yMin l",
  "      h f*",
  "    } {",
  "      exch 2 copy xMin sub dx mul dy div add xMin exch m",
  "      xMax sub dx mul dy div add xMax exch l",
  "      exch 2 copy xMax sub dx mul dy div add xMax exch l",
  "      xMin sub dx mul dy div add xMin exch l",
  "      h f*",
  "    } ifelse",
  "  } ifelse",
  "} def",
  "/radialCol {",
  "  dup t0 lt {",
  "    pop t0",
  "  } {",
  "    dup t1 gt {",
  "      pop t1",
  "    } if",
  "  } ifelse",
  "  func n array astore",
  "} def",
  "/radialSH {",
  "  dup 0 eq {",
  "    true",
  "  } {",
  "    dup 8 eq {",
  "      false",
  "    } {",
  "      2 index dt mul t0 add radialCol",
  "      2 index dt mul t0 add radialCol colordelta",
  "    } ifelse",
  "  } ifelse",
  "  {",
  "    1 add 3 1 roll 2 copy add 0.5 mul",
  "    dup 4 3 roll exch 4 index radialSH",
  "    exch 3 2 roll radialSH",
  "  } {",
  "    pop 2 copy add 0.5 mul dt mul t0 add",
  "~23n",
  "    radialCol sc",
  "~23s",
  "    radialCol aload pop k",
  "~23ns",
  "    encl {",
  "      exch dup dx mul x0 add exch dup dy mul y0 add exch dr mul r0 add",
  "      0 360 arc h",
  "      dup dx mul x0 add exch dup dy mul y0 add exch dr mul r0 add",
  "      360 0 arcn h f",
  "    } {",
  "      2 copy",
  "      dup dx mul x0 add exch dup dy mul y0 add exch dr mul r0 add",
  "      a1 a2 arcn",
  "      dup dx mul x0 add exch dup dy mul y0 add exch dr mul r0 add",
  "      a2 a1 arcn h",
  "      dup dx mul x0 add exch dup dy mul y0 add exch dr mul r0 add",
  "      a1 a2 arc",
  "      dup dx mul x0 add exch dup dy mul y0 add exch dr mul r0 add",
  "      a2 a1 arc h f",
  "    } ifelse",
  "  } ifelse",
  "} def",
  "~123ngs",
  "end",
  NULL
};

static const char *minLineWidthProlog[] = {
  "/pdfDist { dup dtransform dup mul exch dup mul add 0.5 mul sqrt } def",
  "/pdfIDist { dup idtransform dup mul exch dup mul add 0.5 mul sqrt } def",
  "/pdfMinLineDist pdfMinLineWidth pdfDist def",
  "/setlinewidth {",
  "  dup pdfDist pdfMinLineDist lt {",
  "    pop pdfMinLineDist pdfIDist",
  "  } if",
  "  setlinewidth",
  "} bind def",
  NULL
};

static const char *cmapProlog[] = {
  "/CIDInit /ProcSet findresource begin",
  "10 dict begin",
  "  begincmap",
  "  /CMapType 1 def",
  "  /CMapName /Identity-H def",
  "  /CIDSystemInfo 3 dict dup begin",
  "    /Registry (Adobe) def",
  "    /Ordering (Identity) def",
  "    /Supplement 0 def",
  "  end def",
  "  1 begincodespacerange",
  "    <0000> <ffff>",
  "  endcodespacerange",
  "  0 usefont",
  "  1 begincidrange",
  "    <0000> <ffff> 0",
  "  endcidrange",
  "  endcmap",
  "  currentdict CMapName exch /CMap defineresource pop",
  "end",
  "10 dict begin",
  "  begincmap",
  "  /CMapType 1 def",
  "  /CMapName /Identity-V def",
  "  /CIDSystemInfo 3 dict dup begin",
  "    /Registry (Adobe) def",
  "    /Ordering (Identity) def",
  "    /Supplement 0 def",
  "  end def",
  "  /WMode 1 def",
  "  1 begincodespacerange",
  "    <0000> <ffff>",
  "  endcodespacerange",
  "  0 usefont",
  "  1 begincidrange",
  "    <0000> <ffff> 0",
  "  endcidrange",
  "  endcmap",
  "  currentdict CMapName exch /CMap defineresource pop",
  "end",
  "end",
  NULL
};

//------------------------------------------------------------------------
// Fonts
//------------------------------------------------------------------------

struct PSSubstFont {
  const char *psName;		// PostScript name
  double mWidth;		// width of 'm' character
};

// NB: must be in same order as base14SubstFonts in GfxFont.cc
static PSSubstFont psBase14SubstFonts[14] = {
  {"Courier",               0.600},
  {"Courier-Oblique",       0.600},
  {"Courier-Bold",          0.600},
  {"Courier-BoldOblique",   0.600},
  {"Helvetica",             0.833},
  {"Helvetica-Oblique",     0.833},
  {"Helvetica-Bold",        0.889},
  {"Helvetica-BoldOblique", 0.889},
  {"Times-Roman",           0.788},
  {"Times-Italic",          0.722},
  {"Times-Bold",            0.833},
  {"Times-BoldItalic",      0.778},
  // the last two are never used for substitution
  {"Symbol",                0},
  {"ZapfDingbats",          0}
};

class PSFontInfo {
public:

  PSFontInfo(Ref fontIDA)
    { fontID = fontIDA; ff = NULL; }

  Ref fontID;
  PSFontFileInfo *ff;		// pointer to font file info; NULL indicates
				//   font mapping failed
};

enum PSFontFileLocation {
  psFontFileResident,
  psFontFileEmbedded,
  psFontFileExternal
};

class PSFontFileInfo {
public:

  PSFontFileInfo(GString *psNameA, GfxFontType typeA,
		 PSFontFileLocation locA);
  ~PSFontFileInfo();

  GString *psName;		// name under which font is defined
  GfxFontType type;		// font type
  PSFontFileLocation loc;	// font location
  Ref embFontID;		// object ID for the embedded font file
				//   (for all embedded fonts)
  GString *extFileName;		// external font file path
				//   (for all external fonts)
  GString *encoding;		// encoding name (for resident CID fonts)
  int *codeToGID;		// mapping from code/CID to GID
				//   (for TrueType, OpenType-TrueType, and
				//   CID OpenType-CFF fonts)
  int codeToGIDLen;		// length of codeToGID array
};

PSFontFileInfo::PSFontFileInfo(GString *psNameA, GfxFontType typeA,
			       PSFontFileLocation locA) {
  psName = psNameA;
  type = typeA;
  loc = locA;
  embFontID.num = embFontID.gen = -1;
  extFileName = NULL;
  encoding = NULL;
  codeToGID = NULL;
  codeToGIDLen = 0;
}

PSFontFileInfo::~PSFontFileInfo() {
  delete psName;
  if (extFileName) {
    delete extFileName;
  }
  if (encoding) {
    delete encoding;
  }
  if (codeToGID) {
    gfree(codeToGID);
  }
}

//------------------------------------------------------------------------
// process colors
//------------------------------------------------------------------------

#define psProcessCyan     1
#define psProcessMagenta  2
#define psProcessYellow   4
#define psProcessBlack    8
#define psProcessCMYK    15

//------------------------------------------------------------------------
// PSOutCustomColor
//------------------------------------------------------------------------

class PSOutCustomColor {
public:

  PSOutCustomColor(double cA, double mA,
		   double yA, double kA, GString *nameA);
  ~PSOutCustomColor();

  double c, m, y, k;
  GString *name;
  PSOutCustomColor *next;
};

PSOutCustomColor::PSOutCustomColor(double cA, double mA,
				   double yA, double kA, GString *nameA) {
  c = cA;
  m = mA;
  y = yA;
  k = kA;
  name = nameA;
  next = NULL;
}

PSOutCustomColor::~PSOutCustomColor() {
  delete name;
}

//------------------------------------------------------------------------

struct PSOutImgClipRect {
  int x0, x1, y0, y1;
};

//------------------------------------------------------------------------

struct PSOutPaperSize {
  PSOutPaperSize(int wA, int hA) { w = wA; h = hA; }
  int w, h;
};

//------------------------------------------------------------------------
// DeviceNRecoder
//------------------------------------------------------------------------

class DeviceNRecoder: public FilterStream {
public:

  DeviceNRecoder(Stream *strA, int widthA, int heightA,
		 GfxImageColorMap *colorMapA);
  virtual ~DeviceNRecoder();
  virtual Stream *copy();
  virtual StreamKind getKind() { return strWeird; }
  virtual void reset();
  virtual void close();
  virtual int getChar()
    { return (bufIdx >= bufSize && !fillBuf()) ? EOF : buf[bufIdx++]; }
  virtual int lookChar()
    { return (bufIdx >= bufSize && !fillBuf()) ? EOF : buf[bufIdx]; }
  virtual GString *getPSFilter(int psLevel, const char *indent) { return NULL; }
  virtual GBool isBinary(GBool last = gTrue) { return gTrue; }
  virtual GBool isEncoder() { return gTrue; }

private:

  GBool fillBuf();

  int width, height;
  GfxImageColorMap *colorMap;
  Function *func;
  ImageStream *imgStr;
  int buf[gfxColorMaxComps];
  int pixelIdx;
  int bufIdx;
  int bufSize;
};

DeviceNRecoder::DeviceNRecoder(Stream *strA, int widthA, int heightA,
			       GfxImageColorMap *colorMapA):
    FilterStream(strA) {
  width = widthA;
  height = heightA;
  colorMap = colorMapA;
  imgStr = NULL;
  pixelIdx = 0;
  bufIdx = gfxColorMaxComps;
  bufSize = ((GfxDeviceNColorSpace *)colorMap->getColorSpace())->
              getAlt()->getNComps();
  func = ((GfxDeviceNColorSpace *)colorMap->getColorSpace())->
           getTintTransformFunc();
}

DeviceNRecoder::~DeviceNRecoder() {
  if (str->isEncoder()) {
    delete str;
  }
}

Stream *DeviceNRecoder::copy() {
  error(errInternal, -1, "Called copy() on DeviceNRecoder");
  return NULL;
}

void DeviceNRecoder::reset() {
  imgStr = new ImageStream(str, width, colorMap->getNumPixelComps(),
			   colorMap->getBits());
  imgStr->reset();
}

void DeviceNRecoder::close() {
  delete imgStr;
  imgStr = NULL;
  str->close();
}

GBool DeviceNRecoder::fillBuf() {
  Guchar pixBuf[gfxColorMaxComps];
  GfxColor color;
  double x[gfxColorMaxComps], y[gfxColorMaxComps];
  int i;

  if (pixelIdx >= width * height) {
    return gFalse;
  }
  imgStr->getPixel(pixBuf);
  colorMap->getColor(pixBuf, &color);
  for (i = 0;
       i < ((GfxDeviceNColorSpace *)colorMap->getColorSpace())->getNComps();
       ++i) {
    x[i] = colToDbl(color.c[i]);
  }
  func->transform(x, y);
  for (i = 0; i < bufSize; ++i) {
    buf[i] = (int)(y[i] * 255 + 0.5);
  }
  bufIdx = 0;
  ++pixelIdx;
  return gTrue;
}

//------------------------------------------------------------------------
// GrayRecoder
//------------------------------------------------------------------------

class GrayRecoder: public FilterStream {
public:

  GrayRecoder(Stream *strA, int widthA, int heightA,
	      GfxImageColorMap *colorMapA);
  virtual ~GrayRecoder();
  virtual Stream *copy();
  virtual StreamKind getKind() { return strWeird; }
  virtual void reset();
  virtual void close();
  virtual int getChar()
    { return (bufIdx >= width && !fillBuf()) ? EOF : buf[bufIdx++]; }
  virtual int lookChar()
    { return (bufIdx >= width && !fillBuf()) ? EOF : buf[bufIdx]; }
  virtual GString *getPSFilter(int psLevel, const char *indent) { return NULL; }
  virtual GBool isBinary(GBool last = gTrue) { return gTrue; }
  virtual GBool isEncoder() { return gTrue; }

private:

  GBool fillBuf();

  int width, height;
  GfxImageColorMap *colorMap;
  ImageStream *imgStr;
  Guchar *buf;
  int bufIdx;
};

GrayRecoder::GrayRecoder(Stream *strA, int widthA, int heightA,
			 GfxImageColorMap *colorMapA):
    FilterStream(strA) {
  width = widthA;
  height = heightA;
  colorMap = colorMapA;
  imgStr = NULL;
  buf = (Guchar *)gmalloc(width);
  bufIdx = width;
}

GrayRecoder::~GrayRecoder() {
  gfree(buf);
  if (str->isEncoder()) {
    delete str;
  }
}

Stream *GrayRecoder::copy() {
  error(errInternal, -1, "Called copy() on GrayRecoder");
  return NULL;
}

void GrayRecoder::reset() {
  imgStr = new ImageStream(str, width, colorMap->getNumPixelComps(),
			   colorMap->getBits());
  imgStr->reset();
}

void GrayRecoder::close() {
  delete imgStr;
  imgStr = NULL;
  str->close();
}

GBool GrayRecoder::fillBuf() {
  Guchar *line;

  if (!(line = imgStr->getLine())) {
    bufIdx = width;
    return gFalse;
  }
  //~ this should probably use the rendering intent from the image
  //~   dict, or from the content stream
  colorMap->getGrayByteLine(line, buf, width,
			    gfxRenderingIntentRelativeColorimetric);
  bufIdx = 0;
  return gTrue;
}

//------------------------------------------------------------------------
// ColorKeyToMaskEncoder
//------------------------------------------------------------------------

class ColorKeyToMaskEncoder: public FilterStream {
public:

  ColorKeyToMaskEncoder(Stream *strA, int widthA, int heightA,
			GfxImageColorMap *colorMapA, int *maskColorsA);
  virtual ~ColorKeyToMaskEncoder();
  virtual Stream *copy();
  virtual StreamKind getKind() { return strWeird; }
  virtual void reset();
  virtual void close();
  virtual int getChar()
    { return (bufIdx >= bufSize && !fillBuf()) ? EOF : buf[bufIdx++]; }
  virtual int lookChar()
    { return (bufIdx >= bufSize && !fillBuf()) ? EOF : buf[bufIdx]; }
  virtual GString *getPSFilter(int psLevel, const char *indent) { return NULL; }
  virtual GBool isBinary(GBool last = gTrue) { return gTrue; }
  virtual GBool isEncoder() { return gTrue; }

private:

  GBool fillBuf();

  int width, height;
  GfxImageColorMap *colorMap;
  int numComps;
  int *maskColors;
  ImageStream *imgStr;
  Guchar *buf;
  int bufIdx;
  int bufSize;
};

ColorKeyToMaskEncoder::ColorKeyToMaskEncoder(Stream *strA,
					     int widthA, int heightA,
					     GfxImageColorMap *colorMapA,
					     int *maskColorsA):
  FilterStream(strA)
{
  width = widthA;
  height = heightA;
  colorMap = colorMapA;
  numComps = colorMap->getNumPixelComps();
  maskColors = maskColorsA;
  imgStr = NULL;
  bufSize = (width + 7) / 8;
  buf = (Guchar *)gmalloc(bufSize);
  bufIdx = width;
}

ColorKeyToMaskEncoder::~ColorKeyToMaskEncoder() {
  gfree(buf);
  if (str->isEncoder()) {
    delete str;
  }
}

Stream *ColorKeyToMaskEncoder::copy() {
  error(errInternal, -1, "Called copy() on ColorKeyToMaskEncoder");
  return NULL;
}

void ColorKeyToMaskEncoder::reset() {
  imgStr = new ImageStream(str, width, colorMap->getNumPixelComps(),
			   colorMap->getBits());
  imgStr->reset();
}

void ColorKeyToMaskEncoder::close() {
  delete imgStr;
  imgStr = NULL;
  str->close();
}

GBool ColorKeyToMaskEncoder::fillBuf() {
  Guchar *line, *linePtr, *bufPtr;
  Guchar byte;
  int x, xx, i;

  if (!(line = imgStr->getLine())) {
    bufIdx = width;
    return gFalse;
  }
  linePtr = line;
  bufPtr = buf;
  for (x = 0; x < width; x += 8) {
    byte = 0;
    for (xx = 0; xx < 8; ++xx) {
      byte = (Guchar)(byte << 1);
      if (x + xx < width) {
	for (i = 0; i < numComps; ++i) {
	  if (linePtr[i] < maskColors[2 * i] ||
	      linePtr[i] > maskColors[2 * i + 1]) {
	    break;
	  }
	}
	if (i >= numComps) {
	  byte |= 1;
	}
	linePtr += numComps;
      } else {
	byte |= 1;
      }
    }
    *bufPtr++ = byte;
  }
  bufIdx = 0;
  return gTrue;
}

//------------------------------------------------------------------------
// PSOutputDev
//------------------------------------------------------------------------

extern "C" {
typedef void (*SignalFunc)(int);
}

static void outputToFile(void *stream, const char *data, int len) {
  fwrite(data, 1, len, (FILE *)stream);
}

PSOutputDev::PSOutputDev(char *fileName, PDFDoc *docA,
			 int firstPageA, int lastPageA, PSOutMode modeA,
			 int imgLLXA, int imgLLYA, int imgURXA, int imgURYA,
			 GBool manualCtrlA,
			 PSOutCustomCodeCbk customCodeCbkA,
			 void *customCodeCbkDataA,
			 GBool honorUserUnitA) {
  FILE *f;
  PSFileType fileTypeA;

  underlayCbk = NULL;
  underlayCbkData = NULL;
  overlayCbk = NULL;
  overlayCbkData = NULL;
  customCodeCbk = customCodeCbkA;
  customCodeCbkData = customCodeCbkDataA;

  rasterizePage = NULL;
  fontInfo = new GList();
  fontFileInfo = new GHash();
  imgIDs = NULL;
  formIDs = NULL;
  visitedResources = NULL;
  saveStack = NULL;
  paperSizes = NULL;
  embFontList = NULL;
  customColors = NULL;
  haveTextClip = gFalse;
  t3String = NULL;

  // open file or pipe
  if (!strcmp(fileName, "-")) {
    fileTypeA = psStdout;
    f = stdout;
  } else if (fileName[0] == '|') {
    fileTypeA = psPipe;
#ifdef HAVE_POPEN
#ifndef _WIN32
    signal(SIGPIPE, (SignalFunc)SIG_IGN);
#endif
    if (!(f = popen(fileName + 1, "w"))) {
      error(errIO, -1, "Couldn't run print command '{0:s}'", fileName);
      ok = gFalse;
      return;
    }
#else
    error(errIO, -1, "Print commands are not supported ('{0:s}')", fileName);
    ok = gFalse;
    return;
#endif
  } else {
    fileTypeA = psFile;
    if (!(f = fopen(fileName, "w"))) {
      error(errIO, -1, "Couldn't open PostScript file '{0:s}'", fileName);
      ok = gFalse;
      return;
    }
  }

  init(outputToFile, f, fileTypeA,
       docA, firstPageA, lastPageA, modeA,
       imgLLXA, imgLLYA, imgURXA, imgURYA, manualCtrlA, honorUserUnitA);
}

PSOutputDev::PSOutputDev(PSOutputFunc outputFuncA, void *outputStreamA,
			 PDFDoc *docA,
			 int firstPageA, int lastPageA, PSOutMode modeA,
			 int imgLLXA, int imgLLYA, int imgURXA, int imgURYA,
			 GBool manualCtrlA,
			 PSOutCustomCodeCbk customCodeCbkA,
			 void *customCodeCbkDataA,
			 GBool honorUserUnitA) {
  underlayCbk = NULL;
  underlayCbkData = NULL;
  overlayCbk = NULL;
  overlayCbkData = NULL;
  customCodeCbk = customCodeCbkA;
  customCodeCbkData = customCodeCbkDataA;

  rasterizePage = NULL;
  fontInfo = new GList();
  fontFileInfo = new GHash();
  imgIDs = NULL;
  formIDs = NULL;
  visitedResources = NULL;
  saveStack = NULL;
  paperSizes = NULL;
  embFontList = NULL;
  customColors = NULL;
  haveTextClip = gFalse;
  t3String = NULL;

  init(outputFuncA, outputStreamA, psGeneric,
       docA, firstPageA, lastPageA, modeA,
       imgLLXA, imgLLYA, imgURXA, imgURYA, manualCtrlA, honorUserUnitA);
}

void PSOutputDev::init(PSOutputFunc outputFuncA, void *outputStreamA,
		       PSFileType fileTypeA, PDFDoc *docA,
		       int firstPageA, int lastPageA, PSOutMode modeA,
		       int imgLLXA, int imgLLYA, int imgURXA, int imgURYA,
		       GBool manualCtrlA, GBool honorUserUnitA) {
  Catalog *catalog;
  Page *page;
  PDFRectangle *box;
  PSOutPaperSize *size;
  PSFontFileInfo *ff;
  GList *names;
  double userUnit;
  int pg, w, h, i;

  // initialize
  ok = gTrue;
  outputFunc = outputFuncA;
  outputStream = outputStreamA;
  fileType = fileTypeA;
  doc = docA;
  xref = doc->getXRef();
  catalog = doc->getCatalog();
  if ((firstPage = firstPageA) < 1) {
    firstPage = 1;
  }
  if ((lastPage = lastPageA) > doc->getNumPages()) {
    lastPage = doc->getNumPages();
  }
  level = globalParams->getPSLevel();
  mode = modeA;
  honorUserUnit = honorUserUnitA;
  paperWidth = globalParams->getPSPaperWidth();
  paperHeight = globalParams->getPSPaperHeight();
  imgLLX = imgLLXA;
  imgLLY = imgLLYA;
  imgURX = imgURXA;
  imgURY = imgURYA;
  if (imgLLX == 0 && imgURX == 0 && imgLLY == 0 && imgURY == 0) {
    globalParams->getPSImageableArea(&imgLLX, &imgLLY, &imgURX, &imgURY);
  }
  if (paperWidth < 0 || paperHeight < 0) {
    paperMatch = gTrue;
    paperSizes = new GList();
    paperWidth = paperHeight = 1; // in case the document has zero pages
    for (pg = firstPage; pg <= lastPage; ++pg) {
      page = catalog->getPage(pg);
      if (honorUserUnit) {
	userUnit = page->getUserUnit();
      } else {
	userUnit = 1;
      }
      if (globalParams->getPSUseCropBoxAsPage()) {
	w = (int)ceil(page->getCropWidth() * userUnit);
	h = (int)ceil(page->getCropHeight() * userUnit);
      } else {
	w = (int)ceil(page->getMediaWidth() * userUnit);
	h = (int)ceil(page->getMediaHeight() * userUnit);
      }
      for (i = 0; i < paperSizes->getLength(); ++i) {
	size = (PSOutPaperSize *)paperSizes->get(i);
	if (size->w == w && size->h == h) {
	  break;
	}
      }
      if (i == paperSizes->getLength()) {
	paperSizes->append(new PSOutPaperSize(w, h));
      }
      if (w > paperWidth) {
	paperWidth = w;
      }
      if (h > paperHeight) {
	paperHeight = h;
      }
    }
    // NB: img{LLX,LLY,URX,URY} will be set by startPage()
  } else {
    paperMatch = gFalse;
  }
  preload = globalParams->getPSPreload();
  manualCtrl = manualCtrlA;
  if (mode == psModeForm) {
    lastPage = firstPage;
  }
  processColors = 0;
  inType3Char = gFalse;

#if OPI_SUPPORT
  // initialize OPI nesting levels
  opi13Nest = 0;
  opi20Nest = 0;
#endif

  tx0 = ty0 = -1;
  xScale0 = yScale0 = 0;
  rotate0 = -1;
  clipLLX0 = clipLLY0 = 0;
  clipURX0 = clipURY0 = -1;

  // initialize font lists, etc.
  for (i = 0; i < 14; ++i) {
    ff = new PSFontFileInfo(new GString(psBase14SubstFonts[i].psName),
			    fontType1, psFontFileResident);
    fontFileInfo->add(ff->psName, ff);
  }
  names = globalParams->getPSResidentFonts();
  for (i = 0; i < names->getLength(); ++i) {
    if (!fontFileInfo->lookup((GString *)names->get(i))) {
      ff = new PSFontFileInfo((GString *)names->get(i), fontType1,
			      psFontFileResident);
      fontFileInfo->add(ff->psName, ff);
    }
  }
  delete names;
  imgIDLen = 0;
  imgIDSize = 0;
  formIDLen = 0;
  formIDSize = 0;

  noStateChanges = gFalse;
  saveStack = new GList();
  numTilingPatterns = 0;
  nextFunc = 0;

  // initialize embedded font resource comment list
  embFontList = new GString();

  if (!manualCtrl) {
    // this check is needed in case the document has zero pages
    if (firstPage <= catalog->getNumPages()) {
      writeHeader(catalog->getPage(firstPage)->getMediaBox(),
		  catalog->getPage(firstPage)->getCropBox(),
		  catalog->getPage(firstPage)->getRotate());
    } else {
      box = new PDFRectangle(0, 0, 1, 1);
      writeHeader(box, box, 0);
      delete box;
    }
    if (mode != psModeForm) {
      writePS("%%BeginProlog\n");
    }
    writeXpdfProcset();
    if (mode != psModeForm) {
      writePS("%%EndProlog\n");
      writePS("%%BeginSetup\n");
    }
    writeDocSetup(catalog);
    if (mode != psModeForm) {
      writePS("%%EndSetup\n");
    }
  }

  // initialize sequential page number
  seqPage = 1;
}

PSOutputDev::~PSOutputDev() {
  PSOutCustomColor *cc;

  if (ok) {
    if (!manualCtrl) {
      writePS("%%Trailer\n");
      writeTrailer();
      if (mode != psModeForm) {
	writePS("%%EOF\n");
      }
    }
    if (fileType == psFile) {
      fclose((FILE *)outputStream);
    }
#ifdef HAVE_POPEN
    else if (fileType == psPipe) {
      pclose((FILE *)outputStream);
#ifndef _WIN32
      signal(SIGPIPE, (SignalFunc)SIG_DFL);
#endif
    }
#endif
  }
  gfree(rasterizePage);
  if (paperSizes) {
    deleteGList(paperSizes, PSOutPaperSize);
  }
  if (embFontList) {
    delete embFontList;
  }
  deleteGList(fontInfo, PSFontInfo);
  deleteGHash(fontFileInfo, PSFontFileInfo);
  gfree(imgIDs);
  gfree(formIDs);
  if (saveStack) {
    delete saveStack;
  }
  while (customColors) {
    cc = customColors;
    customColors = cc->next;
    delete cc;
  }
}

GBool PSOutputDev::checkIO() {
  if (fileType == psFile || fileType == psPipe || fileType == psStdout) {
    if (ferror((FILE *)outputStream)) {
      error(errIO, -1, "Error writing to PostScript file");
      return gFalse;
    }
  }
  return gTrue;
}

void PSOutputDev::writeHeader(PDFRectangle *mediaBox, PDFRectangle *cropBox,
			      int pageRotate) {
  Object info, obj1;
  PSOutPaperSize *size;
  double x1, y1, x2, y2;
  int i;

  switch (mode) {
  case psModePS:
    writePS("%!PS-Adobe-3.0\n");
    break;
  case psModeEPS:
    writePS("%!PS-Adobe-3.0 EPSF-3.0\n");
    break;
  case psModeForm:
    writePS("%!PS-Adobe-3.0 Resource-Form\n");
    break;
  }

  writePSFmt("%XpdfVersion: {0:s}\n", xpdfVersion);
  xref->getDocInfo(&info);
  if (info.isDict() && info.dictLookup("Creator", &obj1)->isString()) {
    writePS("%%Creator: ");
    writePSTextLine(obj1.getString());
  }
  obj1.free();
  if (info.isDict() && info.dictLookup("Title", &obj1)->isString()) {
    writePS("%%Title: ");
    writePSTextLine(obj1.getString());
  }
  obj1.free();
  info.free();
  writePSFmt("%%LanguageLevel: {0:d}\n",
	     level >= psLevel3 ? 3 : level >= psLevel2 ? 2 : 1);
  if (level == psLevel1Sep || level == psLevel2Sep || level == psLevel3Sep) {
    writePS("%%DocumentProcessColors: (atend)\n");
    writePS("%%DocumentCustomColors: (atend)\n");
  }
  writePS("%%DocumentSuppliedResources: (atend)\n");

  switch (mode) {
  case psModePS:
    if (paperMatch) {      
      for (i = 0; i < paperSizes->getLength(); ++i) {
	size = (PSOutPaperSize *)paperSizes->get(i);
	writePSFmt("%%{0:s} {1:d}x{2:d} {1:d} {2:d} 0 () ()\n",
		   i==0 ? "DocumentMedia:" : "+", size->w, size->h);
      }
    } else {
      writePSFmt("%%DocumentMedia: plain {0:d} {1:d} 0 () ()\n",
		 paperWidth, paperHeight);
    }
    writePSFmt("%%BoundingBox: 0 0 {0:d} {1:d}\n", paperWidth, paperHeight);
    writePSFmt("%%Pages: {0:d}\n", lastPage - firstPage + 1);
    writePS("%%EndComments\n");
    if (!paperMatch) {
      writePS("%%BeginDefaults\n");
      writePS("%%PageMedia: plain\n");
      writePS("%%EndDefaults\n");
    }
    break;
  case psModeEPS:
    epsX1 = cropBox->x1;
    epsY1 = cropBox->y1;
    epsX2 = cropBox->x2;
    epsY2 = cropBox->y2;
    if (pageRotate == 0 || pageRotate == 180) {
      x1 = epsX1;
      y1 = epsY1;
      x2 = epsX2;
      y2 = epsY2;
    } else { // pageRotate == 90 || pageRotate == 270
      x1 = 0;
      y1 = 0;
      x2 = epsY2 - epsY1;
      y2 = epsX2 - epsX1;
    }
    writePSFmt("%%BoundingBox: {0:d} {1:d} {2:d} {3:d}\n",
	       (int)floor(x1), (int)floor(y1), (int)ceil(x2), (int)ceil(y2));
    if (floor(x1) != ceil(x1) || floor(y1) != ceil(y1) ||
	floor(x2) != ceil(x2) || floor(y2) != ceil(y2)) {
      writePSFmt("%%HiResBoundingBox: {0:.6g} {1:.6g} {2:.6g} {3:.6g}\n",
		 x1, y1, x2, y2);
    }
    writePS("%%EndComments\n");
    break;
  case psModeForm:
    writePS("%%EndComments\n");
    writePS("32 dict dup begin\n");
    writePSFmt("/BBox [{0:d} {1:d} {2:d} {3:d}] def\n",
	       (int)floor(mediaBox->x1), (int)floor(mediaBox->y1),
	       (int)ceil(mediaBox->x2), (int)ceil(mediaBox->y2));
    writePS("/FormType 1 def\n");
    writePS("/Matrix [1 0 0 1 0 0] def\n");
    break;
  }
}

void PSOutputDev::writeXpdfProcset() {
  GBool lev1, lev2, lev3, nonSep, gray, sep;
  const char **p;
  const char *q;
  double w;

  writePSFmt("%%BeginResource: procset xpdf {0:s} 0\n", xpdfVersion);
  writePSFmt("%%Copyright: {0:s}\n", xpdfCopyright);
  lev1 = lev2 = lev3 = nonSep = gray = sep = gTrue;
  for (p = prolog; *p; ++p) {
    if ((*p)[0] == '~') {
      lev1 = lev2 = lev3 = nonSep = gray = sep = gFalse;
      for (q = *p + 1; *q; ++q) {
	switch (*q) {
	case '1': lev1 = gTrue; break;
	case '2': lev2 = gTrue; break;
	case '3': lev3 = gTrue; break;
	case 'g': gray = gTrue; break;
	case 'n': nonSep = gTrue; break;
	case 's': sep = gTrue; break;
	}
      }
    } else if ((level == psLevel1 && lev1 && nonSep) ||
	       (level == psLevel1Sep && lev1 && sep) ||
	       (level == psLevel2 && lev2 && nonSep) ||
	       (level == psLevel2Gray && lev2 && gray) ||
	       (level == psLevel2Sep && lev2 && sep) ||
	       (level == psLevel3 && lev3 && nonSep) ||
	       (level == psLevel3Gray && lev3 && gray) ||
	       (level == psLevel3Sep && lev3 && sep)) {
      writePSFmt("{0:s}\n", *p);
    }
  }
  if ((w = globalParams->getPSMinLineWidth()) > 0) {
    writePSFmt("/pdfMinLineWidth {0:.4g} def\n", w);
    for (p = minLineWidthProlog; *p; ++p) {
      writePSFmt("{0:s}\n", *p);
    }
  }
  writePS("%%EndResource\n");

  if (level >= psLevel3) {
    for (p = cmapProlog; *p; ++p) {
      writePSFmt("{0:s}\n", *p);
    }
  }
}

void PSOutputDev::writeDocSetup(Catalog *catalog) {
  Page *page;
  Dict *resDict;
  Annots *annots;
  Form *form;
  Object obj1, obj2, obj3;
  GString *s;
  GBool needDefaultFont;
  int pg, i, j;

  // check to see which pages will be rasterized
  if (firstPage <= lastPage) {
    rasterizePage = (char *)gmalloc(lastPage - firstPage + 1);
    for (pg = firstPage; pg <= lastPage; ++pg) {
      rasterizePage[pg - firstPage] = (char)checkIfPageNeedsToBeRasterized(pg);
    }
  } else {
    rasterizePage = NULL;
  }

  visitedResources = (char *)gmalloc(xref->getNumObjects());
  memset(visitedResources, 0, xref->getNumObjects());

  if (mode == psModeForm) {
    // swap the form and xpdf dicts
    writePS("xpdf end begin dup begin\n");
  } else {
    writePS("xpdf begin\n");
  }
  needDefaultFont = gFalse;
  for (pg = firstPage; pg <= lastPage; ++pg) {
    if (rasterizePage[pg - firstPage]) {
      continue;
    }
    page = catalog->getPage(pg);
    if ((resDict = page->getResourceDict())) {
      setupResources(resDict);
    }
    annots = new Annots(doc, page->getAnnots(&obj1));
    obj1.free();
    if (annots->getNumAnnots()) {
      needDefaultFont = gTrue;
    }
    for (i = 0; i < annots->getNumAnnots(); ++i) {
      if (annots->getAnnot(i)->getAppearance(&obj1)->isStream()) {
	obj1.streamGetDict()->lookup("Resources", &obj2);
	if (obj2.isDict()) {
	  setupResources(obj2.getDict());
	}
	obj2.free();
      }
      obj1.free();
    }
    delete annots;
  }
  if ((form = catalog->getForm())) {
    if (form->getNumFields() > 0) {
      needDefaultFont = gTrue;
    }
    for (i = 0; i < form->getNumFields(); ++i) {
      form->getField(i)->getResources(&obj1);
      if (obj1.isArray()) {
	for (j = 0; j < obj1.arrayGetLength(); ++j) {
	  obj1.arrayGet(j, &obj2);
	  if (obj2.isDict()) {
	    setupResources(obj2.getDict());
	  }
	  obj2.free();
	}
      } else if (obj1.isDict()) {
	setupResources(obj1.getDict());
      }
      obj1.free();
    }
  }
  if (needDefaultFont) {
    setupDefaultFont();
  }
  if (mode != psModeForm) {
    if (mode != psModeEPS && !manualCtrl) {
      writePSFmt("{0:s} pdfSetup\n",
		 globalParams->getPSDuplex() ? "true" : "false");
      if (!paperMatch) {
	writePSFmt("{0:d} {1:d} pdfSetupPaper\n", paperWidth, paperHeight);
      }
    }
#if OPI_SUPPORT
    if (globalParams->getPSOPI()) {
      writePS("/opiMatrix matrix currentmatrix def\n");
    }
#endif
  }
  if (customCodeCbk) {
    if ((s = (*customCodeCbk)(this, psOutCustomDocSetup, 0,
			      customCodeCbkData))) {
      writePS(s->getCString());
      delete s;
    }
  }
  if (mode != psModeForm) {
    writePS("end\n");
  }

  gfree(visitedResources);
  visitedResources = NULL;
}

void PSOutputDev::writePageTrailer() {
  if (mode != psModeForm) {
    writePS("pdfEndPage\n");
  }
}

void PSOutputDev::writeTrailer() {
  PSOutCustomColor *cc;

  if (mode == psModeForm) {
    writePS("/Foo exch /Form defineresource pop\n");
  } else {
    writePS("%%DocumentSuppliedResources:\n");
    writePS(embFontList->getCString());
    if (level == psLevel1Sep || level == psLevel2Sep ||
	level == psLevel3Sep) {
      writePS("%%DocumentProcessColors:");
      if (processColors & psProcessCyan) {
	writePS(" Cyan");
      }
      if (processColors & psProcessMagenta) {
	writePS(" Magenta");
      }
      if (processColors & psProcessYellow) {
	writePS(" Yellow");
      }
      if (processColors & psProcessBlack) {
	writePS(" Black");
      }
      writePS("\n");
      writePS("%%DocumentCustomColors:");
      for (cc = customColors; cc; cc = cc->next) {
	writePS(" ");
	writePSString(cc->name);
      }
      writePS("\n");
      writePS("%%CMYKCustomColor:\n");
      for (cc = customColors; cc; cc = cc->next) {
	writePSFmt("%%+ {0:.4g} {1:.4g} {2:.4g} {3:.4g} ",
		   cc->c, cc->m, cc->y, cc->k);
	writePSString(cc->name);
	writePS("\n");
      }
    }
  }
}

GBool PSOutputDev::checkIfPageNeedsToBeRasterized(int pg) {
  PreScanOutputDev *scan;
  GBool rasterize;

  if (globalParams->getPSAlwaysRasterize()) {
    rasterize = gTrue;
  } else {
    scan = new PreScanOutputDev();
    //~ this could depend on the printing flag, e.g., if an annotation
    //~   uses transparency --> need to pass the printing flag into
    //~   constructor, init, writeDocSetup
    doc->getCatalog()->getPage(pg)->display(scan, 72, 72, 0,
					    gTrue, gTrue, gTrue);
    rasterize = scan->usesTransparency() || scan->usesPatternImageMask();
    delete scan;
    if (rasterize && globalParams->getPSNeverRasterize()) {
      error(errSyntaxWarning, -1,
	    "PDF page uses transparency and the psNeverRasterize option is "
	    "set - output may not be correct");
      rasterize = gFalse;
    }
  }
  return rasterize;
}

void PSOutputDev::setupResources(Dict *resDict) {
  Object xObjDict, xObjRef, xObj, patDict, patRef, pat;
  Object gsDict, gsRef, gs, smask, smaskGroup, resObj;
  Ref ref0;
  GBool skip;
  int i;

  setupFonts(resDict);
  setupImages(resDict);

  //----- recursively scan XObjects
  resDict->lookup("XObject", &xObjDict);
  if (xObjDict.isDict()) {
    for (i = 0; i < xObjDict.dictGetLength(); ++i) {

      // check for an already-visited XObject
      skip = gFalse;
      if ((xObjDict.dictGetValNF(i, &xObjRef)->isRef())) {
	ref0 = xObjRef.getRef();
	if (ref0.num < 0 || ref0.num >= xref->getNumObjects()) {
	  skip = gTrue;
	} else {
	  skip = (GBool)visitedResources[ref0.num];
	  visitedResources[ref0.num] = 1;
	}
      }
      if (!skip) {

	// process the XObject's resource dictionary
	xObjDict.dictGetVal(i, &xObj);
	if (xObj.isStream()) {
	  xObj.streamGetDict()->lookup("Resources", &resObj);
	  if (resObj.isDict()) {
	    setupResources(resObj.getDict());
	  }
	  resObj.free();
	}
	xObj.free();
      }

      xObjRef.free();
    }
  }
  xObjDict.free();

  //----- recursively scan Patterns
  resDict->lookup("Pattern", &patDict);
  if (patDict.isDict()) {
    inType3Char = gTrue;
    for (i = 0; i < patDict.dictGetLength(); ++i) {

      // check for an already-visited Pattern
      skip = gFalse;
      if ((patDict.dictGetValNF(i, &patRef)->isRef())) {
	ref0 = patRef.getRef();
	if (ref0.num < 0 || ref0.num >= xref->getNumObjects()) {
	  skip = gTrue;
	} else {
	  skip = (GBool)visitedResources[ref0.num];
	  visitedResources[ref0.num] = 1;
	}
      }
      if (!skip) {

	// process the Pattern's resource dictionary
	patDict.dictGetVal(i, &pat);
	if (pat.isStream()) {
	  pat.streamGetDict()->lookup("Resources", &resObj);
	  if (resObj.isDict()) {
	    setupResources(resObj.getDict());
	  }
	  resObj.free();
	}
	pat.free();
      }

      patRef.free();
    }
    inType3Char = gFalse;
  }
  patDict.free();

  //----- recursively scan SMask transparency groups in ExtGState dicts
  resDict->lookup("ExtGState", &gsDict);
  if (gsDict.isDict()) {
    for (i = 0; i < gsDict.dictGetLength(); ++i) {

      // check for an already-visited ExtGState
      skip = gFalse;
      if ((gsDict.dictGetValNF(i, &gsRef)->isRef())) {
	ref0 = gsRef.getRef();
	if (ref0.num < 0 || ref0.num >= xref->getNumObjects()) {
	  skip = gTrue;
	} else {
	  skip = (GBool)visitedResources[ref0.num];
	  visitedResources[ref0.num] = 1;
	}
      }
      if (!skip) {

	// process the ExtGState's SMask's transparency group's resource dict
	if (gsDict.dictGetVal(i, &gs)->isDict()) {
	  if (gs.dictLookup("SMask", &smask)->isDict()) {
	    if (smask.dictLookup("G", &smaskGroup)->isStream()) {
	      smaskGroup.streamGetDict()->lookup("Resources", &resObj);
	      if (resObj.isDict()) {
		setupResources(resObj.getDict());
	      }
	      resObj.free();
	    }
	    smaskGroup.free();
	  }
	  smask.free();
	}
	gs.free();
      }

      gsRef.free();
    }
  }
  gsDict.free();

  setupForms(resDict);
}

void PSOutputDev::setupFonts(Dict *resDict) {
  Object obj1, obj2;
  Ref r;
  GfxFontDict *gfxFontDict;
  GfxFont *font;
  int i;

  gfxFontDict = NULL;
  resDict->lookupNF("Font", &obj1);
  if (obj1.isRef()) {
    obj1.fetch(xref, &obj2);
    if (obj2.isDict()) {
      r = obj1.getRef();
      gfxFontDict = new GfxFontDict(xref, &r, obj2.getDict());
    }
    obj2.free();
  } else if (obj1.isDict()) {
    gfxFontDict = new GfxFontDict(xref, NULL, obj1.getDict());
  }
  if (gfxFontDict) {
    for (i = 0; i < gfxFontDict->getNumFonts(); ++i) {
      if ((font = gfxFontDict->getFont(i))) {
	setupFont(font, resDict);
      }
    }
    delete gfxFontDict;
  }
  obj1.free();
}

void PSOutputDev::setupFont(GfxFont *font, Dict *parentResDict) {
  PSFontInfo *fi;
  GfxFontLoc *fontLoc;
  GBool subst;
  char buf[16];
  UnicodeMap *uMap;
  char *charName;
  double xs, ys;
  int code;
  double w1, w2;
  int i, j;

  // check if font is already set up
  for (i = 0; i < fontInfo->getLength(); ++i) {
    fi = (PSFontInfo *)fontInfo->get(i);
    if (fi->fontID.num == font->getID()->num &&
	fi->fontID.gen == font->getID()->gen) {
      return;
    }
  }

  // add fontInfo entry
  fi = new PSFontInfo(*font->getID());
  fontInfo->append(fi);

  xs = ys = 1;
  subst = gFalse;

  if (font->getType() == fontType3) {
    fi->ff = setupType3Font(font, parentResDict);
  } else {
    if ((fontLoc = font->locateFont(xref, gTrue))) {
      switch (fontLoc->locType) {
      case gfxFontLocEmbedded:
	switch (fontLoc->fontType) {
	case fontType1:
	  fi->ff = setupEmbeddedType1Font(font, &fontLoc->embFontID);
	  break;
	case fontType1C:
	  fi->ff = setupEmbeddedType1CFont(font, &fontLoc->embFontID);
	  break;
	case fontType1COT:
	  fi->ff = setupEmbeddedOpenTypeT1CFont(font, &fontLoc->embFontID);
	  break;
	case fontTrueType:
	case fontTrueTypeOT:
	  fi->ff = setupEmbeddedTrueTypeFont(font, &fontLoc->embFontID);
	  break;
	case fontCIDType0C:
	  fi->ff = setupEmbeddedCIDType0Font(font, &fontLoc->embFontID);
	  break;
	case fontCIDType2:
	case fontCIDType2OT:
	  //~ should check to see if font actually uses vertical mode
	  fi->ff = setupEmbeddedCIDTrueTypeFont(font, &fontLoc->embFontID,
						gTrue);
	  break;
	case fontCIDType0COT:
	  fi->ff = setupEmbeddedOpenTypeCFFFont(font, &fontLoc->embFontID);
	  break;
	default:
	  break;
	}
	break;
      case gfxFontLocExternal:
	//~ add cases for other external 16-bit fonts
	switch (fontLoc->fontType) {
	case fontType1:
	  fi->ff = setupExternalType1Font(font, fontLoc->path);
	  break;
	case fontTrueType:
	case fontTrueTypeOT:
	  fi->ff = setupExternalTrueTypeFont(font, fontLoc->path,
					     fontLoc->fontNum);
	  break;
	case fontCIDType2:
	case fontCIDType2OT:
	  //~ should check to see if font actually uses vertical mode
	  fi->ff = setupExternalCIDTrueTypeFont(font, fontLoc->path,
						fontLoc->fontNum, gTrue);
	  break;
	case fontCIDType0COT:
	  fi->ff = setupExternalOpenTypeCFFFont(font, fontLoc->path);
	  break;
	default:
	  break;
	}
	break;
      case gfxFontLocResident:
	if (!(fi->ff = (PSFontFileInfo *)fontFileInfo->lookup(fontLoc->path))) {
	  // handle psFontPassthrough
	  fi->ff = new PSFontFileInfo(fontLoc->path->copy(), fontLoc->fontType,
				      psFontFileResident);
	  fontFileInfo->add(fi->ff->psName, fi->ff);
	}
	break;
      }
    }

    if (!fi->ff) {
      if (font->isCIDFont()) {
	error(errSyntaxError, -1,
	      "Couldn't find a font for '{0:s}' ('{1:s}' character collection)",
	      font->getName() ? font->getName()->getCString()
	                      : "(unnamed)",
	      ((GfxCIDFont *)font)->getCollection()
	          ? ((GfxCIDFont *)font)->getCollection()->getCString()
	          : "(unknown)");
      } else {
	error(errSyntaxError, -1,
	      "Couldn't find a font for '{0:s}'",
	      font->getName() ? font->getName()->getCString()
	                      : "(unnamed)");
      }
      delete fontLoc;
      return;
    }

    // scale substituted 8-bit fonts
    if (fontLoc->locType == gfxFontLocResident &&
	fontLoc->substIdx >= 0) {
      subst = gTrue;
      for (code = 0; code < 256; ++code) {
	if ((charName = ((Gfx8BitFont *)font)->getCharName(code)) &&
	    charName[0] == 'm' && charName[1] == '\0') {
	  break;
	}
      }
      if (code < 256) {
	w1 = ((Gfx8BitFont *)font)->getWidth((Guchar)code);
      } else {
	w1 = 0;
      }
      w2 = psBase14SubstFonts[fontLoc->substIdx].mWidth;
      xs = w1 / w2;
      if (xs < 0.1) {
	xs = 1;
      }
    }

    // handle encodings for substituted CID fonts
    if (fontLoc->locType == gfxFontLocResident &&
	fontLoc->fontType >= fontCIDType0) {
      subst = gTrue;
      if ((uMap = globalParams->getUnicodeMap(fontLoc->encoding))) {
	fi->ff->encoding = fontLoc->encoding->copy();
	uMap->decRefCnt();
      } else {
	error(errSyntaxError, -1,
	      "Couldn't find Unicode map for 16-bit font encoding '{0:t}'",
	      fontLoc->encoding);
      }
    }

    delete fontLoc;
  }

  // generate PostScript code to set up the font
  if (font->isCIDFont()) {
    if (level >= psLevel3) {
      writePSFmt("/F{0:d}_{1:d} /{2:t} {3:d} pdfMakeFont16L3\n",
		 font->getID()->num, font->getID()->gen, fi->ff->psName,
		 font->getWMode());
    } else {
      writePSFmt("/F{0:d}_{1:d} /{2:t} {3:d} pdfMakeFont16\n",
		 font->getID()->num, font->getID()->gen, fi->ff->psName,
		 font->getWMode());
    }
  } else {
    writePSFmt("/F{0:d}_{1:d} /{2:t} {3:.6g} {4:.6g}\n",
	       font->getID()->num, font->getID()->gen, fi->ff->psName, xs, ys);
    for (i = 0; i < 256; i += 8) {
      writePS((char *)((i == 0) ? "[ " : "  "));
      for (j = 0; j < 8; ++j) {
	if (font->getType() == fontTrueType &&
	    !subst &&
	    !((Gfx8BitFont *)font)->getHasEncoding()) {
	  sprintf(buf, "c%02x", i+j);
	  charName = buf;
	} else {
	  charName = ((Gfx8BitFont *)font)->getCharName(i+j);
	}
	writePS("/");
	writePSName(charName ? charName : (char *)".notdef");
	// the empty name is legal in PDF and PostScript, but PostScript
	// uses a double-slash (//...) for "immediately evaluated names",
	// so we need to add a space character here
	if (charName && !charName[0]) {
	  writePS(" ");
	}
      }
      writePS((i == 256-8) ? (char *)"]\n" : (char *)"\n");
    }
    writePS("pdfMakeFont\n");
  }
}

PSFontFileInfo *PSOutputDev::setupEmbeddedType1Font(GfxFont *font, Ref *id) {
  GString *psName, *origFont, *cleanFont;
  PSFontFileInfo *ff;
  Object refObj, strObj, obj1, obj2;
  Dict *dict;
  char buf[4096];
  GBool rename;
  int length1, length2, n;

  // check if font is already embedded
  if (!font->getEmbeddedFontName()) {
    rename = gTrue;
  } else if ((ff = (PSFontFileInfo *)
	             fontFileInfo->lookup(font->getEmbeddedFontName()))) {
    if (ff->loc == psFontFileEmbedded &&
	ff->embFontID.num == id->num &&
	ff->embFontID.gen == id->gen) {
      return ff;
    }
    rename = gTrue;
  } else {
    rename = gFalse;
  }

  // generate name
  // (this assumes that the PS font name matches the PDF font name)
  if (rename) {
    psName = makePSFontName(font, id);
  } else {
    psName = font->getEmbeddedFontName()->copy();
  }

  // get the font stream and info
  refObj.initRef(id->num, id->gen);
  refObj.fetch(xref, &strObj);
  refObj.free();
  if (!strObj.isStream()) {
    error(errSyntaxError, -1, "Embedded font file object is not a stream");
    goto err1;
  }
  if (!(dict = strObj.streamGetDict())) {
    error(errSyntaxError, -1,
	  "Embedded font stream is missing its dictionary");
    goto err1;
  }
  dict->lookup("Length1", &obj1);
  dict->lookup("Length2", &obj2);
  if (!obj1.isInt() || !obj2.isInt()) {
    error(errSyntaxError, -1,
	  "Missing length fields in embedded font stream dictionary");
    obj1.free();
    obj2.free();
    goto err1;
  }
  length1 = obj1.getInt();
  length2 = obj2.getInt();
  obj1.free();
  obj2.free();

  // read the font file
  origFont = new GString();
  strObj.streamReset();
  while ((n = strObj.streamGetBlock(buf, sizeof(buf))) > 0) {
    origFont->append(buf, n);
  }
  strObj.streamClose();
  strObj.free();

  // beginning comment
  writePSFmt("%%BeginResource: font {0:t}\n", psName);
  embFontList->append("%%+ font ");
  embFontList->append(psName->getCString());
  embFontList->append("\n");

  // clean up the font file
  cleanFont = fixType1Font(origFont, length1, length2);
  if (rename) {
    renameType1Font(cleanFont, psName);
  }
  writePSBlock(cleanFont->getCString(), cleanFont->getLength());
  delete cleanFont;
  delete origFont;

  // ending comment
  writePS("%%EndResource\n");

  ff = new PSFontFileInfo(psName, font->getType(), psFontFileEmbedded);
  ff->embFontID = *id;
  fontFileInfo->add(ff->psName, ff);
  return ff;

 err1:
  strObj.free();
  delete psName;
  return NULL;
}

PSFontFileInfo *PSOutputDev::setupExternalType1Font(GfxFont *font,
						    GString *fileName) {
  static char hexChar[17] = "0123456789abcdef";
  GString *psName;
  PSFontFileInfo *ff;
  FILE *fontFile;
  int buf[6];
  int c, n, i;

  if (font->getName()) {
    // check if font is already embedded
    if ((ff = (PSFontFileInfo *)fontFileInfo->lookup(font->getName()))) {
      return ff;
    }
    // this assumes that the PS font name matches the PDF font name
    psName = font->getName()->copy();
  } else {
    // generate name
    //~ this won't work -- the PS font name won't match
    psName = makePSFontName(font, font->getID());
  }

  // beginning comment
  writePSFmt("%%BeginResource: font {0:t}\n", psName);
  embFontList->append("%%+ font ");
  embFontList->append(psName->getCString());
  embFontList->append("\n");

  // open the font file
  if (!(fontFile = fopen(fileName->getCString(), "rb"))) {
    error(errIO, -1, "Couldn't open external font file");
    return NULL;
  }

  // check for PFB format
  buf[0] = fgetc(fontFile);
  buf[1] = fgetc(fontFile);
  if (buf[0] == 0x80 && buf[1] == 0x01) {
    while (1) {
      for (i = 2; i < 6; ++i) {
	buf[i] = fgetc(fontFile);
      }
      if (buf[2] == EOF || buf[3] == EOF || buf[4] == EOF || buf[5] == EOF) {
	break;
      }
      n = buf[2] + (buf[3] << 8) + (buf[4] << 16) + (buf[5] << 24);
      if (buf[1] == 0x01) {
	for (i = 0; i < n; ++i) {
	  if ((c = fgetc(fontFile)) == EOF) {
	    break;
	  }
	  writePSChar((char)c);
	}
      } else {
	for (i = 0; i < n; ++i) {
	  if ((c = fgetc(fontFile)) == EOF) {
	    break;
	  }
	  writePSChar(hexChar[(c >> 4) & 0x0f]);
	  writePSChar(hexChar[c & 0x0f]);
	  if (i % 32 == 31) {
	    writePSChar('\n');
	  }
	}
      }
      buf[0] = fgetc(fontFile);
      buf[1] = fgetc(fontFile);
      if (buf[0] == EOF || buf[1] == EOF ||
	  (buf[0] == 0x80 && buf[1] == 0x03)) {
	break;
      } else if (!(buf[0] == 0x80 &&
		   (buf[1] == 0x01 || buf[1] == 0x02))) {
	error(errSyntaxError, -1,
	      "Invalid PFB header in external font file");
	break;
      }
    }
    writePSChar('\n');

  // plain text (PFA) format
  } else {
    writePSChar((char)buf[0]);
    writePSChar((char)buf[1]);
    while ((c = fgetc(fontFile)) != EOF) {
      writePSChar((char)c);
    }
  }

  fclose(fontFile);

  // ending comment
  writePS("%%EndResource\n");

  ff = new PSFontFileInfo(psName, font->getType(), psFontFileExternal);
  ff->extFileName = fileName->copy();
  fontFileInfo->add(ff->psName, ff);
  return ff;
}

PSFontFileInfo *PSOutputDev::setupEmbeddedType1CFont(GfxFont *font, Ref *id) {
  GString *psName;
  PSFontFileInfo *ff;
  char *fontBuf;
  int fontLen;
  FoFiType1C *ffT1C;
  GHashIter *iter;

  // check if font is already embedded
  fontFileInfo->startIter(&iter);
  while (fontFileInfo->getNext(&iter, &psName, (void **)&ff)) {
    if (ff->loc == psFontFileEmbedded &&
	ff->embFontID.num == id->num &&
	ff->embFontID.gen == id->gen) {
      fontFileInfo->killIter(&iter);
      return ff;
    }
  }

  // generate name
  psName = makePSFontName(font, id);

  // beginning comment
  writePSFmt("%%BeginResource: font {0:t}\n", psName);
  embFontList->append("%%+ font ");
  embFontList->append(psName->getCString());
  embFontList->append("\n");

  // convert it to a Type 1 font
  if ((fontBuf = font->readEmbFontFile(xref, &fontLen))) {
    if ((ffT1C = FoFiType1C::make(fontBuf, fontLen))) {
      ffT1C->convertToType1(psName->getCString(), NULL, gTrue,
			    outputFunc, outputStream);
      delete ffT1C;
    }
    gfree(fontBuf);
  }

  // ending comment
  writePS("%%EndResource\n");

  ff = new PSFontFileInfo(psName, font->getType(), psFontFileEmbedded);
  ff->embFontID = *id;
  fontFileInfo->add(ff->psName, ff);
  return ff;
}

PSFontFileInfo *PSOutputDev::setupEmbeddedOpenTypeT1CFont(GfxFont *font,
							  Ref *id) {
  GString *psName;
  PSFontFileInfo *ff;
  char *fontBuf;
  int fontLen;
  FoFiTrueType *ffTT;
  GHashIter *iter;

  // check if font is already embedded
  fontFileInfo->startIter(&iter);
  while (fontFileInfo->getNext(&iter, &psName, (void **)&ff)) {
    if (ff->loc == psFontFileEmbedded &&
	ff->embFontID.num == id->num &&
	ff->embFontID.gen == id->gen) {
      fontFileInfo->killIter(&iter);
      return ff;
    }
  }

  // generate name
  psName = makePSFontName(font, id);

  // beginning comment
  writePSFmt("%%BeginResource: font {0:t}\n", psName);
  embFontList->append("%%+ font ");
  embFontList->append(psName->getCString());
  embFontList->append("\n");

  // convert it to a Type 1 font
  if ((fontBuf = font->readEmbFontFile(xref, &fontLen))) {
    if ((ffTT = FoFiTrueType::make(fontBuf, fontLen, 0, gTrue))) {
      if (ffTT->isOpenTypeCFF()) {
	ffTT->convertToType1(psName->getCString(), NULL, gTrue,
			     outputFunc, outputStream);
      }
      delete ffTT;
    }
    gfree(fontBuf);
  }

  // ending comment
  writePS("%%EndResource\n");

  ff = new PSFontFileInfo(psName, font->getType(), psFontFileEmbedded);
  ff->embFontID = *id;
  fontFileInfo->add(ff->psName, ff);
  return ff;
}

PSFontFileInfo *PSOutputDev::setupEmbeddedTrueTypeFont(GfxFont *font, Ref *id) {
  GString *psName;
  PSFontFileInfo *ff;
  char *fontBuf;
  int fontLen;
  FoFiTrueType *ffTT;
  int *codeToGID;
  GHashIter *iter;

  // get the code-to-GID mapping
  if (!(fontBuf = font->readEmbFontFile(xref, &fontLen))) {
    return NULL;
  }
  if (!(ffTT = FoFiTrueType::make(fontBuf, fontLen, 0))) {
    gfree(fontBuf);
    return NULL;
  }
  codeToGID = ((Gfx8BitFont *)font)->getCodeToGIDMap(ffTT);

  // check if font is already embedded
  fontFileInfo->startIter(&iter);
  while (fontFileInfo->getNext(&iter, &psName, (void **)&ff)) {
    if (ff->loc == psFontFileEmbedded &&
	ff->type == font->getType() &&
	ff->embFontID.num == id->num &&
	ff->embFontID.gen == id->gen &&
	ff->codeToGIDLen == 256 &&
	!memcmp(ff->codeToGID, codeToGID, 256 * sizeof(int))) {
      fontFileInfo->killIter(&iter);
      gfree(codeToGID);
      delete ffTT;
      gfree(fontBuf);
      return ff;
    }
  }

  // generate name
  psName = makePSFontName(font, id);

  // beginning comment
  writePSFmt("%%BeginResource: font {0:t}\n", psName);
  embFontList->append("%%+ font ");
  embFontList->append(psName->getCString());
  embFontList->append("\n");

  // convert it to a Type 42 font
  ffTT->convertToType42(psName->getCString(),
			((Gfx8BitFont *)font)->getHasEncoding()
			  ? ((Gfx8BitFont *)font)->getEncoding()
			  : (char **)NULL,
			codeToGID, outputFunc, outputStream);
  delete ffTT;
  gfree(fontBuf);

  // ending comment
  writePS("%%EndResource\n");

  ff = new PSFontFileInfo(psName, font->getType(), psFontFileEmbedded);
  ff->embFontID = *id;
  ff->codeToGID = codeToGID;
  ff->codeToGIDLen = 256;
  fontFileInfo->add(ff->psName, ff);
  return ff;
}

PSFontFileInfo *PSOutputDev::setupExternalTrueTypeFont(GfxFont *font,
						       GString *fileName,
						       int fontNum) {
  GString *psName;
  PSFontFileInfo *ff;
  FoFiTrueType *ffTT;
  int *codeToGID;
  GHashIter *iter;

  // get the code-to-GID mapping
  if (!(ffTT = FoFiTrueType::load(fileName->getCString(), fontNum))) {
    return NULL;
  }
  codeToGID = ((Gfx8BitFont *)font)->getCodeToGIDMap(ffTT);

  // check if font is already embedded
  fontFileInfo->startIter(&iter);
  while (fontFileInfo->getNext(&iter, &psName, (void **)&ff)) {
    if (ff->loc == psFontFileExternal &&
	ff->type == font->getType() &&
	!ff->extFileName->cmp(fileName) &&
	ff->codeToGIDLen == 256 &&
	!memcmp(ff->codeToGID, codeToGID, 256 * sizeof(int))) {
      fontFileInfo->killIter(&iter);
      gfree(codeToGID);
      delete ffTT;
      return ff;
    }
  }

  // generate name
  psName = makePSFontName(font, font->getID());

  // beginning comment
  writePSFmt("%%BeginResource: font {0:t}\n", psName);
  embFontList->append("%%+ font ");
  embFontList->append(psName->getCString());
  embFontList->append("\n");

  // convert it to a Type 42 font
  ffTT->convertToType42(psName->getCString(),
			((Gfx8BitFont *)font)->getHasEncoding()
			  ? ((Gfx8BitFont *)font)->getEncoding()
			  : (char **)NULL,
			codeToGID, outputFunc, outputStream);
  delete ffTT;

  // ending comment
  writePS("%%EndResource\n");

  ff = new PSFontFileInfo(psName, font->getType(), psFontFileExternal);
  ff->extFileName = fileName->copy();
  ff->codeToGID = codeToGID;
  ff->codeToGIDLen = 256;
  fontFileInfo->add(ff->psName, ff);
  return ff;
}

PSFontFileInfo *PSOutputDev::setupEmbeddedCIDType0Font(GfxFont *font, Ref *id) {
  GString *psName;
  PSFontFileInfo *ff;
  char *fontBuf;
  int fontLen;
  FoFiType1C *ffT1C;
  GHashIter *iter;

  // check if font is already embedded
  fontFileInfo->startIter(&iter);
  while (fontFileInfo->getNext(&iter, &psName, (void **)&ff)) {
    if (ff->loc == psFontFileEmbedded &&
	ff->embFontID.num == id->num &&
	ff->embFontID.gen == id->gen) {
      fontFileInfo->killIter(&iter);
      return ff;
    }
  }

  // generate name
  psName = makePSFontName(font, id);

  // beginning comment
  writePSFmt("%%BeginResource: font {0:t}\n", psName);
  embFontList->append("%%+ font ");
  embFontList->append(psName->getCString());
  embFontList->append("\n");

  // convert it to a Type 0 font
  if ((fontBuf = font->readEmbFontFile(xref, &fontLen))) {
    if ((ffT1C = FoFiType1C::make(fontBuf, fontLen))) {
      if (globalParams->getPSLevel() >= psLevel3) {
	// Level 3: use a CID font
	ffT1C->convertToCIDType0(psName->getCString(),
				 ((GfxCIDFont *)font)->getCIDToGID(),
				 ((GfxCIDFont *)font)->getCIDToGIDLen(),
				 outputFunc, outputStream);
      } else {
	// otherwise: use a non-CID composite font
	ffT1C->convertToType0(psName->getCString(),
			      ((GfxCIDFont *)font)->getCIDToGID(),
			      ((GfxCIDFont *)font)->getCIDToGIDLen(),
			      outputFunc, outputStream);
      }
      delete ffT1C;
    }
    gfree(fontBuf);
  }

  // ending comment
  writePS("%%EndResource\n");

  ff = new PSFontFileInfo(psName, font->getType(), psFontFileEmbedded);
  ff->embFontID = *id;
  fontFileInfo->add(ff->psName, ff);
  return ff;
}

PSFontFileInfo *PSOutputDev::setupEmbeddedCIDTrueTypeFont(
				 GfxFont *font, Ref *id,
				 GBool needVerticalMetrics) {
  GString *psName;
  PSFontFileInfo *ff;
  char *fontBuf;
  int fontLen;
  FoFiTrueType *ffTT;
  int *codeToGID;
  int codeToGIDLen;
  GHashIter *iter;

  // get the code-to-GID mapping
  codeToGID = ((GfxCIDFont *)font)->getCIDToGID();
  codeToGIDLen = ((GfxCIDFont *)font)->getCIDToGIDLen();

  // check if font is already embedded
  fontFileInfo->startIter(&iter);
  while (fontFileInfo->getNext(&iter, &psName, (void **)&ff)) {
    if (ff->loc == psFontFileEmbedded &&
	ff->type == font->getType() &&
	ff->embFontID.num == id->num &&
	ff->embFontID.gen == id->gen &&
	ff->codeToGIDLen == codeToGIDLen &&
	((!ff->codeToGID && !codeToGID) ||
	 (ff->codeToGID && codeToGID &&
	  !memcmp(ff->codeToGID, codeToGID, codeToGIDLen * sizeof(int))))) {
      fontFileInfo->killIter(&iter);
      return ff;
    }
  }

  // generate name
  psName = makePSFontName(font, id);

  // beginning comment
  writePSFmt("%%BeginResource: font {0:t}\n", psName);
  embFontList->append("%%+ font ");
  embFontList->append(psName->getCString());
  embFontList->append("\n");

  // convert it to a Type 0 font
  if ((fontBuf = font->readEmbFontFile(xref, &fontLen))) {
    if ((ffTT = FoFiTrueType::make(fontBuf, fontLen, 0))) {
      if (globalParams->getPSLevel() >= psLevel3) {
	// Level 3: use a CID font
	ffTT->convertToCIDType2(psName->getCString(),
				codeToGID, codeToGIDLen,
				needVerticalMetrics,
				outputFunc, outputStream);
      } else {
	// otherwise: use a non-CID composite font
	ffTT->convertToType0(psName->getCString(),
			     codeToGID, codeToGIDLen,
			     needVerticalMetrics,
			     outputFunc, outputStream);
      }
      delete ffTT;
    }
    gfree(fontBuf);
  }

  // ending comment
  writePS("%%EndResource\n");

  ff = new PSFontFileInfo(psName, font->getType(), psFontFileEmbedded);
  ff->embFontID = *id;
  if (codeToGIDLen) {
    ff->codeToGID = (int *)gmallocn(codeToGIDLen, sizeof(int));
    memcpy(ff->codeToGID, codeToGID, codeToGIDLen * sizeof(int));
    ff->codeToGIDLen = codeToGIDLen;
  }
  fontFileInfo->add(ff->psName, ff);
  return ff;
}

PSFontFileInfo *PSOutputDev::setupExternalCIDTrueTypeFont(
				 GfxFont *font,
				 GString *fileName,
				 int fontNum,
				 GBool needVerticalMetrics) {
  GString *psName;
  PSFontFileInfo *ff;
  FoFiTrueType *ffTT;
  int *codeToGID;
  int codeToGIDLen;
  CharCodeToUnicode *ctu;
  Unicode uBuf[8];
  int cmap, cmapPlatform, cmapEncoding, code;
  GHashIter *iter;

  // create a code-to-GID mapping, via Unicode
  if (!(ffTT = FoFiTrueType::load(fileName->getCString(), fontNum))) {
    return NULL;
  }
  if (!(ctu = ((GfxCIDFont *)font)->getToUnicode())) {
    error(errSyntaxError, -1,
	  "Couldn't find a mapping to Unicode for font '{0:s}'",
	  font->getName() ? font->getName()->getCString() : "(unnamed)");
    delete ffTT;
    return NULL;
  }
  // look for a Unicode cmap
  for (cmap = 0; cmap < ffTT->getNumCmaps(); ++cmap) {
    cmapPlatform = ffTT->getCmapPlatform(cmap);
    cmapEncoding = ffTT->getCmapEncoding(cmap);
    if ((cmapPlatform == 3 && cmapEncoding == 1) ||
	(cmapPlatform == 0 && cmapEncoding <= 4)) {
      break;
    }
  }
  if (cmap >= ffTT->getNumCmaps()) {
    error(errSyntaxError, -1,
	  "Couldn't find a Unicode cmap in font '{0:s}'",
	  font->getName() ? font->getName()->getCString() : "(unnamed)");
    ctu->decRefCnt();
    delete ffTT;
    return NULL;
  }
  // map CID -> Unicode -> GID
  if (ctu->isIdentity()) {
    codeToGIDLen = 65536;
  } else {
    codeToGIDLen = ctu->getLength();
  }
  codeToGID = (int *)gmallocn(codeToGIDLen, sizeof(int));
  for (code = 0; code < codeToGIDLen; ++code) {
    if (ctu->mapToUnicode(code, uBuf, 8) > 0) {
      codeToGID[code] = ffTT->mapCodeToGID(cmap, uBuf[0]);
    } else {
      codeToGID[code] = 0;
    }
  }
  ctu->decRefCnt();

  // check if font is already embedded
  fontFileInfo->startIter(&iter);
  while (fontFileInfo->getNext(&iter, &psName, (void **)&ff)) {
    if (ff->loc == psFontFileExternal &&
	ff->type == font->getType() &&
	!ff->extFileName->cmp(fileName) &&
	ff->codeToGIDLen == codeToGIDLen &&
	ff->codeToGID &&
	!memcmp(ff->codeToGID, codeToGID, codeToGIDLen * sizeof(int))) {
      fontFileInfo->killIter(&iter);
      gfree(codeToGID);
      delete ffTT;
      return ff;
    }
  }

  // check for embedding permission
  if (ffTT->getEmbeddingRights() < 1) {
    error(errSyntaxError, -1,
	  "TrueType font '{0:s}' does not allow embedding",
	  font->getName() ? font->getName()->getCString() : "(unnamed)");
    gfree(codeToGID);
    delete ffTT;
    return NULL;
  }

  // generate name
  psName = makePSFontName(font, font->getID());

  // beginning comment
  writePSFmt("%%BeginResource: font {0:t}\n", psName);
  embFontList->append("%%+ font ");
  embFontList->append(psName->getCString());
  embFontList->append("\n");

  // convert it to a Type 0 font
  //~ this should use fontNum to load the correct font
  if (globalParams->getPSLevel() >= psLevel3) {
    // Level 3: use a CID font
    ffTT->convertToCIDType2(psName->getCString(),
			    codeToGID, codeToGIDLen,
			    needVerticalMetrics,
			    outputFunc, outputStream);
  } else {
    // otherwise: use a non-CID composite font
    ffTT->convertToType0(psName->getCString(),
			 codeToGID, codeToGIDLen,
			 needVerticalMetrics,
			 outputFunc, outputStream);
  }
  delete ffTT;

  // ending comment
  writePS("%%EndResource\n");

  ff = new PSFontFileInfo(psName, font->getType(), psFontFileExternal);
  ff->extFileName = fileName->copy();
  ff->codeToGID = codeToGID;
  ff->codeToGIDLen = codeToGIDLen;
  fontFileInfo->add(ff->psName, ff);
  return ff;
}

PSFontFileInfo *PSOutputDev::setupEmbeddedOpenTypeCFFFont(GfxFont *font,
							  Ref *id) {
  GString *psName;
  PSFontFileInfo *ff;
  char *fontBuf;
  int fontLen;
  FoFiTrueType *ffTT;
  GHashIter *iter;
  int n;

  // check if font is already embedded
  fontFileInfo->startIter(&iter);
  while (fontFileInfo->getNext(&iter, &psName, (void **)&ff)) {
    if (ff->loc == psFontFileEmbedded &&
	ff->embFontID.num == id->num &&
	ff->embFontID.gen == id->gen) {
      fontFileInfo->killIter(&iter);
      return ff;
    }
  }

  // generate name
  psName = makePSFontName(font, id);

  // beginning comment
  writePSFmt("%%BeginResource: font {0:t}\n", psName);
  embFontList->append("%%+ font ");
  embFontList->append(psName->getCString());
  embFontList->append("\n");

  // convert it to a Type 0 font
  if ((fontBuf = font->readEmbFontFile(xref, &fontLen))) {
    if ((ffTT = FoFiTrueType::make(fontBuf, fontLen, 0, gTrue))) {
      if (ffTT->isOpenTypeCFF()) {
	if (globalParams->getPSLevel() >= psLevel3) {
	  // Level 3: use a CID font
	  ffTT->convertToCIDType0(psName->getCString(),
				  ((GfxCIDFont *)font)->getCIDToGID(),
				  ((GfxCIDFont *)font)->getCIDToGIDLen(),
				  outputFunc, outputStream);
	} else {
	  // otherwise: use a non-CID composite font
	  ffTT->convertToType0(psName->getCString(),
			       ((GfxCIDFont *)font)->getCIDToGID(),
			       ((GfxCIDFont *)font)->getCIDToGIDLen(),
			       outputFunc, outputStream);
	}
      }
      delete ffTT;
    }
    gfree(fontBuf);
  }

  // ending comment
  writePS("%%EndResource\n");

  ff = new PSFontFileInfo(psName, font->getType(), psFontFileEmbedded);
  ff->embFontID = *id;
  if ((n = ((GfxCIDFont *)font)->getCIDToGIDLen())) {
    ff->codeToGID = (int *)gmallocn(n, sizeof(int));
    memcpy(ff->codeToGID, ((GfxCIDFont *)font)->getCIDToGID(), n * sizeof(int));
    ff->codeToGIDLen = n;
  }
  fontFileInfo->add(ff->psName, ff);
  return ff;
}

// This assumes an OpenType CFF font that has a Unicode cmap (in the
// OpenType section), and a CFF blob that uses an identity CID-to-GID
// mapping.
PSFontFileInfo *PSOutputDev::setupExternalOpenTypeCFFFont(GfxFont *font,
							  GString *fileName) {
  GString *psName;
  PSFontFileInfo *ff;
  FoFiTrueType *ffTT;
  GHashIter *iter;
  CharCodeToUnicode *ctu;
  Unicode uBuf[8];
  int *codeToGID;
  int codeToGIDLen;
  int cmap, cmapPlatform, cmapEncoding, code;

  // create a code-to-GID mapping, via Unicode
  if (!(ffTT = FoFiTrueType::load(fileName->getCString(), 0, gTrue))) {
    return NULL;
  }
  if (!ffTT->isOpenTypeCFF()) {
    delete ffTT;
    return NULL;
  }
  if (!(ctu = ((GfxCIDFont *)font)->getToUnicode())) {
    error(errSyntaxError, -1,
	  "Couldn't find a mapping to Unicode for font '{0:s}'",
	  font->getName() ? font->getName()->getCString() : "(unnamed)");
    delete ffTT;
    return NULL;
  }
  // look for a Unicode cmap
  for (cmap = 0; cmap < ffTT->getNumCmaps(); ++cmap) {
    cmapPlatform = ffTT->getCmapPlatform(cmap);
    cmapEncoding = ffTT->getCmapEncoding(cmap);
    if ((cmapPlatform == 3 && cmapEncoding == 1) ||
	(cmapPlatform == 0 && cmapEncoding <= 4)) {
      break;
    }
  }
  if (cmap >= ffTT->getNumCmaps()) {
    error(errSyntaxError, -1,
	  "Couldn't find a Unicode cmap in font '{0:s}'",
	  font->getName() ? font->getName()->getCString() : "(unnamed)");
    ctu->decRefCnt();
    delete ffTT;
    return NULL;
  }
  // map CID -> Unicode -> GID
  if (ctu->isIdentity()) {
    codeToGIDLen = 65536;
  } else {
    codeToGIDLen = ctu->getLength();
  }
  codeToGID = (int *)gmallocn(codeToGIDLen, sizeof(int));
  for (code = 0; code < codeToGIDLen; ++code) {
    if (ctu->mapToUnicode(code, uBuf, 8) > 0) {
      codeToGID[code] = ffTT->mapCodeToGID(cmap, uBuf[0]);
    } else {
      codeToGID[code] = 0;
    }
  }
  ctu->decRefCnt();

  // check if font is already embedded
  fontFileInfo->startIter(&iter);
  while (fontFileInfo->getNext(&iter, &psName, (void **)&ff)) {
    if (ff->loc == psFontFileExternal &&
	ff->type == font->getType() &&
	!ff->extFileName->cmp(fileName) &&
	ff->codeToGIDLen == codeToGIDLen &&
	ff->codeToGID &&
	!memcmp(ff->codeToGID, codeToGID, codeToGIDLen * sizeof(int))) {
      fontFileInfo->killIter(&iter);
      gfree(codeToGID);
      delete ffTT;
      return ff;
    }
  }

  // generate name
  psName = makePSFontName(font, font->getID());

  // beginning comment
  writePSFmt("%%BeginResource: font {0:t}\n", psName);
  embFontList->append("%%+ font ");
  embFontList->append(psName->getCString());
  embFontList->append("\n");

  // convert it to a Type 0 font
  if (globalParams->getPSLevel() >= psLevel3) {
    // Level 3: use a CID font
    ffTT->convertToCIDType0(psName->getCString(),
			    codeToGID, codeToGIDLen,
			    outputFunc, outputStream);
  } else {
    // otherwise: use a non-CID composite font
    ffTT->convertToType0(psName->getCString(),
			 codeToGID, codeToGIDLen,
			 outputFunc, outputStream);
  }
  delete ffTT;

  // ending comment
  writePS("%%EndResource\n");

  ff = new PSFontFileInfo(psName, font->getType(), psFontFileExternal);
  ff->extFileName = fileName->copy();
  ff->codeToGID = codeToGID;
  ff->codeToGIDLen = codeToGIDLen;
  fontFileInfo->add(ff->psName, ff);
  return ff;
}

PSFontFileInfo *PSOutputDev::setupType3Font(GfxFont *font,
					    Dict *parentResDict) {
  PSFontFileInfo *ff;
  GString *psName;
  Dict *resDict;
  Dict *charProcs;
  Object charProc;
  Gfx *gfx;
  PDFRectangle box;
  double *m;
  GString *buf;
  int i;

  // generate name
  psName = GString::format("T3_{0:d}_{1:d}",
			   font->getID()->num, font->getID()->gen);

  // set up resources used by font
  if ((resDict = ((Gfx8BitFont *)font)->getResources())) {
    inType3Char = gTrue;
    setupResources(resDict);
    inType3Char = gFalse;
  } else {
    resDict = parentResDict;
  }

  // beginning comment
  writePSFmt("%%BeginResource: font {0:t}\n", psName);
  embFontList->append("%%+ font ");
  embFontList->append(psName->getCString());
  embFontList->append("\n");

  // font dictionary
  writePS("8 dict begin\n");
  writePS("/FontType 3 def\n");
  m = font->getFontMatrix();
  writePSFmt("/FontMatrix [{0:.6g} {1:.6g} {2:.6g} {3:.6g} {4:.6g} {5:.6g}] def\n",
	     m[0], m[1], m[2], m[3], m[4], m[5]);
  m = font->getFontBBox();
  writePSFmt("/FontBBox [{0:.6g} {1:.6g} {2:.6g} {3:.6g}] def\n",
	     m[0], m[1], m[2], m[3]);
  writePS("/Encoding 256 array def\n");
  writePS("  0 1 255 { Encoding exch /.notdef put } for\n");
  writePS("/BuildGlyph {\n");
  writePS("  exch /CharProcs get exch\n");
  writePS("  2 copy known not { pop /.notdef } if\n");
  writePS("  get exec\n");
  writePS("} bind def\n");
  writePS("/BuildChar {\n");
  writePS("  1 index /Encoding get exch get\n");
  writePS("  1 index /BuildGlyph get exec\n");
  writePS("} bind def\n");
  if ((charProcs = ((Gfx8BitFont *)font)->getCharProcs())) {
    writePSFmt("/CharProcs {0:d} dict def\n", charProcs->getLength());
    writePS("CharProcs begin\n");
    box.x1 = m[0];
    box.y1 = m[1];
    box.x2 = m[2];
    box.y2 = m[3];
    gfx = new Gfx(doc, this, resDict, &box, NULL);
    inType3Char = gTrue;
    for (i = 0; i < charProcs->getLength(); ++i) {
      t3FillColorOnly = gFalse;
      t3Cacheable = gFalse;
      t3NeedsRestore = gFalse;
      writePS("/");
      writePSName(charProcs->getKey(i));
      writePS(" {\n");
      gfx->display(charProcs->getValNF(i, &charProc));
      charProc.free();
      if (t3String) {
	if (t3Cacheable) {
	  buf = GString::format("{0:.6g} {1:.6g} {2:.6g} {3:.6g} {4:.6g} {5:.6g} setcachedevice\n",
				t3WX, t3WY, t3LLX, t3LLY, t3URX, t3URY);
	} else {
	  buf = GString::format("{0:.6g} {1:.6g} setcharwidth\n", t3WX, t3WY);
	}
	(*outputFunc)(outputStream, buf->getCString(), buf->getLength());
	delete buf;
	(*outputFunc)(outputStream, t3String->getCString(),
		      t3String->getLength());
	delete t3String;
	t3String = NULL;
      }
      if (t3NeedsRestore) {
	(*outputFunc)(outputStream, "Q\n", 2);
      }
      writePS("} def\n");
    }
    inType3Char = gFalse;
    delete gfx;
    writePS("end\n");
  }
  writePS("currentdict end\n");
  writePSFmt("/{0:t} exch definefont pop\n", psName);

  // ending comment
  writePS("%%EndResource\n");

  ff = new PSFontFileInfo(psName, font->getType(), psFontFileEmbedded);
  fontFileInfo->add(ff->psName, ff);
  return ff;
}

// Make a unique PS font name, based on the names given in the PDF
// font object, and an object ID (font file object for 
GString *PSOutputDev::makePSFontName(GfxFont *font, Ref *id) {
  GString *psName, *s;

  if ((s = font->getEmbeddedFontName())) {
    psName = filterPSName(s);
    if (!fontFileInfo->lookup(psName)) {
      return psName;
    }
    delete psName;
  }
  if ((s = font->getName())) {
    psName = filterPSName(s);
    if (!fontFileInfo->lookup(psName)) {
      return psName;
    }
    delete psName;
  }
  psName = GString::format("FF{0:d}_{1:d}", id->num, id->gen);
  if ((s = font->getEmbeddedFontName())) {
    s = filterPSName(s);
    psName->append('_')->append(s);
    delete s;
  } else if ((s = font->getName())) {
    s = filterPSName(s);
    psName->append('_')->append(s);
    delete s;
  }
  return psName;
}

GString *PSOutputDev::fixType1Font(GString *font, int length1, int length2) {
  Guchar *fontData;
  GString *out, *binSection;
  GBool pfb;
  int fontSize, i;

  fontData = (Guchar *)font->getCString();
  fontSize = font->getLength();

  // check for PFB
  pfb = fontSize >= 6 && fontData[0] == 0x80 && fontData[1] == 0x01;
  out = new GString();
  binSection = new GString();
  if (pfb) {
    if (!splitType1PFB(fontData, fontSize, out, binSection)) {
      delete out;
      delete binSection;
      return copyType1PFB(fontData, fontSize);
    }
  } else {
    if (!splitType1PFA(fontData, fontSize, length1, length2,
		       out, binSection)) {
      delete out;
      delete binSection;
      return copyType1PFA(fontData, fontSize);
    }
  }

  out->append('\n');

  binSection = asciiHexDecodeType1EexecSection(binSection);

  if (!fixType1EexecSection(binSection, out)) {
    delete out;
    delete binSection;
    return pfb ? copyType1PFB(fontData, fontSize)
               : copyType1PFA(fontData, fontSize);
  }
  delete binSection;

  for (i = 0; i < 8; ++i) {
    out->append("0000000000000000000000000000000000000000000000000000000000000000\n");
  }
  out->append("cleartomark\n");

  return out;
}

// Split a Type 1 font in PFA format into a text section and a binary
// section.
GBool PSOutputDev::splitType1PFA(Guchar *font, int fontSize,
				 int length1, int length2,
				 GString *textSection, GString *binSection) {
  int textLength, binStart, binLength, lastSpace, i;

  //--- extract the text section

  // Length1 is correct, and the text section ends with whitespace
  if (length1 <= fontSize &&
      length1 >= 18 &&
      !memcmp(font + length1 - 18, "currentfile eexec", 17)) {
    textLength = length1 - 1;

  // Length1 is correct, but the trailing whitespace is missing
  } else if (length1 <= fontSize &&
	     length1 >= 17 &&
	     !memcmp(font + length1 - 17, "currentfile eexec", 17)) {
    textLength = length1;

  // Length1 is incorrect
  } else {
    for (textLength = 17; textLength <= fontSize; ++textLength) {
      if (!memcmp(font + textLength - 17, "currentfile eexec", 17)) {
	break;
      }
    }
    if (textLength > fontSize) {
      return gFalse;
    }
  }

  textSection->append((char *)font, textLength);

  //--- skip whitespace between the text section and the binary section

  for (i = 0, binStart = textLength;
       i < 8 && binStart < fontSize;
       ++i, ++binStart) {
    if (font[binStart] != ' ' && font[binStart] != '\t' &&
	font[binStart] != '\n' && font[binStart] != '\r') {
      break;
    }
  }
  if (i == 8) {
    return gFalse;
  }

  //--- extract binary section

  // if we see "0000", assume Length2 is correct
  // (if Length2 is too long, it will be corrected by fixType1EexecSection)
  if (length2 > 0 && length2 < INT_MAX - 4 &&
      binStart <= fontSize - length2 - 4 &&
      !memcmp(font + binStart + length2, "0000", 4)) {
    binLength = length2;

  } else {

    // look for "0000" near the end of the font (note that there can
    // be intervening "\n", "\r\n", etc.), then search backward
    if (fontSize - binStart < 512) {
      return gFalse;
    }
    if (!memcmp(font + fontSize - 256, "0000", 4) ||
	!memcmp(font + fontSize - 255, "0000", 4) ||
	!memcmp(font + fontSize - 254, "0000", 4) ||
	!memcmp(font + fontSize - 253, "0000", 4) ||
	!memcmp(font + fontSize - 252, "0000", 4) ||
	!memcmp(font + fontSize - 251, "0000", 4)) {
      i = fontSize - 252;
      lastSpace = -1;
      while (i >= binStart) {
	if (font[i] == ' ' || font[i] == '\t' ||
	    font[i] == '\n' || font[i] == '\r') {
	  lastSpace = i;
	  --i;
	} else if (font[i] == '0') {
	  --i;
	} else {
	  break;
	}
      }
      if (lastSpace < 0) {
	return gFalse;
      }
      // check for the case where the newline/space is missing between
      // the binary section and the first set of 64 '0' chars
      if (lastSpace - binStart > 64 &&
	  !memcmp(font + lastSpace - 64,
		  "0000000000000000000000000000000000000000000000000000000000000000",
		  64)) {
	binLength = lastSpace - 64 - binStart;
      } else {
	binLength = lastSpace - binStart;
      }

    // couldn't find zeros after binary section -- assume they're
    // missing and the binary section extends to the end of the file
    } else {
      binLength = fontSize - binStart;
    }
  }

  binSection->append((char *)(font + binStart), binLength);

  return gTrue;
}

// Split a Type 1 font in PFB format into a text section and a binary
// section.
GBool PSOutputDev::splitType1PFB(Guchar *font, int fontSize,
				 GString *textSection, GString *binSection) {
  Guchar *p;
  int state, remain, len, n;

  // states:
  // 0: text section
  // 1: binary section
  // 2: trailer section
  // 3: eof

  state = 0;
  p = font;
  remain = fontSize;
  while (remain >= 2) {
    if (p[0] != 0x80) {
      return gFalse;
    }
    switch (state) {
    case 0:
      if (p[1] == 0x02) {
	state = 1;
      } else if (p[1] != 0x01) {
	return gFalse;
      }
      break;
    case 1:
      if (p[1] == 0x01) {
	state = 2;
      } else if (p[1] != 0x02) {
	return gFalse;
      }
      break;
    case 2:
      if (p[1] == 0x03) {
	state = 3;
      } else if (p[1] != 0x01) {
	return gFalse;
      }
      break;
    default: // shouldn't happen
      return gFalse;
    }
    if (state == 3) {
      break;
    }

    if (remain < 6) {
      break;
    }
    len = p[2] + (p[3] << 8) + (p[4] << 16) + (p[5] << 24);
    if (len < 0 || len > remain - 6) {
      return gFalse;
    }

    switch (state) {
    case 0:
      textSection->append((char *)(p + 6), len);
      break;
    case 1:
      binSection->append((char *)(p + 6), len);
      break;
    case 2:
      // we don't use the trailer
      break;
    default: // shouldn't happen
      return gFalse;
    }

    p += len + 6;
    remain -= len + 6;
  }

  if (state != 3) {
    return gFalse;
  }

  n = textSection->getLength();
  if (n >= 18 && !memcmp(textSection->getCString() + n - 18,
			 "currentfile eexec", 17)) {
    // remove the trailing whitespace
    textSection->del(n - 1, 1);
  } else if (n >= 17 && !memcmp(textSection->getCString() + n - 17,
				"currentfile eexec", 17)) {
    // missing whitespace at end -- leave as-is
  } else {
    return gFalse;
  }

  return gTrue;
}

// If <in> is ASCIIHex-encoded, decode it, delete <in>, and return the
// binary version.  Else return <in> unchanged.
GString *PSOutputDev::asciiHexDecodeType1EexecSection(GString *in) {
  GString *out;
  char c;
  Guchar byte;
  int state, i;

  out = new GString();
  state = 0;
  byte = 0;
  for (i = 0; i < in->getLength(); ++i) {
    c = in->getChar(i);
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
      continue;
    }
    if (c >= '0' && c <= '9') {
      byte = (Guchar)(byte + (c - '0'));
    } else if (c >= 'A' && c <= 'F') {
      byte = (Guchar)(byte + (c - 'A' + 10));
    } else if (c >= 'a' && c <= 'f') {
      byte = (Guchar)(byte + (c - 'a' + 10));
    } else {
      delete out;
      return in;
    }
    if (state == 0) {
      byte = (Guchar)(byte << 4);
      state = 1;
    } else {
      out->append((char)byte);
      state = 0;
      byte = 0;
    }
  }
  delete in;
  return out;
}

GBool PSOutputDev::fixType1EexecSection(GString *binSection, GString *out) {
  static char hexChars[17] = "0123456789abcdef";
  Guchar buf[16], buf2[16];
  Guchar byte;
  int r, i, j;

  // eexec-decode the binary section, keeping the last 16 bytes
  r = 55665;
  for (i = 0; i < binSection->getLength(); ++i) {
    byte = (Guchar)binSection->getChar(i);
    buf[i & 15] = byte ^ (Guchar)(r >> 8);
    r = ((r + byte) * 52845 + 22719) & 0xffff;
  }
  for (j = 0; j < 16; ++j) {
    buf2[j] = buf[(i + j) & 15];
  }

  // look for 'closefile'
  for (i = 0; i <= 16 - 9; ++i) {
    if (!memcmp(buf2 + i, "closefile", 9)) {
      break;
    }
  }
  if (i > 16 - 9) {
    return gFalse;
  }
  // three cases:
  // - short: missing space after "closefile" (i == 16 - 9)
  // - correct: exactly one space after "closefile" (i == 16 - 10)
  // - long: extra chars after "closefile" (i < 16 - 10)
  if (i == 16 - 9) {
    binSection->append((char)((Guchar)'\n' ^ (Guchar)(r >> 8)));
  } else if (i < 16 - 10) {
    binSection->del(binSection->getLength() - (16 - 10 - i), 16 - 10 - i);
  }
    
  // ASCIIHex encode
  for (i = 0; i < binSection->getLength(); i += 32) {
    for (j = 0; j < 32 && i+j < binSection->getLength(); ++j) {
      byte = (Guchar)binSection->getChar(i+j);
      out->append(hexChars[(byte >> 4) & 0x0f]);
      out->append(hexChars[byte & 0x0f]);
    }
    out->append('\n');
  }

  return gTrue;
}

// The Type 1 cleanup code failed -- assume it's a valid PFA-format
// font and copy it to the output.
GString *PSOutputDev::copyType1PFA(Guchar *font, int fontSize) {
  GString *out;

  error(errSyntaxWarning, -1, "Couldn't parse embedded Type 1 font");

  out = new GString((char *)font, fontSize);
  // append a newline to avoid problems where the original font
  // doesn't end with one
  out->append('\n');
  return out;
}

// The Type 1 cleanup code failed -- assume it's a valid PFB-format
// font, decode the PFB blocks, and copy them to the output.
GString *PSOutputDev::copyType1PFB(Guchar *font, int fontSize) {
  static char hexChars[17] = "0123456789abcdef";
  GString *out;
  Guchar *p;
  int remain, len, i, j;

  error(errSyntaxWarning, -1, "Couldn't parse embedded Type 1 (PFB) font");

  out = new GString();
  p = font;
  remain = fontSize;
  while (remain >= 6 &&
	 p[0] == 0x80 &&
	 (p[1] == 0x01 || p[1] == 0x02)) {
    len = p[2] + (p[3] << 8) + (p[4] << 16) + (p[5] << 24);
    if (len > remain - 6) {
      break;
    }
    if (p[1] == 0x01) {
      out->append((char *)(p + 6), len);
    } else {
      for (i = 0; i < len; i += 32) {
	for (j = 0; j < 32 && i+j < len; ++j) {
	  out->append(hexChars[(p[6+i+j] >> 4) & 0x0f]);
	  out->append(hexChars[p[6+i+j] & 0x0f]);
	}
	out->append('\n');
      }
    }
    p += len + 6;
    remain -= len + 6;
  }
  // append a newline to avoid problems where the original font
  // doesn't end with one
  out->append('\n');
  return out;
}

void PSOutputDev::renameType1Font(GString *font, GString *name) {
  char *p1, *p2;
  int i;

  if (!(p1 = strstr(font->getCString(), "\n/FontName")) &&
      !(p1 = strstr(font->getCString(), "\r/FontName"))) {
    return;
  }
  p1 += 10;
  while (*p1 == ' ' || *p1 == '\t' || *p1 == '\n' || *p1 == '\r') {
    ++p1;
  }
  if (*p1 != '/') {
    return;
  }
  ++p1;
  p2 = p1;
  while (*p2 && *p2 != ' ' && *p2 != '\t' && *p2 != '\n' && *p2 != '\r') {
    ++p2;
  }
  if (!*p2) {
    return;
  }
  i = (int)(p1 - font->getCString());
  font->del(i, (int)(p2 - p1));
  font->insert(i, name);
}

void PSOutputDev::setupDefaultFont() {
  writePS("/xpdf_default_font /Helvetica 1 1 ISOLatin1Encoding pdfMakeFont\n");
}

void PSOutputDev::setupImages(Dict *resDict) {
  Object xObjDict, xObj, xObjRef, subtypeObj, maskObj, maskRef;
  Ref imgID;
  int i, j;

  if (!(mode == psModeForm || inType3Char || preload)) {
    return;
  }

  resDict->lookup("XObject", &xObjDict);
  if (xObjDict.isDict()) {
    for (i = 0; i < xObjDict.dictGetLength(); ++i) {
      xObjDict.dictGetValNF(i, &xObjRef);
      xObjDict.dictGetVal(i, &xObj);
      if (xObj.isStream()) {
	xObj.streamGetDict()->lookup("Subtype", &subtypeObj);
	if (subtypeObj.isName("Image")) {
	  if (xObjRef.isRef()) {
	    imgID = xObjRef.getRef();
	    for (j = 0; j < imgIDLen; ++j) {
	      if (imgIDs[j].num == imgID.num && imgIDs[j].gen == imgID.gen) {
		break;
	      }
	    }
	    if (j == imgIDLen) {
	      if (imgIDLen >= imgIDSize) {
		if (imgIDSize == 0) {
		  imgIDSize = 64;
		} else {
		  imgIDSize *= 2;
		}
		imgIDs = (Ref *)greallocn(imgIDs, imgIDSize, sizeof(Ref));
	      }
	      imgIDs[imgIDLen++] = imgID;
	      setupImage(imgID, xObj.getStream(), gFalse, NULL);
	      if (level >= psLevel3) {
		xObj.streamGetDict()->lookup("Mask", &maskObj);
		if (maskObj.isStream()) {
		  setupImage(imgID, maskObj.getStream(), gTrue, NULL);
		} else if (level == psLevel3Gray && maskObj.isArray()) {
		  setupImage(imgID, xObj.getStream(), gFalse,
			     maskObj.getArray());
		}
	        maskObj.free();
	      }
	    }
	  } else {
	    error(errSyntaxError, -1,
		  "Image in resource dict is not an indirect reference");
	  }
	}
	subtypeObj.free();
      }
      xObj.free();
      xObjRef.free();
    }
  }
  xObjDict.free();
}

void PSOutputDev::setupImage(Ref id, Stream *str, GBool mask,
			     Array *colorKeyMask) {
  StreamColorSpaceMode csMode;
  GfxColorSpace *colorSpace;
  GfxImageColorMap *colorMap;
  int maskColors[2*gfxColorMaxComps];
  Object obj1;
  GBool imageMask, useLZW, useRLE, useCompressed, useASCIIHex;
  GString *s;
  int c, width, height, bits, size, line, col, i;

  // check for mask
  str->getDict()->lookup("ImageMask", &obj1);
  if (obj1.isBool()) {
    imageMask = obj1.getBool();
  } else {
    imageMask = gFalse;
  }
  obj1.free();

  // get image size
  str->getDict()->lookup("Width", &obj1);
  if (!obj1.isInt() || obj1.getInt() <= 0) {
    error(errSyntaxError, -1, "Invalid Width in image");
    obj1.free();
    return;
  }
  width = obj1.getInt();
  obj1.free();
  str->getDict()->lookup("Height", &obj1);
  if (!obj1.isInt() || obj1.getInt() <= 0) {
    error(errSyntaxError, -1, "Invalid Height in image");
    obj1.free();
    return;
  }
  height = obj1.getInt();
  obj1.free();

  // build the color map
  if (mask || imageMask) {
    colorMap = NULL;
  } else {
    bits = 0;
    csMode = streamCSNone;
    str->getImageParams(&bits, &csMode);
    if (bits == 0) {
      str->getDict()->lookup("BitsPerComponent", &obj1);
      if (!obj1.isInt()) {
	error(errSyntaxError, -1, "Invalid BitsPerComponent in image");
	obj1.free();
	return;
      }
      bits = obj1.getInt();
      obj1.free();
    }
    str->getDict()->lookup("ColorSpace", &obj1);
    if (!obj1.isNull()) {
      colorSpace = GfxColorSpace::parse(&obj1
					);
    } else if (csMode == streamCSDeviceGray) {
      colorSpace = GfxColorSpace::create(csDeviceGray);
    } else if (csMode == streamCSDeviceRGB) {
      colorSpace = GfxColorSpace::create(csDeviceRGB);
    } else if (csMode == streamCSDeviceCMYK) {
      colorSpace = GfxColorSpace::create(csDeviceCMYK);
    } else {
      colorSpace = NULL;
    }
    obj1.free();
    if (!colorSpace) {
      error(errSyntaxError, -1, "Invalid ColorSpace in image");
      return;
    }
    str->getDict()->lookup("Decode", &obj1);
    colorMap = new GfxImageColorMap(bits, &obj1, colorSpace);
    obj1.free();
  }

  // filters
  if (level < psLevel2) {
    useLZW = useRLE = gFalse;
    useCompressed = gFalse;
    useASCIIHex = gTrue;
  } else {
    if (colorKeyMask) {
      if (globalParams->getPSUncompressPreloadedImages()) {
	useLZW = useRLE = gFalse;
      } else if (globalParams->getPSLZW()) {
	useLZW = gTrue;
	useRLE = gFalse;
      } else {
	useRLE = gTrue;
	useLZW = gFalse;
      }
      useCompressed = gFalse;
    } else if (colorMap &&
	       (colorMap->getColorSpace()->getMode() == csDeviceN ||
		level == psLevel2Gray || level == psLevel3Gray)) {
      if (globalParams->getPSLZW()) {
	useLZW = gTrue;
	useRLE = gFalse;
      } else {
	useRLE = gTrue;
	useLZW = gFalse;
      }
      useCompressed = gFalse;
    } else if (globalParams->getPSUncompressPreloadedImages()) {
      useLZW = useRLE = gFalse;
      useCompressed = gFalse;
    } else {
      s = str->getPSFilter(level < psLevel3 ? 2 : 3, "");
      if (s) {
	useLZW = useRLE = gFalse;
	useCompressed = gTrue;
	delete s;
      } else {
	if (globalParams->getPSLZW()) {
	  useLZW = gTrue;
	  useRLE = gFalse;
	} else {
	  useRLE = gTrue;
	  useLZW = gFalse;
	}
	useCompressed = gFalse;
      }
    }
    useASCIIHex = globalParams->getPSASCIIHex();
  }
  if (useCompressed) {
    str = str->getUndecodedStream();
  }
  if (colorKeyMask) {
    memset(maskColors, 0, sizeof(maskColors));
    for (i = 0; i < colorKeyMask->getLength() && i < 2*gfxColorMaxComps; ++i) {
      colorKeyMask->get(i, &obj1);
      if (obj1.isInt()) {
	maskColors[i] = obj1.getInt();
      }
      obj1.free();
    }
    str = new ColorKeyToMaskEncoder(str, width, height, colorMap, maskColors);
  } else if (colorMap && (level == psLevel2Gray || level == psLevel3Gray)) {
    str = new GrayRecoder(str, width, height, colorMap);
  } else if (colorMap && colorMap->getColorSpace()->getMode() == csDeviceN) {
    str = new DeviceNRecoder(str, width, height, colorMap);
  }
  if (useLZW) {
    str = new LZWEncoder(str);
  } else if (useRLE) {
    str = new RunLengthEncoder(str);
  }
  if (useASCIIHex) {
    str = new ASCIIHexEncoder(str);
  } else {
    str = new ASCII85Encoder(str);
  }

  // compute image data size
  str->reset();
  col = size = 0;
  do {
    do {
      c = str->getChar();
    } while (c == '\n' || c == '\r');
    if (c == (useASCIIHex ? '>' : '~') || c == EOF) {
      break;
    }
    if (c == 'z') {
      ++col;
    } else {
      ++col;
      for (i = 1; i <= (useASCIIHex ? 1 : 4); ++i) {
	do {
	  c = str->getChar();
	} while (c == '\n' || c == '\r');
	if (c == (useASCIIHex ? '>' : '~') || c == EOF) {
	  break;
	}
	++col;
      }
    }
    if (col > 225) {
      ++size;
      col = 0;
    }
  } while (c != (useASCIIHex ? '>' : '~') && c != EOF);
  // add one entry for the final line of data; add another entry
  // because the LZWDecode/RunLengthDecode filter may read past the end
  ++size;
  if (useLZW || useRLE) {
    ++size;
  }
  writePSFmt("{0:d} array dup /{1:s}Data_{2:d}_{3:d} exch def\n",
	     size, (mask || colorKeyMask) ? "Mask" : "Im", id.num, id.gen);
  str->close();

  // write the data into the array
  str->reset();
  line = col = 0;
  writePS((char *)(useASCIIHex ? "dup 0 <" : "dup 0 <~"));
  do {
    do {
      c = str->getChar();
    } while (c == '\n' || c == '\r');
    if (c == (useASCIIHex ? '>' : '~') || c == EOF) {
      break;
    }
    if (c == 'z') {
      writePSChar((char)c);
      ++col;
    } else {
      writePSChar((char)c);
      ++col;
      for (i = 1; i <= (useASCIIHex ? 1 : 4); ++i) {
	do {
	  c = str->getChar();
	} while (c == '\n' || c == '\r');
	if (c == (useASCIIHex ? '>' : '~') || c == EOF) {
	  break;
	}
	writePSChar((char)c);
	++col;
      }
    }
    // each line is: "dup nnnnn <~...data...~> put<eol>"
    // so max data length = 255 - 20 = 235
    // chunks are 1 or 4 bytes each, so we have to stop at 232
    // but make it 225 just to be safe
    if (col > 225) {
      writePS((char *)(useASCIIHex ? "> put\n" : "~> put\n"));
      ++line;
      writePSFmt((char *)(useASCIIHex ? "dup {0:d} <" : "dup {0:d} <~"), line);
      col = 0;
    }
  } while (c != (useASCIIHex ? '>' : '~') && c != EOF);
  writePS((char *)(useASCIIHex ? "> put\n" : "~> put\n"));
  if (useLZW || useRLE) {
    ++line;
    writePSFmt("{0:d} <> put\n", line);
  } else {
    writePS("pop\n");
  }
  str->close();

  delete str;

  if (colorMap) {
    delete colorMap;
  }
}

void PSOutputDev::setupForms(Dict *resDict) {
  Object xObjDict, xObj, xObjRef, subtypeObj;
  int i;

  if (!preload) {
    return;
  }

  resDict->lookup("XObject", &xObjDict);
  if (xObjDict.isDict()) {
    for (i = 0; i < xObjDict.dictGetLength(); ++i) {
      xObjDict.dictGetValNF(i, &xObjRef);
      xObjDict.dictGetVal(i, &xObj);
      if (xObj.isStream()) {
	xObj.streamGetDict()->lookup("Subtype", &subtypeObj);
	if (subtypeObj.isName("Form")) {
	  if (xObjRef.isRef()) {
	    setupForm(&xObjRef, &xObj);
	  } else {
	    error(errSyntaxError, -1,
		  "Form in resource dict is not an indirect reference");
	  }
	}
	subtypeObj.free();
      }
      xObj.free();
      xObjRef.free();
    }
  }
  xObjDict.free();
}

void PSOutputDev::setupForm(Object *strRef, Object *strObj) {
  Dict *dict, *resDict;
  Object matrixObj, bboxObj, resObj, obj1;
  double m[6], bbox[4];
  PDFRectangle box;
  Gfx *gfx;
  int i;

  // check if form is already defined
  for (i = 0; i < formIDLen; ++i) {
    if (formIDs[i].num == strRef->getRefNum() &&
	formIDs[i].gen == strRef->getRefGen()) {
      return;
    }
  }

  // add entry to formIDs list
  if (formIDLen >= formIDSize) {
    if (formIDSize == 0) {
      formIDSize = 64;
    } else {
      formIDSize *= 2;
    }
    formIDs = (Ref *)greallocn(formIDs, formIDSize, sizeof(Ref));
  }
  formIDs[formIDLen++] = strRef->getRef();

  dict = strObj->streamGetDict();

  // get bounding box
  dict->lookup("BBox", &bboxObj);
  if (!bboxObj.isArray()) {
    bboxObj.free();
    error(errSyntaxError, -1, "Bad form bounding box");
    return;
  }
  for (i = 0; i < 4; ++i) {
    bboxObj.arrayGet(i, &obj1);
    bbox[i] = obj1.getNum();
    obj1.free();
  }
  bboxObj.free();

  // get matrix
  dict->lookup("Matrix", &matrixObj);
  if (matrixObj.isArray()) {
    for (i = 0; i < 6; ++i) {
      matrixObj.arrayGet(i, &obj1);
      m[i] = obj1.getNum();
      obj1.free();
    }
  } else {
    m[0] = 1; m[1] = 0;
    m[2] = 0; m[3] = 1;
    m[4] = 0; m[5] = 0;
  }
  matrixObj.free();

  // get resources
  dict->lookup("Resources", &resObj);
  resDict = resObj.isDict() ? resObj.getDict() : (Dict *)NULL;

  writePSFmt("/f_{0:d}_{1:d} {{\n", strRef->getRefNum(), strRef->getRefGen());
  writePS("q\n");
  writePSFmt("[{0:.6g} {1:.6g} {2:.6g} {3:.6g} {4:.6g} {5:.6g}] cm\n",
	     m[0], m[1], m[2], m[3], m[4], m[5]);

  box.x1 = bbox[0];
  box.y1 = bbox[1];
  box.x2 = bbox[2];
  box.y2 = bbox[3];
  gfx = new Gfx(doc, this, resDict, &box, &box);
  gfx->display(strRef);
  delete gfx;

  writePS("Q\n");
  writePS("} def\n");

  resObj.free();
}

GBool PSOutputDev::checkPageSlice(Page *page, double hDPI, double vDPI,
				  int rotateA, GBool useMediaBox, GBool crop,
				  int sliceX, int sliceY,
				  int sliceW, int sliceH,
				  GBool printing,
				  GBool (*abortCheckCbk)(void *data),
				  void *abortCheckCbkData) {
  int pg;
#if HAVE_SPLASH
  GBool mono;
  GBool useLZW;
  double dpi;
  SplashOutputDev *splashOut;
  SplashColor paperColor;
  PDFRectangle box;
  GfxState *state;
  SplashBitmap *bitmap;
  Stream *str0, *str;
  Object obj;
  Guchar *p;
  Guchar col[4];
  char buf[4096];
  double userUnit, hDPI2, vDPI2;
  double m0, m1, m2, m3, m4, m5;
  int nStripes, stripeH, stripeY;
  int w, h, x, y, comp, i, n;
#endif

  pg = page->getNum();
  if (!(pg >= firstPage && pg <= lastPage &&
	rasterizePage[pg - firstPage])) {
    return gTrue;
  }

#if HAVE_SPLASH
  // get the rasterization parameters
  dpi = globalParams->getPSRasterResolution();
  mono = globalParams->getPSRasterMono() ||
         level == psLevel1 ||
         level == psLevel2Gray ||
         level == psLevel3Gray;
  useLZW = globalParams->getPSLZW();

  // get the UserUnit
  if (honorUserUnit) {
    userUnit = page->getUserUnit();
  } else {
    userUnit = 1;
  }

  // start the PS page
  page->makeBox(userUnit * dpi, userUnit * dpi, rotateA, useMediaBox, gFalse,
		sliceX, sliceY, sliceW, sliceH, &box, &crop);
  rotateA += page->getRotate();
  if (rotateA >= 360) {
    rotateA -= 360;
  } else if (rotateA < 0) {
    rotateA += 360;
  }
  state = new GfxState(dpi, dpi, &box, rotateA, gFalse);
  startPage(page->getNum(), state);
  delete state;

  // set up the SplashOutputDev
  if (mono) {
    paperColor[0] = 0xff;
    splashOut = new SplashOutputDev(splashModeMono8, 1, gFalse,
				    paperColor, gFalse,
				    globalParams->getAntialiasPrinting());
#if SPLASH_CMYK
  } else if (level == psLevel1Sep) {
    paperColor[0] = paperColor[1] = paperColor[2] = paperColor[3] = 0;
    splashOut = new SplashOutputDev(splashModeCMYK8, 1, gFalse,
				    paperColor, gFalse,
				    globalParams->getAntialiasPrinting());
#endif
  } else {
    paperColor[0] = paperColor[1] = paperColor[2] = 0xff;
    splashOut = new SplashOutputDev(splashModeRGB8, 1, gFalse,
				    paperColor, gFalse,
				    globalParams->getAntialiasPrinting());
  }
  splashOut->startDoc(xref);

  // break the page into stripes
  // NB: startPage() has already multiplied xScale and yScale by UserUnit
  hDPI2 = xScale * dpi;
  vDPI2 = yScale * dpi;
  if (sliceW < 0 || sliceH < 0) {
    if (useMediaBox) {
      box = *page->getMediaBox();
    } else {
      box = *page->getCropBox();
    }
    sliceX = sliceY = 0;
    sliceW = (int)((box.x2 - box.x1) * hDPI2 / 72.0);
    sliceH = (int)((box.y2 - box.y1) * vDPI2 / 72.0);
  }
  nStripes = (int)ceil(((double)sliceW * (double)sliceH) /
		       (double)globalParams->getPSRasterSliceSize());
  stripeH = (sliceH + nStripes - 1) / nStripes;

  // render the stripes
  for (stripeY = sliceY; stripeY < sliceH; stripeY += stripeH) {

    // rasterize a stripe
    page->makeBox(hDPI2, vDPI2, 0, useMediaBox, gFalse,
		  sliceX, stripeY, sliceW, stripeH, &box, &crop);
    m0 = box.x2 - box.x1;
    m1 = 0;
    m2 = 0;
    m3 = box.y2 - box.y1;
    m4 = box.x1;
    m5 = box.y1;
    page->displaySlice(splashOut, hDPI2, vDPI2,
		       (360 - page->getRotate()) % 360, useMediaBox, crop,
		       sliceX, stripeY, sliceW, stripeH,
		       printing, abortCheckCbk, abortCheckCbkData);

    // draw the rasterized image
    bitmap = splashOut->getBitmap();
    w = bitmap->getWidth();
    h = bitmap->getHeight();
    writePS("gsave\n");
    writePSFmt("[{0:.6g} {1:.6g} {2:.6g} {3:.6g} {4:.6g} {5:.6g}] concat\n",
	       m0, m1, m2, m3, m4, m5);
    switch (level) {
    case psLevel1:
      writePSFmt("{0:d} {1:d} 8 [{2:d} 0 0 {3:d} 0 {4:d}] pdfIm1\n",
		 w, h, w, -h, h);
      p = bitmap->getDataPtr() + (h - 1) * bitmap->getRowSize();
      i = 0;
      for (y = 0; y < h; ++y) {
	for (x = 0; x < w; ++x) {
	  writePSFmt("{0:02x}", *p++);
	  if (++i == 32) {
	    writePSChar('\n');
	    i = 0;
	  }
	}
      }
      if (i != 0) {
	writePSChar('\n');
      }
      break;
    case psLevel1Sep:
      writePSFmt("{0:d} {1:d} 8 [{2:d} 0 0 {3:d} 0 {4:d}] pdfIm1Sep\n",
		 w, h, w, -h, h);
      p = bitmap->getDataPtr() + (h - 1) * bitmap->getRowSize();
      i = 0;
      col[0] = col[1] = col[2] = col[3] = 0;
      for (y = 0; y < h; ++y) {
	for (comp = 0; comp < 4; ++comp) {
	  for (x = 0; x < w; ++x) {
	    writePSFmt("{0:02x}", p[4*x + comp]);
	    col[comp] |= p[4*x + comp];
	    if (++i == 32) {
	      writePSChar('\n');
	      i = 0;
	    }
	  }
	}
	p -= bitmap->getRowSize();
      }
      if (i != 0) {
	writePSChar('\n');
      }
      if (col[0]) {
	processColors |= psProcessCyan;
      }
      if (col[1]) {
	processColors |= psProcessMagenta;
      }
      if (col[2]) {
	processColors |= psProcessYellow;
      }
      if (col[3]) {
	processColors |= psProcessBlack;
      }
      break;
    case psLevel2:
    case psLevel2Gray:
    case psLevel2Sep:
    case psLevel3:
    case psLevel3Gray:
    case psLevel3Sep:
      if (mono) {
	writePS("/DeviceGray setcolorspace\n");
      } else {
	writePS("/DeviceRGB setcolorspace\n");
      }
      writePS("<<\n  /ImageType 1\n");
      writePSFmt("  /Width {0:d}\n", bitmap->getWidth());
      writePSFmt("  /Height {0:d}\n", bitmap->getHeight());
      writePSFmt("  /ImageMatrix [{0:d} 0 0 {1:d} 0 {2:d}]\n", w, -h, h);
      writePS("  /BitsPerComponent 8\n");
      if (mono) {
	writePS("  /Decode [0 1]\n");
      } else {
	writePS("  /Decode [0 1 0 1 0 1]\n");
      }
      writePS("  /DataSource currentfile\n");
      if (globalParams->getPSASCIIHex()) {
	writePS("    /ASCIIHexDecode filter\n");
      } else {
	writePS("    /ASCII85Decode filter\n");
      }
      if (useLZW) {
	writePS("    /LZWDecode filter\n");
      } else {
	writePS("    /RunLengthDecode filter\n");
      }
      writePS(">>\n");
      writePS("image\n");
      obj.initNull();
      p = bitmap->getDataPtr() + (h - 1) * bitmap->getRowSize();
      str0 = new MemStream((char *)p, 0, w * h * (mono ? 1 : 3), &obj);
      if (useLZW) {
	str = new LZWEncoder(str0);
      } else {
	str = new RunLengthEncoder(str0);
      }
      if (globalParams->getPSASCIIHex()) {
	str = new ASCIIHexEncoder(str);
      } else {
	str = new ASCII85Encoder(str);
      }
      str->reset();
      while ((n = str->getBlock(buf, sizeof(buf))) > 0) {
	writePSBlock(buf, n);
      }
      str->close();
      delete str;
      delete str0;
      writePSChar('\n');
      processColors |= mono ? psProcessBlack : psProcessCMYK;
      break;
    }
    writePS("grestore\n");
  }

  delete splashOut;

  // finish the PS page
  endPage();

  return gFalse;

#else // HAVE_SPLASH

  error(errSyntaxWarning, -1,
	"PDF page uses transparency and PSOutputDev was built without"
	" the Splash rasterizer - output may not be correct");
  return gTrue;
#endif // HAVE_SPLASH
}

void PSOutputDev::startPage(int pageNum, GfxState *state) {
  Page *page;
  double userUnit;
  int x1, y1, x2, y2, width, height, t;
  int imgWidth, imgHeight, imgWidth2, imgHeight2;
  GBool landscape;
  GString *s;

  page = doc->getCatalog()->getPage(pageNum);
  if (honorUserUnit) {
    userUnit = page->getUserUnit();
  } else {
    userUnit = 1;
  }

  if (mode == psModePS) {
    writePSFmt("%%Page: {0:d} {1:d}\n", pageNum, seqPage);
    if (paperMatch) {
      imgLLX = imgLLY = 0;
      if (globalParams->getPSUseCropBoxAsPage()) {
	imgURX = (int)ceil(page->getCropWidth() * userUnit);
	imgURY = (int)ceil(page->getCropHeight() * userUnit);
      } else {
	imgURX = (int)ceil(page->getMediaWidth() * userUnit);
	imgURY = (int)ceil(page->getMediaHeight() * userUnit);
      }
      if (state->getRotate() == 90 || state->getRotate() == 270) {
	t = imgURX;
	imgURX = imgURY;
	imgURY = t;
      }
      writePSFmt("%%PageMedia: {0:d}x{1:d}\n", imgURX, imgURY);
      writePSFmt("%%PageBoundingBox: 0 0 {0:d} {1:d}\n", imgURX, imgURY);
    }
    writePS("%%BeginPageSetup\n");
  }
  if (mode != psModeForm) {
    writePS("xpdf begin\n");
  }

  // set up paper size for paper=match mode
  // NB: this must be done *before* the saveState() for overlays.
  if (mode == psModePS && paperMatch) {
    writePSFmt("{0:d} {1:d} pdfSetupPaper\n", imgURX, imgURY);
  }

  // underlays
  if (underlayCbk) {
    (*underlayCbk)(this, underlayCbkData);
  }
  if (overlayCbk) {
    saveState(NULL);
  }

  switch (mode) {

  case psModePS:
    // rotate, translate, and scale page
    imgWidth = imgURX - imgLLX;
    imgHeight = imgURY - imgLLY;
    x1 = (int)floor(state->getX1());
    y1 = (int)floor(state->getY1());
    x2 = (int)ceil(state->getX2());
    y2 = (int)ceil(state->getY2());
    width = x2 - x1;
    height = y2 - y1;
    tx = ty = 0;
    // rotation and portrait/landscape mode
    if (paperMatch) {
      rotate = (360 - state->getRotate()) % 360;
      landscape = gFalse;
    } else if (rotate0 >= 0) {
      rotate = (360 - rotate0) % 360;
      landscape = gFalse;
    } else {
      rotate = (360 - state->getRotate()) % 360;
      if (rotate == 0 || rotate == 180) {
	if ((width < height && imgWidth > imgHeight && height > imgHeight) ||
	    (width > height && imgWidth < imgHeight && width > imgWidth)) {
	  rotate += 90;
	  landscape = gTrue;
	} else {
	  landscape = gFalse;
	}
      } else { // rotate == 90 || rotate == 270
	if ((height < width && imgWidth > imgHeight && width > imgHeight) ||
	    (height > width && imgWidth < imgHeight && height > imgWidth)) {
	  rotate = 270 - rotate;
	  landscape = gTrue;
	} else {
	  landscape = gFalse;
	}
      }
    }
    writePSFmt("%%PageOrientation: {0:s}\n",
	       landscape ? "Landscape" : "Portrait");
    writePS("pdfStartPage\n");
    if (rotate == 0) {
      imgWidth2 = imgWidth;
      imgHeight2 = imgHeight;
    } else if (rotate == 90) {
      writePS("90 rotate\n");
      ty = -imgWidth;
      imgWidth2 = imgHeight;
      imgHeight2 = imgWidth;
    } else if (rotate == 180) {
      writePS("180 rotate\n");
      imgWidth2 = imgWidth;
      imgHeight2 = imgHeight;
      tx = -imgWidth;
      ty = -imgHeight;
    } else { // rotate == 270
      writePS("270 rotate\n");
      tx = -imgHeight;
      imgWidth2 = imgHeight;
      imgHeight2 = imgWidth;
    }
    // shrink or expand
    if (xScale0 > 0 && yScale0 > 0) {
      xScale = xScale0 * userUnit;
      yScale = yScale0 * userUnit;
    } else if ((globalParams->getPSShrinkLarger() &&
		(width * userUnit > imgWidth2 ||
		 height * userUnit > imgHeight2)) ||
	       (globalParams->getPSExpandSmaller() &&
		(width * userUnit < imgWidth2 &&
		 height * userUnit < imgHeight2))) {
      xScale = (double)imgWidth2 / (double)width;
      yScale = (double)imgHeight2 / (double)height;
      if (yScale < xScale) {
	xScale = yScale;
      } else {
	yScale = xScale;
      }
    } else {
      xScale = yScale = userUnit;
    }
    // deal with odd bounding boxes or clipping
    if (clipLLX0 < clipURX0 && clipLLY0 < clipURY0) {
      tx -= xScale * clipLLX0;
      ty -= yScale * clipLLY0;
    } else {
      tx -= xScale * x1;
      ty -= yScale * y1;
    }
    // center
    if (tx0 >= 0 && ty0 >= 0) {
      tx += (rotate == 0 || rotate == 180) ? tx0 : ty0;
      ty += (rotate == 0 || rotate == 180) ? ty0 : -tx0;
    } else if (globalParams->getPSCenter()) {
      if (clipLLX0 < clipURX0 && clipLLY0 < clipURY0) {
	tx += (imgWidth2 - xScale * (clipURX0 - clipLLX0)) / 2;
	ty += (imgHeight2 - yScale * (clipURY0 - clipLLY0)) / 2;
      } else {
	tx += (imgWidth2 - xScale * width) / 2;
	ty += (imgHeight2 - yScale * height) / 2;
      }
    }
    tx += (rotate == 0 || rotate == 180) ? imgLLX : imgLLY;
    ty += (rotate == 0 || rotate == 180) ? imgLLY : -imgLLX;
    if (tx != 0 || ty != 0) {
      writePSFmt("{0:.6g} {1:.6g} translate\n", tx, ty);
    }
    if (xScale != 1 || yScale != 1) {
      writePSFmt("{0:.4f} {1:.4f} scale\n", xScale, yScale);
    }
    if (clipLLX0 < clipURX0 && clipLLY0 < clipURY0) {
      writePSFmt("{0:.6g} {1:.6g} {2:.6g} {3:.6g} re W\n",
		 clipLLX0, clipLLY0, clipURX0 - clipLLX0, clipURY0 - clipLLY0);
    } else {
      writePSFmt("{0:d} {1:d} {2:d} {3:d} re W\n", x1, y1, x2 - x1, y2 - y1);
    }

    ++seqPage;
    break;

  case psModeEPS:
    writePS("pdfStartPage\n");
    tx = ty = 0;
    rotate = (360 - state->getRotate()) % 360;
    if (rotate == 0) {
    } else if (rotate == 90) {
      writePS("90 rotate\n");
      tx = -epsX1;
      ty = -epsY2;
    } else if (rotate == 180) {
      writePS("180 rotate\n");
      tx = -(epsX1 + epsX2);
      ty = -(epsY1 + epsY2);
    } else { // rotate == 270
      writePS("270 rotate\n");
      tx = -epsX2;
      ty = -epsY1;
    }
    if (tx != 0 || ty != 0) {
      writePSFmt("{0:.6g} {1:.6g} translate\n", tx, ty);
    }
    xScale = yScale = 1;
    break;

  case psModeForm:
    writePS("/PaintProc {\n");
    writePS("begin xpdf begin\n");
    writePS("pdfStartPage\n");
    tx = ty = 0;
    xScale = yScale = 1;
    rotate = 0;
    break;
  }

  if (level == psLevel2Gray || level == psLevel3Gray) {
    writePS("/DeviceGray setcolorspace\n");
  }

  if (customCodeCbk) {
    if ((s = (*customCodeCbk)(this, psOutCustomPageSetup, pageNum,
			      customCodeCbkData))) {
      writePS(s->getCString());
      delete s;
    }
  }

  if (mode == psModePS) {
    writePS("%%EndPageSetup\n");
  }

  noStateChanges = gFalse;
}

void PSOutputDev::endPage() {
  if (overlayCbk) {
    restoreState(NULL);
    (*overlayCbk)(this, overlayCbkData);
  }

  if (mode == psModeForm) {
    writePS("pdfEndPage\n");
    writePS("end end\n");
    writePS("} def\n");
    writePS("end end\n");
  } else {
    if (!manualCtrl) {
      writePS("showpage\n");
    }
    writePS("%%PageTrailer\n");
    writePageTrailer();
    writePS("end\n");
  }
}

void PSOutputDev::saveState(GfxState *state) {
  // The noStateChanges and saveStack fields are used to implement an
  // optimization to reduce gsave/grestore nesting.  The idea is to
  // look for sequences like this:
  //   q  q AAA Q BBB Q     (where AAA and BBB are sequences of operations)
  // and transform them to:
  //   q AAA Q q BBB Q
  if (noStateChanges) {
    // any non-NULL pointer will work here
    saveStack->append(this);
  } else {
    saveStack->append((PSOutputDev *)NULL);
    writePS("q\n");
    noStateChanges = gTrue;
  }
}

void PSOutputDev::restoreState(GfxState *state) {
  if (saveStack->getLength()) {
    writePS("Q\n");
    if (saveStack->del(saveStack->getLength() - 1)) {
      writePS("q\n");
      noStateChanges = gTrue;
    } else {
      noStateChanges = gFalse;
    }
  }
}

void PSOutputDev::updateCTM(GfxState *state, double m11, double m12,
			    double m21, double m22, double m31, double m32) {
  if (m11 == 1 && m12 == 0 && m21 == 0 && m22 == 1 && m31 == 0 && m32 == 0) {
    return;
  }
  if (fabs(m11 * m22 - m12 * m21) < 1e-10) {
    // avoid a singular (or close-to-singular) matrix
    writePSFmt("[0.00001 0 0 0.00001 {0:.6g} {1:.6g}] cm\n", m31, m32);
  } else {
    writePSFmt("[{0:.6g} {1:.6g} {2:.6g} {3:.6g} {4:.6g} {5:.6g}] cm\n",
	       m11, m12, m21, m22, m31, m32);
  }
  noStateChanges = gFalse;
}

void PSOutputDev::updateLineDash(GfxState *state) {
  double *dash;
  double start;
  int length, i;

  state->getLineDash(&dash, &length, &start);
  writePS("[");
  for (i = 0; i < length; ++i) {
    writePSFmt("{0:.6g}{1:w}",
	       dash[i] < 0 ? 0 : dash[i],
	       (i == length-1) ? 0 : 1);
  }
  writePSFmt("] {0:.6g} d\n", start);
  noStateChanges = gFalse;
}

void PSOutputDev::updateFlatness(GfxState *state) {
  writePSFmt("{0:.4g} i\n", state->getFlatness());
  noStateChanges = gFalse;
}

void PSOutputDev::updateLineJoin(GfxState *state) {
  writePSFmt("{0:d} j\n", state->getLineJoin());
  noStateChanges = gFalse;
}

void PSOutputDev::updateLineCap(GfxState *state) {
  writePSFmt("{0:d} J\n", state->getLineCap());
  noStateChanges = gFalse;
}

void PSOutputDev::updateMiterLimit(GfxState *state) {
  writePSFmt("{0:.4g} M\n", state->getMiterLimit());
  noStateChanges = gFalse;
}

void PSOutputDev::updateLineWidth(GfxState *state) {
  writePSFmt("{0:.6g} w\n", state->getLineWidth());
  noStateChanges = gFalse;
}

void PSOutputDev::updateFillColorSpace(GfxState *state) {
  switch (level) {
  case psLevel1:
  case psLevel1Sep:
    break;
  case psLevel2:
  case psLevel3:
    if (state->getFillColorSpace()->getMode() != csPattern) {
      dumpColorSpaceL2(state, state->getFillColorSpace(),
		       gTrue, gFalse, gFalse);
      writePS(" cs\n");
      noStateChanges = gFalse;
    }
    break;
  case psLevel2Gray:
  case psLevel3Gray:
  case psLevel2Sep:
  case psLevel3Sep:
    break;
  }
}

void PSOutputDev::updateStrokeColorSpace(GfxState *state) {
  switch (level) {
  case psLevel1:
  case psLevel1Sep:
    break;
  case psLevel2:
  case psLevel3:
    if (state->getStrokeColorSpace()->getMode() != csPattern) {
      dumpColorSpaceL2(state, state->getStrokeColorSpace(),
		       gTrue, gFalse, gFalse);
      writePS(" CS\n");
      noStateChanges = gFalse;
    }
    break;
  case psLevel2Gray:
  case psLevel3Gray:
  case psLevel2Sep:
  case psLevel3Sep:
    break;
  }
}

void PSOutputDev::updateFillColor(GfxState *state) {
  GfxColor color;
  GfxColor *colorPtr;
  GfxGray gray;
  GfxCMYK cmyk;
  GfxSeparationColorSpace *sepCS;
  double c, m, y, k;
  int i;

  switch (level) {
  case psLevel1:
  case psLevel2Gray:
  case psLevel3Gray:
    state->getFillGray(&gray);
    writePSFmt("{0:.4g} g\n", colToDbl(gray));
    break;
  case psLevel1Sep:
    state->getFillCMYK(&cmyk);
    c = colToDbl(cmyk.c);
    m = colToDbl(cmyk.m);
    y = colToDbl(cmyk.y);
    k = colToDbl(cmyk.k);
    writePSFmt("{0:.4g} {1:.4g} {2:.4g} {3:.4g} k\n", c, m, y, k);
    addProcessColor(c, m, y, k);
    break;
  case psLevel2:
  case psLevel3:
    if (state->getFillColorSpace()->getMode() != csPattern) {
      colorPtr = state->getFillColor();
      writePS("[");
      for (i = 0; i < state->getFillColorSpace()->getNComps(); ++i) {
	if (i > 0) {
	  writePS(" ");
	}
	writePSFmt("{0:.4g}", colToDbl(colorPtr->c[i]));
      }
      writePS("] sc\n");
    }
    break;
  case psLevel2Sep:
  case psLevel3Sep:
    if (state->getFillColorSpace()->getMode() == csSeparation) {
      sepCS = (GfxSeparationColorSpace *)state->getFillColorSpace();
      color.c[0] = gfxColorComp1;
      sepCS->getCMYK(&color, &cmyk, state->getRenderingIntent());
      writePSFmt("{0:.4g} {1:.4g} {2:.4g} {3:.4g} {4:.4g} ({5:t}) ck\n",
		 colToDbl(state->getFillColor()->c[0]),
		 colToDbl(cmyk.c), colToDbl(cmyk.m),
		 colToDbl(cmyk.y), colToDbl(cmyk.k),
		 sepCS->getName());
      addCustomColor(state, sepCS);
    } else {
      state->getFillCMYK(&cmyk);
      c = colToDbl(cmyk.c);
      m = colToDbl(cmyk.m);
      y = colToDbl(cmyk.y);
      k = colToDbl(cmyk.k);
      writePSFmt("{0:.4g} {1:.4g} {2:.4g} {3:.4g} k\n", c, m, y, k);
      addProcessColor(c, m, y, k);
    }
    break;
  }
  t3Cacheable = gFalse;
  noStateChanges = gFalse;
}

void PSOutputDev::updateStrokeColor(GfxState *state) {
  GfxColor color;
  GfxColor *colorPtr;
  GfxGray gray;
  GfxCMYK cmyk;
  GfxSeparationColorSpace *sepCS;
  double c, m, y, k;
  int i;

  switch (level) {
  case psLevel1:
  case psLevel2Gray:
  case psLevel3Gray:
    state->getStrokeGray(&gray);
    writePSFmt("{0:.4g} G\n", colToDbl(gray));
    break;
  case psLevel1Sep:
    state->getStrokeCMYK(&cmyk);
    c = colToDbl(cmyk.c);
    m = colToDbl(cmyk.m);
    y = colToDbl(cmyk.y);
    k = colToDbl(cmyk.k);
    writePSFmt("{0:.4g} {1:.4g} {2:.4g} {3:.4g} K\n", c, m, y, k);
    addProcessColor(c, m, y, k);
    break;
  case psLevel2:
  case psLevel3:
    if (state->getStrokeColorSpace()->getMode() != csPattern) {
      colorPtr = state->getStrokeColor();
      writePS("[");
      for (i = 0; i < state->getStrokeColorSpace()->getNComps(); ++i) {
	if (i > 0) {
	  writePS(" ");
	}
	writePSFmt("{0:.4g}", colToDbl(colorPtr->c[i]));
      }
      writePS("] SC\n");
    }
    break;
  case psLevel2Sep:
  case psLevel3Sep:
    if (state->getStrokeColorSpace()->getMode() == csSeparation) {
      sepCS = (GfxSeparationColorSpace *)state->getStrokeColorSpace();
      color.c[0] = gfxColorComp1;
      sepCS->getCMYK(&color, &cmyk, state->getRenderingIntent());
      writePSFmt("{0:.4g} {1:.4g} {2:.4g} {3:.4g} {4:.4g} ({5:t}) CK\n",
		 colToDbl(state->getStrokeColor()->c[0]),
		 colToDbl(cmyk.c), colToDbl(cmyk.m),
		 colToDbl(cmyk.y), colToDbl(cmyk.k),
		 sepCS->getName());
      addCustomColor(state, sepCS);
    } else {
      state->getStrokeCMYK(&cmyk);
      c = colToDbl(cmyk.c);
      m = colToDbl(cmyk.m);
      y = colToDbl(cmyk.y);
      k = colToDbl(cmyk.k);
      writePSFmt("{0:.4g} {1:.4g} {2:.4g} {3:.4g} K\n", c, m, y, k);
      addProcessColor(c, m, y, k);
    }
    break;
  }
  t3Cacheable = gFalse;
  noStateChanges = gFalse;
}

void PSOutputDev::addProcessColor(double c, double m, double y, double k) {
  if (c > 0) {
    processColors |= psProcessCyan;
  }
  if (m > 0) {
    processColors |= psProcessMagenta;
  }
  if (y > 0) {
    processColors |= psProcessYellow;
  }
  if (k > 0) {
    processColors |= psProcessBlack;
  }
}

void PSOutputDev::addCustomColor(GfxState *state,
				 GfxSeparationColorSpace *sepCS) {
  PSOutCustomColor *cc;
  GfxColor color;
  GfxCMYK cmyk;

  for (cc = customColors; cc; cc = cc->next) {
    if (!cc->name->cmp(sepCS->getName())) {
      return;
    }
  }
  color.c[0] = gfxColorComp1;
  sepCS->getCMYK(&color, &cmyk, state->getRenderingIntent());
  cc = new PSOutCustomColor(colToDbl(cmyk.c), colToDbl(cmyk.m),
			    colToDbl(cmyk.y), colToDbl(cmyk.k),
			    sepCS->getName()->copy());
  cc->next = customColors;
  customColors = cc;
}

void PSOutputDev::addCustomColors(GfxState *state,
				  GfxDeviceNColorSpace *devnCS) {
  PSOutCustomColor *cc;
  GfxColor color;
  GfxCMYK cmyk;
  int i;

  for (i = 0; i < devnCS->getNComps(); ++i) {
    color.c[i] = 0;
  }
  for (i = 0; i < devnCS->getNComps(); ++i) {
    for (cc = customColors; cc; cc = cc->next) {
      if (!cc->name->cmp(devnCS->getColorantName(i))) {
	break;
      }
    }
    if (cc) {
      continue;
    }
    color.c[i] = gfxColorComp1;
    devnCS->getCMYK(&color, &cmyk, state->getRenderingIntent());
    color.c[i] = 0;
    cc = new PSOutCustomColor(colToDbl(cmyk.c), colToDbl(cmyk.m),
			      colToDbl(cmyk.y), colToDbl(cmyk.k),
			      devnCS->getColorantName(i)->copy());
    cc->next = customColors;
    customColors = cc;
  }
}

void PSOutputDev::updateFillOverprint(GfxState *state) {
  if (level == psLevel2 || level == psLevel2Sep ||
      level == psLevel3 || level == psLevel3Sep) {
    writePSFmt("{0:s} op\n", state->getFillOverprint() ? "true" : "false");
    noStateChanges = gFalse;
  }
}

void PSOutputDev::updateStrokeOverprint(GfxState *state) {
  if (level == psLevel2 || level == psLevel2Sep ||
      level == psLevel3 || level == psLevel3Sep) {
    writePSFmt("{0:s} OP\n", state->getStrokeOverprint() ? "true" : "false");
    noStateChanges = gFalse;
  }
}

void PSOutputDev::updateOverprintMode(GfxState *state) {
  if (level == psLevel3 || level == psLevel3Sep) {
    writePSFmt("{0:s} opm\n", state->getOverprintMode() ? "true" : "false");
    noStateChanges = gFalse;
  }
}

void PSOutputDev::updateTransfer(GfxState *state) {
  Function **funcs;
  int i;

  funcs = state->getTransfer();
  if (funcs[0] && funcs[1] && funcs[2] && funcs[3]) {
    if (level == psLevel2 || level == psLevel2Sep ||
	level == psLevel3 || level == psLevel3Sep) {
      for (i = 0; i < 4; ++i) {
	cvtFunction(funcs[i]);
      }
      writePS("setcolortransfer\n");
    } else {
      cvtFunction(funcs[3]);
      writePS("settransfer\n");
    }
  } else if (funcs[0]) {
    cvtFunction(funcs[0]);
    writePS("settransfer\n");
  } else {
    writePS("{} settransfer\n");
  }
  noStateChanges = gFalse;
}

void PSOutputDev::updateFont(GfxState *state) {
  if (state->getFont()) {
    if (state->getFont()->getTag() &&
	!state->getFont()->getTag()->cmp("xpdf_default_font")) {
      writePSFmt("/xpdf_default_font {0:.6g} Tf\n",
		 fabs(state->getFontSize()) < 0.0001 ? 0.0001
		                                     : state->getFontSize());
    } else {
      writePSFmt("/F{0:d}_{1:d} {2:.6g} Tf\n",
		 state->getFont()->getID()->num, state->getFont()->getID()->gen,
		 fabs(state->getFontSize()) < 0.0001 ? 0.0001
		                                     : state->getFontSize());
    }
    noStateChanges = gFalse;
  }
}

void PSOutputDev::updateTextMat(GfxState *state) {
  double *mat;

  mat = state->getTextMat();
  if (fabs(mat[0] * mat[3] - mat[1] * mat[2]) < 1e-10) {
    // avoid a singular (or close-to-singular) matrix
    writePSFmt("[0.00001 0 0 0.00001 {0:.6g} {1:.6g}] Tm\n", mat[4], mat[5]);
  } else {
    writePSFmt("[{0:.6g} {1:.6g} {2:.6g} {3:.6g} {4:.6g} {5:.6g}] Tm\n",
	       mat[0], mat[1], mat[2], mat[3], mat[4], mat[5]);
  }
  noStateChanges = gFalse;
}

void PSOutputDev::updateCharSpace(GfxState *state) {
  writePSFmt("{0:.6g} Tc\n", state->getCharSpace());
  noStateChanges = gFalse;
}

void PSOutputDev::updateRender(GfxState *state) {
  int rm;

  rm = state->getRender();
  writePSFmt("{0:d} Tr\n", rm);
  rm &= 3;
  if (rm != 0 && rm != 3) {
    t3Cacheable = gFalse;
  }
  noStateChanges = gFalse;
}

void PSOutputDev::updateRise(GfxState *state) {
  writePSFmt("{0:.6g} Ts\n", state->getRise());
  noStateChanges = gFalse;
}

void PSOutputDev::updateWordSpace(GfxState *state) {
  writePSFmt("{0:.6g} Tw\n", state->getWordSpace());
  noStateChanges = gFalse;
}

void PSOutputDev::updateHorizScaling(GfxState *state) {
  double h;

  h = state->getHorizScaling();
  if (fabs(h) < 0.01) {
    h = 0.01;
  }
  writePSFmt("{0:.6g} Tz\n", h);
  noStateChanges = gFalse;
}

void PSOutputDev::updateTextPos(GfxState *state) {
  writePSFmt("{0:.6g} {1:.6g} Td\n", state->getLineX(), state->getLineY());
  noStateChanges = gFalse;
}

void PSOutputDev::updateTextShift(GfxState *state, double shift) {
  if (state->getFont()->getWMode()) {
    writePSFmt("{0:.6g} TJmV\n", shift);
  } else {
    writePSFmt("{0:.6g} TJm\n", shift);
  }
  noStateChanges = gFalse;
}

void PSOutputDev::saveTextPos(GfxState *state) {
  writePS("currentpoint\n");
  noStateChanges = gFalse;
}

void PSOutputDev::restoreTextPos(GfxState *state) {
  writePS("m\n");
  noStateChanges = gFalse;
}

void PSOutputDev::stroke(GfxState *state) {
  doPath(state->getPath());
  if (inType3Char && t3FillColorOnly) {
    // if we're constructing a cacheable Type 3 glyph, we need to do
    // everything in the fill color
    writePS("Sf\n");
  } else {
    writePS("S\n");
  }
  noStateChanges = gFalse;
}

void PSOutputDev::fill(GfxState *state) {
  doPath(state->getPath());
  writePS("f\n");
  noStateChanges = gFalse;
}

void PSOutputDev::eoFill(GfxState *state) {
  doPath(state->getPath());
  writePS("f*\n");
  noStateChanges = gFalse;
}

void PSOutputDev::tilingPatternFill(GfxState *state, Gfx *gfx, Object *strRef,
				    int paintType, int tilingType,
				    Dict *resDict,
				    double *mat, double *bbox,
				    int x0, int y0, int x1, int y1,
				    double xStep, double yStep) {
  if (level <= psLevel1Sep) {
    tilingPatternFillL1(state, gfx, strRef, paintType, tilingType,
			resDict, mat, bbox, x0, y0, x1, y1, xStep, yStep);
  } else {
    tilingPatternFillL2(state, gfx, strRef, paintType, tilingType,
			resDict, mat, bbox, x0, y0, x1, y1, xStep, yStep);
  }
}

void PSOutputDev::tilingPatternFillL1(GfxState *state, Gfx *gfx,
				      Object *strRef,
				      int paintType, int tilingType,
				      Dict *resDict,
				      double *mat, double *bbox,
				      int x0, int y0, int x1, int y1,
				      double xStep, double yStep) {
  PDFRectangle box;
  Gfx *gfx2;

  // define a Type 3 font
  writePS("8 dict begin\n");
  writePS("/FontType 3 def\n");
  writePS("/FontMatrix [1 0 0 1 0 0] def\n");
  writePSFmt("/FontBBox [{0:.6g} {1:.6g} {2:.6g} {3:.6g}] def\n",
	     bbox[0], bbox[1], bbox[2], bbox[3]);
  writePS("/Encoding 256 array def\n");
  writePS("  0 1 255 { Encoding exch /.notdef put } for\n");
  writePS("  Encoding 120 /x put\n");
  writePS("/BuildGlyph {\n");
  writePS("  exch /CharProcs get exch\n");
  writePS("  2 copy known not { pop /.notdef } if\n");
  writePS("  get exec\n");
  writePS("} bind def\n");
  writePS("/BuildChar {\n");
  writePS("  1 index /Encoding get exch get\n");
  writePS("  1 index /BuildGlyph get exec\n");
  writePS("} bind def\n");
  writePS("/CharProcs 1 dict def\n");
  writePS("CharProcs begin\n");
  box.x1 = bbox[0];
  box.y1 = bbox[1];
  box.x2 = bbox[2];
  box.y2 = bbox[3];
  gfx2 = new Gfx(doc, this, resDict, &box, NULL);
  gfx2->takeContentStreamStack(gfx);
  writePS("/x {\n");
  if (paintType == 2) {
    writePSFmt("{0:.6g} 0 {1:.6g} {2:.6g} {3:.6g} {4:.6g} setcachedevice\n",
	       xStep, bbox[0], bbox[1], bbox[2], bbox[3]);
    t3FillColorOnly = gTrue;
  } else {
    if (x1 - 1 <= x0) {
      writePS("1 0 setcharwidth\n");
    } else {
      writePSFmt("{0:.6g} 0 setcharwidth\n", xStep);
    }
    t3FillColorOnly = gFalse;
  }
  inType3Char = gTrue;
  ++numTilingPatterns;
  gfx2->display(strRef);
  --numTilingPatterns;
  inType3Char = gFalse;
  writePS("} def\n");
  delete gfx2;
  writePS("end\n");
  writePS("currentdict end\n");
  writePSFmt("/xpdfTile{0:d} exch definefont pop\n", numTilingPatterns);

  // draw the tiles
  writePSFmt("/xpdfTile{0:d} findfont setfont\n", numTilingPatterns);
  writePS("fCol\n");
  writePSFmt("gsave [{0:.6g} {1:.6g} {2:.6g} {3:.6g} {4:.6g} {5:.6g}] concat\n",
	     mat[0], mat[1], mat[2], mat[3], mat[4], mat[5]);
  writePSFmt("{0:d} 1 {1:d} {{ {2:.6g} exch {3:.6g} mul m {4:d} 1 {5:d} {{ pop (x) show }} for }} for\n",
	     y0, y1 - 1, x0 * xStep, yStep, x0, x1 - 1);
  writePS("grestore\n");
  noStateChanges = gFalse;
}

void PSOutputDev::tilingPatternFillL2(GfxState *state, Gfx *gfx,
				      Object *strRef,
				      int paintType, int tilingType,
				      Dict *resDict,
				      double *mat, double *bbox,
				      int x0, int y0, int x1, int y1,
				      double xStep, double yStep) {
  PDFRectangle box;
  Gfx *gfx2;

  // switch to pattern space
  writePSFmt("gsave [{0:.6g} {1:.6g} {2:.6g} {3:.6g} {4:.6g} {5:.6g}] concat\n",
	     mat[0], mat[1], mat[2], mat[3], mat[4], mat[5]);

  // define a pattern
  writePSFmt("/xpdfTile{0:d}\n", numTilingPatterns);
  writePS("<<\n");
  writePS("  /PatternType 1\n");
  writePSFmt("  /PaintType {0:d}\n", paintType);
  writePSFmt("  /TilingType {0:d}\n", tilingType);
  writePSFmt("  /BBox [{0:.6g} {1:.6g} {2:.6g} {3:.6g}]\n",
	     bbox[0], bbox[1], bbox[2], bbox[3]);
  writePSFmt("  /XStep {0:.6g}\n", xStep);
  writePSFmt("  /YStep {0:.6g}\n", yStep);
  writePS("  /PaintProc {\n");
  writePS("    pop\n");
  box.x1 = bbox[0];
  box.y1 = bbox[1];
  box.x2 = bbox[2];
  box.y2 = bbox[3];
  gfx2 = new Gfx(doc, this, resDict, &box, NULL);
  gfx2->takeContentStreamStack(gfx);
  t3FillColorOnly = paintType == 2;
  inType3Char = gTrue;
  ++numTilingPatterns;
  gfx2->display(strRef);
  --numTilingPatterns;
  inType3Char = gFalse;
  delete gfx2;
  writePS("  }\n");
  writePS(">> matrix makepattern def\n");

  // set the pattern
  if (paintType == 2) {
    writePS("currentcolor ");
  }
  writePSFmt("xpdfTile{0:d} setpattern\n", numTilingPatterns);

  // fill with the pattern
  writePSFmt("{0:.6g} {1:.6g} {2:.6g} {3:.6g} rectfill\n",
	     x0 * xStep + bbox[0],
	     y0 * yStep + bbox[1],
	     (x1 - x0) * xStep + bbox[2],
	     (y1 - y0) * yStep + bbox[3]);

  writePS("grestore\n");
  noStateChanges = gFalse;
}

GBool PSOutputDev::functionShadedFill(GfxState *state,
				      GfxFunctionShading *shading) {
  double x0, y0, x1, y1;
  double *mat;
  int i;

  if (level == psLevel2Sep || level == psLevel3Sep) {
    if (shading->getColorSpace()->getMode() != csDeviceCMYK) {
      return gFalse;
    }
    processColors |= psProcessCMYK;
  }

  shading->getDomain(&x0, &y0, &x1, &y1);
  mat = shading->getMatrix();
  writePSFmt("/mat [{0:.6g} {1:.6g} {2:.6g} {3:.6g} {4:.6g} {5:.6g}] def\n",
	     mat[0], mat[1], mat[2], mat[3], mat[4], mat[5]);
  writePSFmt("/n {0:d} def\n", shading->getColorSpace()->getNComps());
  if (shading->getNFuncs() == 1) {
    writePS("/func ");
    cvtFunction(shading->getFunc(0));
    writePS("def\n");
  } else {
    writePS("/func {\n");
    for (i = 0; i < shading->getNFuncs(); ++i) {
      if (i < shading->getNFuncs() - 1) {
	writePS("2 copy\n");
      }
      cvtFunction(shading->getFunc(i));
      writePS("exec\n");
      if (i < shading->getNFuncs() - 1) {
	writePS("3 1 roll\n");
      }
    }
    writePS("} def\n");
  }
  writePSFmt("{0:.6g} {1:.6g} {2:.6g} {3:.6g} 0 funcSH\n", x0, y0, x1, y1);

  noStateChanges = gFalse;
  return gTrue;
}

GBool PSOutputDev::axialShadedFill(GfxState *state, GfxAxialShading *shading) {
  double xMin, yMin, xMax, yMax;
  double x0, y0, x1, y1, dx, dy, mul;
  double tMin, tMax, t, t0, t1;
  int i;

  if (level == psLevel2Sep || level == psLevel3Sep) {
    if (shading->getColorSpace()->getMode() != csDeviceCMYK) {
      return gFalse;
    }
    processColors |= psProcessCMYK;
  }

  // get the clip region bbox
  state->getUserClipBBox(&xMin, &yMin, &xMax, &yMax);

  // compute min and max t values, based on the four corners of the
  // clip region bbox
  shading->getCoords(&x0, &y0, &x1, &y1);
  dx = x1 - x0;
  dy = y1 - y0;
  if (fabs(dx) < 0.01 && fabs(dy) < 0.01) {
    return gTrue;
  } else {
    mul = 1 / (dx * dx + dy * dy);
    tMin = tMax = ((xMin - x0) * dx + (yMin - y0) * dy) * mul;
    t = ((xMin - x0) * dx + (yMax - y0) * dy) * mul;
    if (t < tMin) {
      tMin = t;
    } else if (t > tMax) {
      tMax = t;
    }
    t = ((xMax - x0) * dx + (yMin - y0) * dy) * mul;
    if (t < tMin) {
      tMin = t;
    } else if (t > tMax) {
      tMax = t;
    }
    t = ((xMax - x0) * dx + (yMax - y0) * dy) * mul;
    if (t < tMin) {
      tMin = t;
    } else if (t > tMax) {
      tMax = t;
    }
    if (tMin < 0 && !shading->getExtend0()) {
      tMin = 0;
    }
    if (tMax > 1 && !shading->getExtend1()) {
      tMax = 1;
    }
  }

  // get the function domain
  t0 = shading->getDomain0();
  t1 = shading->getDomain1();

  // generate the PS code
  writePSFmt("/t0 {0:.6g} def\n", t0);
  writePSFmt("/t1 {0:.6g} def\n", t1);
  writePSFmt("/dt {0:.6g} def\n", t1 - t0);
  writePSFmt("/x0 {0:.6g} def\n", x0);
  writePSFmt("/y0 {0:.6g} def\n", y0);
  writePSFmt("/dx {0:.6g} def\n", x1 - x0);
  writePSFmt("/x1 {0:.6g} def\n", x1);
  writePSFmt("/y1 {0:.6g} def\n", y1);
  writePSFmt("/dy {0:.6g} def\n", y1 - y0);
  writePSFmt("/xMin {0:.6g} def\n", xMin);
  writePSFmt("/yMin {0:.6g} def\n", yMin);
  writePSFmt("/xMax {0:.6g} def\n", xMax);
  writePSFmt("/yMax {0:.6g} def\n", yMax);
  writePSFmt("/n {0:d} def\n", shading->getColorSpace()->getNComps());
  if (shading->getNFuncs() == 1) {
    writePS("/func ");
    cvtFunction(shading->getFunc(0));
    writePS("def\n");
  } else {
    writePS("/func {\n");
    for (i = 0; i < shading->getNFuncs(); ++i) {
      if (i < shading->getNFuncs() - 1) {
	writePS("dup\n");
      }
      cvtFunction(shading->getFunc(i));
      writePS("exec\n");
      if (i < shading->getNFuncs() - 1) {
	writePS("exch\n");
      }
    }
    writePS("} def\n");
  }
  writePSFmt("{0:.6g} {1:.6g} 0 axialSH\n", tMin, tMax);

  noStateChanges = gFalse;
  return gTrue;
}

GBool PSOutputDev::radialShadedFill(GfxState *state,
				    GfxRadialShading *shading) {
  double xMin, yMin, xMax, yMax;
  double x0, y0, r0, x1, y1, r1, t0, t1;
  double xa, ya, ra;
  double sMin, sMax, h, ta;
  double sLeft, sRight, sTop, sBottom, sZero, sDiag;
  GBool haveSLeft, haveSRight, haveSTop, haveSBottom, haveSZero;
  GBool haveSMin, haveSMax;
  double theta, alpha, a1, a2;
  GBool enclosed;
  int i;

  if (level == psLevel2Sep || level == psLevel3Sep) {
    if (shading->getColorSpace()->getMode() != csDeviceCMYK) {
      return gFalse;
    }
    processColors |= psProcessCMYK;
  }

  // get the shading info
  shading->getCoords(&x0, &y0, &r0, &x1, &y1, &r1);
  t0 = shading->getDomain0();
  t1 = shading->getDomain1();

  // Compute the point at which r(s) = 0; check for the enclosed
  // circles case; and compute the angles for the tangent lines.
  h = sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0));
  if (h == 0) {
    enclosed = gTrue;
    theta = 0; // make gcc happy
  } else if (r1 - r0 == 0) {
    enclosed = gFalse;
    theta = 0;
  } else if (fabs(r1 - r0) >= h) {
    enclosed = gTrue;
    theta = 0; // make gcc happy
  } else {
    enclosed = gFalse;
    theta = asin((r1 - r0) / h);
  }
  if (enclosed) {
    a1 = 0;
    a2 = 360;
  } else {
    alpha = atan2(y1 - y0, x1 - x0);
    a1 = (180 / M_PI) * (alpha + theta) + 90;
    a2 = (180 / M_PI) * (alpha - theta) - 90;
    while (a2 < a1) {
      a2 += 360;
    }
  }

  // compute the (possibly extended) s range
  state->getUserClipBBox(&xMin, &yMin, &xMax, &yMax);
  if (enclosed) {
    sMin = 0;
    sMax = 1;
  } else {
    // solve x(sLeft) + r(sLeft) = xMin
    if ((haveSLeft = fabs((x1 + r1) - (x0 + r0)) > 0.000001)) {
      sLeft = (xMin - (x0 + r0)) / ((x1 + r1) - (x0 + r0));
    } else {
      sLeft = 0; // make gcc happy
    }
    // solve x(sRight) - r(sRight) = xMax
    if ((haveSRight = fabs((x1 - r1) - (x0 - r0)) > 0.000001)) {
      sRight = (xMax - (x0 - r0)) / ((x1 - r1) - (x0 - r0));
    } else {
      sRight = 0; // make gcc happy
    }
    // solve y(sBottom) + r(sBottom) = yMin
    if ((haveSBottom = fabs((y1 + r1) - (y0 + r0)) > 0.000001)) {
      sBottom = (yMin - (y0 + r0)) / ((y1 + r1) - (y0 + r0));
    } else {
      sBottom = 0; // make gcc happy
    }
    // solve y(sTop) - r(sTop) = yMax
    if ((haveSTop = fabs((y1 - r1) - (y0 - r0)) > 0.000001)) {
      sTop = (yMax - (y0 - r0)) / ((y1 - r1) - (y0 - r0));
    } else {
      sTop = 0; // make gcc happy
    }
    // solve r(sZero) = 0
    if ((haveSZero = fabs(r1 - r0) > 0.000001)) {
      sZero = -r0 / (r1 - r0);
    } else {
      sZero = 0; // make gcc happy
    }
    // solve r(sDiag) = sqrt((xMax-xMin)^2 + (yMax-yMin)^2)
    if (haveSZero) {
      sDiag = (sqrt((xMax - xMin) * (xMax - xMin) +
		    (yMax - yMin) * (yMax - yMin)) - r0) / (r1 - r0);
    } else {
      sDiag = 0; // make gcc happy
    }
    // compute sMin
    if (shading->getExtend0()) {
      sMin = 0;
      haveSMin = gFalse;
      if (x0 < x1 && haveSLeft && sLeft < 0) {
	sMin = sLeft;
	haveSMin = gTrue;
      } else if (x0 > x1 && haveSRight && sRight < 0) {
	sMin = sRight;
	haveSMin = gTrue;
      }
      if (y0 < y1 && haveSBottom && sBottom < 0) {
	if (!haveSMin || sBottom > sMin) {
	  sMin = sBottom;
	  haveSMin = gTrue;
	}
      } else if (y0 > y1 && haveSTop && sTop < 0) {
	if (!haveSMin || sTop > sMin) {
	  sMin = sTop;
	  haveSMin = gTrue;
	}
      }
      if (haveSZero && sZero < 0) {
	if (!haveSMin || sZero > sMin) {
	  sMin = sZero;
	}
      }
    } else {
      sMin = 0;
    }
    // compute sMax
    if (shading->getExtend1()) {
      sMax = 1;
      haveSMax = gFalse;
      if (x1 < x0 && haveSLeft && sLeft > 1) {
	sMax = sLeft;
	haveSMax = gTrue;
      } else if (x1 > x0 && haveSRight && sRight > 1) {
	sMax = sRight;
	haveSMax = gTrue;
      }
      if (y1 < y0 && haveSBottom && sBottom > 1) {
	if (!haveSMax || sBottom < sMax) {
	  sMax = sBottom;
	  haveSMax = gTrue;
	}
      } else if (y1 > y0 && haveSTop && sTop > 1) {
	if (!haveSMax || sTop < sMax) {
	  sMax = sTop;
	  haveSMax = gTrue;
	}
      }
      if (haveSZero && sDiag > 1) {
	if (!haveSMax || sDiag < sMax) {
	  sMax = sDiag;
	}
      }
    } else {
      sMax = 1;
    }
  }

  // generate the PS code
  writePSFmt("/x0 {0:.6g} def\n", x0);
  writePSFmt("/x1 {0:.6g} def\n", x1);
  writePSFmt("/dx {0:.6g} def\n", x1 - x0);
  writePSFmt("/y0 {0:.6g} def\n", y0);
  writePSFmt("/y1 {0:.6g} def\n", y1);
  writePSFmt("/dy {0:.6g} def\n", y1 - y0);
  writePSFmt("/r0 {0:.6g} def\n", r0);
  writePSFmt("/r1 {0:.6g} def\n", r1);
  writePSFmt("/dr {0:.6g} def\n", r1 - r0);
  writePSFmt("/t0 {0:.6g} def\n", t0);
  writePSFmt("/t1 {0:.6g} def\n", t1);
  writePSFmt("/dt {0:.6g} def\n", t1 - t0);
  writePSFmt("/n {0:d} def\n", shading->getColorSpace()->getNComps());
  writePSFmt("/encl {0:s} def\n", enclosed ? "true" : "false");
  writePSFmt("/a1 {0:.6g} def\n", a1);
  writePSFmt("/a2 {0:.6g} def\n", a2);
  if (shading->getNFuncs() == 1) {
    writePS("/func ");
    cvtFunction(shading->getFunc(0));
    writePS("def\n");
  } else {
    writePS("/func {\n");
    for (i = 0; i < shading->getNFuncs(); ++i) {
      if (i < shading->getNFuncs() - 1) {
	writePS("dup\n");
      }
      cvtFunction(shading->getFunc(i));
      writePS("exec\n");
      if (i < shading->getNFuncs() - 1) {
	writePS("exch\n");
      }
    }
    writePS("} def\n");
  }
  writePSFmt("{0:.6g} {1:.6g} 0 radialSH\n", sMin, sMax);

  // extend the 'enclosed' case
  if (enclosed) {
    // extend the smaller circle
    if ((shading->getExtend0() && r0 <= r1) ||
	(shading->getExtend1() && r1 < r0)) {
      if (r0 <= r1) {
	ta = t0;
	ra = r0;
	xa = x0;
	ya = y0;
      } else {
	ta = t1;
	ra = r1;
	xa = x1;
	ya = y1;
      }
      if (level == psLevel2Sep || level == psLevel3Sep) {
	writePSFmt("{0:.6g} radialCol aload pop k\n", ta);
      } else {
	writePSFmt("{0:.6g} radialCol sc\n", ta);
      }
      writePSFmt("{0:.6g} {1:.6g} {2:.6g} 0 360 arc h f*\n", xa, ya, ra);
    }

    // extend the larger circle
    if ((shading->getExtend0() && r0 > r1) ||
	(shading->getExtend1() && r1 >= r0)) {
      if (r0 > r1) {
	ta = t0;
	ra = r0;
	xa = x0;
	ya = y0;
      } else {
	ta = t1;
	ra = r1;
	xa = x1;
	ya = y1;
      }
      if (level == psLevel2Sep || level == psLevel3Sep) {
	writePSFmt("{0:.6g} radialCol aload pop k\n", ta);
      } else {
	writePSFmt("{0:.6g} radialCol sc\n", ta);
      }
      writePSFmt("{0:.6g} {1:.6g} {2:.6g} 0 360 arc h\n", xa, ya, ra);
      writePSFmt("{0:.6g} {1:.6g} m {2:.6g} {3:.6g} l {4:.6g} {5:.6g} l {6:.6g} {7:.6g} l h f*\n",
		 xMin, yMin, xMin, yMax, xMax, yMax, xMax, yMin);
    }
  }

  noStateChanges = gFalse;
  return gTrue;
}

void PSOutputDev::clip(GfxState *state) {
  doPath(state->getPath());
  writePS("W\n");
  noStateChanges = gFalse;
}

void PSOutputDev::eoClip(GfxState *state) {
  doPath(state->getPath());
  writePS("W*\n");
  noStateChanges = gFalse;
}

void PSOutputDev::clipToStrokePath(GfxState *state) {
  doPath(state->getPath());
  writePS("Ws\n");
  noStateChanges = gFalse;
}

void PSOutputDev::doPath(GfxPath *path) {
  GfxSubpath *subpath;
  double x0, y0, x1, y1, x2, y2, x3, y3, x4, y4;
  int n, m, i, j;

  n = path->getNumSubpaths();

  if (n == 1 && path->getSubpath(0)->getNumPoints() == 5) {
    subpath = path->getSubpath(0);
    x0 = subpath->getX(0);
    y0 = subpath->getY(0);
    x4 = subpath->getX(4);
    y4 = subpath->getY(4);
    if (x4 == x0 && y4 == y0) {
      x1 = subpath->getX(1);
      y1 = subpath->getY(1);
      x2 = subpath->getX(2);
      y2 = subpath->getY(2);
      x3 = subpath->getX(3);
      y3 = subpath->getY(3);
      if (x0 == x1 && x2 == x3 && y0 == y3 && y1 == y2) {
	writePSFmt("{0:.6g} {1:.6g} {2:.6g} {3:.6g} re\n",
		   x0 < x2 ? x0 : x2, y0 < y1 ? y0 : y1,
		   fabs(x2 - x0), fabs(y1 - y0));
	return;
      } else if (x0 == x3 && x1 == x2 && y0 == y1 && y2 == y3) {
	writePSFmt("{0:.6g} {1:.6g} {2:.6g} {3:.6g} re\n",
		   x0 < x1 ? x0 : x1, y0 < y2 ? y0 : y2,
		   fabs(x1 - x0), fabs(y2 - y0));
	return;
      }
    }
  }

  for (i = 0; i < n; ++i) {
    subpath = path->getSubpath(i);
    m = subpath->getNumPoints();
    writePSFmt("{0:.6g} {1:.6g} m\n", subpath->getX(0), subpath->getY(0));
    j = 1;
    while (j < m) {
      if (subpath->getCurve(j)) {
	writePSFmt("{0:.6g} {1:.6g} {2:.6g} {3:.6g} {4:.6g} {5:.6g} c\n",
		   subpath->getX(j), subpath->getY(j),
		   subpath->getX(j+1), subpath->getY(j+1),
		   subpath->getX(j+2), subpath->getY(j+2));
	j += 3;
      } else {
	writePSFmt("{0:.6g} {1:.6g} l\n", subpath->getX(j), subpath->getY(j));
	++j;
      }
    }
    if (subpath->isClosed()) {
      writePS("h\n");
    }
  }
}

void PSOutputDev::drawString(GfxState *state, GString *s) {
  GfxFont *font;
  int wMode;
  int *codeToGID;
  GString *s2;
  double dx, dy, originX, originY, originX0, originY0, tOriginX0, tOriginY0;
  char *p;
  PSFontInfo *fi;
  UnicodeMap *uMap;
  CharCode code;
  Unicode u[8];
  char buf[8];
  double *dxdy;
  int dxdySize, len, nChars, uLen, n, m, i, j;

  // check for invisible text -- this is used by Acrobat Capture
  if (state->getRender() == 3) {
    return;
  }

  // ignore empty strings
  if (s->getLength() == 0) {
    return;
  }

  // get the font
  if (!(font = state->getFont())) {
    return;
  }
  wMode = font->getWMode();

  fi = NULL;
  for (i = 0; i < fontInfo->getLength(); ++i) {
    fi = (PSFontInfo *)fontInfo->get(i);
    if (fi->fontID.num == font->getID()->num &&
	fi->fontID.gen == font->getID()->gen) {
      break;
    }
    fi = NULL;
  }

  // check for a subtitute 16-bit font
  uMap = NULL;
  codeToGID = NULL;
  if (font->isCIDFont()) {
    if (!(fi && fi->ff)) {
      // font substitution failed, so don't output any text
      return;
    }
    if (fi->ff->encoding) {
      uMap = globalParams->getUnicodeMap(fi->ff->encoding);
    }

  // check for an 8-bit code-to-GID map
  } else {
    if (fi && fi->ff) {
      codeToGID = fi->ff->codeToGID;
    }
  }

  // compute the positioning (dx, dy) for each char in the string
  nChars = 0;
  p = s->getCString();
  len = s->getLength();
  s2 = new GString();
  dxdySize = font->isCIDFont() ? 8 : s->getLength();
  dxdy = (double *)gmallocn(2 * dxdySize, sizeof(double));
  originX0 = originY0 = 0; // make gcc happy
  while (len > 0) {
    n = font->getNextChar(p, len, &code,
			  u, (int)(sizeof(u) / sizeof(Unicode)), &uLen,
			  &dx, &dy, &originX, &originY);
    //~ this doesn't handle the case where the origin offset changes
    //~   within a string of characters -- which could be fixed by
    //~   modifying dx,dy as needed for each character
    if (p == s->getCString()) {
      originX0 = originX;
      originY0 = originY;
    }
    dx *= state->getFontSize();
    dy *= state->getFontSize();
    if (wMode) {
      dy += state->getCharSpace();
      if (n == 1 && *p == ' ') {
	dy += state->getWordSpace();
      }
    } else {
      dx += state->getCharSpace();
      if (n == 1 && *p == ' ') {
	dx += state->getWordSpace();
      }
    }
    dx *= state->getHorizScaling();
    if (font->isCIDFont()) {
      if (uMap) {
	if (nChars + uLen > dxdySize) {
	  do {
	    dxdySize *= 2;
	  } while (nChars + uLen > dxdySize);
	  dxdy = (double *)greallocn(dxdy, 2 * dxdySize, sizeof(double));
	}
	for (i = 0; i < uLen; ++i) {
	  m = uMap->mapUnicode(u[i], buf, (int)sizeof(buf));
	  for (j = 0; j < m; ++j) {
	    s2->append(buf[j]);
	  }
	  //~ this really needs to get the number of chars in the target
	  //~ encoding - which may be more than the number of Unicode
	  //~ chars
	  dxdy[2 * nChars] = dx;
	  dxdy[2 * nChars + 1] = dy;
	  ++nChars;
	}
      } else {
	if (nChars + 1 > dxdySize) {
	  dxdySize *= 2;
	  dxdy = (double *)greallocn(dxdy, 2 * dxdySize, sizeof(double));
	}
	s2->append((char)((code >> 8) & 0xff));
	s2->append((char)(code & 0xff));
	dxdy[2 * nChars] = dx;
	dxdy[2 * nChars + 1] = dy;
	++nChars;
      }
    } else {
      if (!codeToGID || codeToGID[code] >= 0) {
	s2->append((char)code);
	dxdy[2 * nChars] = dx;
	dxdy[2 * nChars + 1] = dy;
	++nChars;
      }
    }
    p += n;
    len -= n;
  }
  if (uMap) {
    uMap->decRefCnt();
  }
  originX0 *= state->getFontSize();
  originY0 *= state->getFontSize();
  state->textTransformDelta(originX0, originY0, &tOriginX0, &tOriginY0);

  if (nChars > 0) {
    if (wMode) {
      writePSFmt("{0:.6g} {1:.6g} rmoveto\n", -tOriginX0, -tOriginY0);
    }
    writePSString(s2);
    writePS("\n[");
    for (i = 0; i < 2 * nChars; ++i) {
      if (i > 0) {
	writePS("\n");
      }
      writePSFmt("{0:.6g}", dxdy[i]);
    }
    if (font->getType() == fontType3) {
      writePS("] Tj3\n");
    } else {
      writePS("] Tj\n");
    }
    if (wMode) {
      writePSFmt("{0:.6g} {1:.6g} rmoveto\n", tOriginX0, tOriginY0);
    }
  }
  gfree(dxdy);
  delete s2;

  if ((state->getRender() & 4) && font->getType() != fontType3) {
    haveTextClip = gTrue;
  }

  noStateChanges = gFalse;
}

void PSOutputDev::endTextObject(GfxState *state) {
  if (haveTextClip) {
    writePS("Tclip\n");
    haveTextClip = gFalse;
    noStateChanges = gFalse;
  }
}

void PSOutputDev::drawImageMask(GfxState *state, Object *ref, Stream *str,
				int width, int height, GBool invert,
				GBool inlineImg, GBool interpolate) {
  int len;

  len = height * ((width + 7) / 8);
  switch (level) {
  case psLevel1:
  case psLevel1Sep:
    doImageL1(ref, state, NULL, invert, inlineImg, str, width, height, len);
    break;
  case psLevel2:
  case psLevel2Gray:
  case psLevel2Sep:
    doImageL2(ref, state, NULL, invert, inlineImg, str, width, height, len,
	      NULL, NULL, 0, 0, gFalse);
    break;
  case psLevel3:
  case psLevel3Gray:
  case psLevel3Sep:
    doImageL3(ref, state, NULL, invert, inlineImg, str, width, height, len,
	      NULL, NULL, 0, 0, gFalse);
    break;
  }
  noStateChanges = gFalse;
}

void PSOutputDev::drawImage(GfxState *state, Object *ref, Stream *str,
			    int width, int height, GfxImageColorMap *colorMap,
			    int *maskColors, GBool inlineImg,
			    GBool interpolate) {
  int len;

  len = height * ((width * colorMap->getNumPixelComps() *
		   colorMap->getBits() + 7) / 8);
  switch (level) {
  case psLevel1:
    doImageL1(ref, state, colorMap, gFalse, inlineImg, str,
	      width, height, len);
    break;
  case psLevel1Sep:
    //~ handle indexed, separation, ... color spaces
    doImageL1Sep(state, colorMap, gFalse, inlineImg, str, width, height, len);
    break;
  case psLevel2:
  case psLevel2Gray:
  case psLevel2Sep:
    doImageL2(ref, state, colorMap, gFalse, inlineImg, str,
	      width, height, len, maskColors, NULL, 0, 0, gFalse);
    break;
  case psLevel3:
  case psLevel3Gray:
  case psLevel3Sep:
    doImageL3(ref, state, colorMap, gFalse, inlineImg, str,
	      width, height, len, maskColors, NULL, 0, 0, gFalse);
    break;
  }
  t3Cacheable = gFalse;
  noStateChanges = gFalse;
}

void PSOutputDev::drawMaskedImage(GfxState *state, Object *ref, Stream *str,
				  int width, int height,
				  GfxImageColorMap *colorMap,
				  Stream *maskStr,
				  int maskWidth, int maskHeight,
				  GBool maskInvert, GBool interpolate) {
  int len;

  len = height * ((width * colorMap->getNumPixelComps() *
		   colorMap->getBits() + 7) / 8);
  switch (level) {
  case psLevel1:
    doImageL1(ref, state, colorMap, gFalse, gFalse, str, width, height, len);
    break;
  case psLevel1Sep:
    //~ handle indexed, separation, ... color spaces
    doImageL1Sep(state, colorMap, gFalse, gFalse, str, width, height, len);
    break;
  case psLevel2:
  case psLevel2Gray:
  case psLevel2Sep:
    doImageL2(ref, state, colorMap, gFalse, gFalse, str, width, height, len,
	      NULL, maskStr, maskWidth, maskHeight, maskInvert);
    break;
  case psLevel3:
  case psLevel3Gray:
  case psLevel3Sep:
    doImageL3(ref, state, colorMap, gFalse, gFalse, str, width, height, len,
	      NULL, maskStr, maskWidth, maskHeight, maskInvert);
    break;
  }
  t3Cacheable = gFalse;
  noStateChanges = gFalse;
}

void PSOutputDev::doImageL1(Object *ref, GfxState *state,
			    GfxImageColorMap *colorMap,
			    GBool invert, GBool inlineImg,
			    Stream *str, int width, int height, int len) {
  ImageStream *imgStr;
  Guchar pixBuf[gfxColorMaxComps];
  GfxGray gray;
  int col, x, y, c, i;

  if ((inType3Char || preload) && !colorMap) {
    if (inlineImg) {
      // create an array
      str = new FixedLengthEncoder(str, len);
      str = new ASCIIHexEncoder(str);
      str->reset();
      col = 0;
      writePS("[<");
      do {
	do {
	  c = str->getChar();
	} while (c == '\n' || c == '\r');
	if (c == '>' || c == EOF) {
	  break;
	}
	writePSChar((char)c);
	++col;
	// each line is: "<...data...><eol>"
	// so max data length = 255 - 4 = 251
	// but make it 240 just to be safe
	// chunks are 2 bytes each, so we need to stop on an even col number
	if (col == 240) {
	  writePS(">\n<");
	  col = 0;
	}
      } while (c != '>' && c != EOF);
      writePS(">]\n");
      writePS("0\n");
      str->close();
      delete str;
    } else {
      // set up to use the array already created by setupImages()
      writePSFmt("ImData_{0:d}_{1:d} 0\n", ref->getRefNum(), ref->getRefGen());
    }
  }

  // image/imagemask command
  if ((inType3Char || preload) && !colorMap) {
    writePSFmt("{0:d} {1:d} {2:s} [{3:d} 0 0 {4:d} 0 {5:d}] pdfImM1a\n",
	       width, height, invert ? "true" : "false",
	       width, -height, height);
  } else if (colorMap) {
    writePSFmt("{0:d} {1:d} 8 [{2:d} 0 0 {3:d} 0 {4:d}] pdfIm1\n",
	       width, height,
	       width, -height, height);
  } else {
    writePSFmt("{0:d} {1:d} {2:s} [{3:d} 0 0 {4:d} 0 {5:d}] pdfImM1\n",
	       width, height, invert ? "true" : "false",
	       width, -height, height);
  }

  // image data
  if (!((inType3Char || preload) && !colorMap)) {

    if (colorMap) {

      // set up to process the data stream
      imgStr = new ImageStream(str, width, colorMap->getNumPixelComps(),
			       colorMap->getBits());
      imgStr->reset();

      // process the data stream
      i = 0;
      for (y = 0; y < height; ++y) {

	// write the line
	for (x = 0; x < width; ++x) {
	  imgStr->getPixel(pixBuf);
	  colorMap->getGray(pixBuf, &gray, state->getRenderingIntent());
	  writePSFmt("{0:02x}", colToByte(gray));
	  if (++i == 32) {
	    writePSChar('\n');
	    i = 0;
	  }
	}
      }
      if (i != 0) {
	writePSChar('\n');
      }
      str->close();
      delete imgStr;

    // imagemask
    } else {
      str->reset();
      i = 0;
      for (y = 0; y < height; ++y) {
	for (x = 0; x < width; x += 8) {
	  writePSFmt("{0:02x}", str->getChar() & 0xff);
	  if (++i == 32) {
	    writePSChar('\n');
	    i = 0;
	  }
	}
      }
      if (i != 0) {
	writePSChar('\n');
      }
      str->close();
    }
  }
}

void PSOutputDev::doImageL1Sep(GfxState *state, GfxImageColorMap *colorMap,
			       GBool invert, GBool inlineImg,
			       Stream *str, int width, int height, int len) {
  ImageStream *imgStr;
  Guchar *lineBuf;
  Guchar pixBuf[gfxColorMaxComps];
  GfxCMYK cmyk;
  int x, y, i, comp;

  // width, height, matrix, bits per component
  writePSFmt("{0:d} {1:d} 8 [{2:d} 0 0 {3:d} 0 {4:d}] pdfIm1Sep\n",
	     width, height,
	     width, -height, height);

  // allocate a line buffer
  lineBuf = (Guchar *)gmallocn(width, 4);

  // set up to process the data stream
  imgStr = new ImageStream(str, width, colorMap->getNumPixelComps(),
			   colorMap->getBits());
  imgStr->reset();

  // process the data stream
  i = 0;
  for (y = 0; y < height; ++y) {

    // read the line
    for (x = 0; x < width; ++x) {
      imgStr->getPixel(pixBuf);
      colorMap->getCMYK(pixBuf, &cmyk, state->getRenderingIntent());
      lineBuf[4*x+0] = colToByte(cmyk.c);
      lineBuf[4*x+1] = colToByte(cmyk.m);
      lineBuf[4*x+2] = colToByte(cmyk.y);
      lineBuf[4*x+3] = colToByte(cmyk.k);
      addProcessColor(colToDbl(cmyk.c), colToDbl(cmyk.m),
		      colToDbl(cmyk.y), colToDbl(cmyk.k));
    }

    // write one line of each color component
    for (comp = 0; comp < 4; ++comp) {
      for (x = 0; x < width; ++x) {
	writePSFmt("{0:02x}", lineBuf[4*x + comp]);
	if (++i == 32) {
	  writePSChar('\n');
	  i = 0;
	}
      }
    }
  }

  if (i != 0) {
    writePSChar('\n');
  }

  str->close();
  delete imgStr;
  gfree(lineBuf);
}

void PSOutputDev::doImageL2(Object *ref, GfxState *state,
			    GfxImageColorMap *colorMap,
			    GBool invert, GBool inlineImg,
			    Stream *str, int width, int height, int len,
			    int *maskColors, Stream *maskStr,
			    int maskWidth, int maskHeight, GBool maskInvert) {
  Stream *str2;
  GString *s;
  int n, numComps;
  GBool useLZW, useRLE, useASCII, useASCIIHex, useCompressed;
  GfxSeparationColorSpace *sepCS;
  GfxColor color;
  GfxCMYK cmyk;
  char buf[4096];
  int c, col, i;

  // color key masking
  if (maskColors && colorMap && !inlineImg) {
    // can't read the stream twice for inline images -- but masking
    // isn't allowed with inline images anyway
    convertColorKeyMaskToClipRects(colorMap, str, width, height, maskColors);

  // explicit masking
  } else if (maskStr) {
    convertExplicitMaskToClipRects(maskStr, maskWidth, maskHeight, maskInvert);
  }

  // color space
  if (colorMap && !(level == psLevel2Gray || level == psLevel3Gray)) {
    dumpColorSpaceL2(state, colorMap->getColorSpace(), gFalse, gTrue, gFalse);
    writePS(" setcolorspace\n");
  }

  useASCIIHex = globalParams->getPSASCIIHex();

  // set up the image data
  if (mode == psModeForm || inType3Char || preload) {
    if (inlineImg) {
      // create an array
      str2 = new FixedLengthEncoder(str, len);
      if (colorMap && (level == psLevel2Gray || level == psLevel3Gray)) {
	str2 = new GrayRecoder(str2, width, height, colorMap);
      }
      if (globalParams->getPSLZW()) {
	str2 = new LZWEncoder(str2);
      } else {
	str2 = new RunLengthEncoder(str2);
      }
      if (useASCIIHex) {
	str2 = new ASCIIHexEncoder(str2);
      } else {
	str2 = new ASCII85Encoder(str2);
      }
      str2->reset();
      col = 0;
      writePS((char *)(useASCIIHex ? "[<" : "[<~"));
      do {
	do {
	  c = str2->getChar();
	} while (c == '\n' || c == '\r');
	if (c == (useASCIIHex ? '>' : '~') || c == EOF) {
	  break;
	}
	if (c == 'z') {
	  writePSChar((char)c);
	  ++col;
	} else {
	  writePSChar((char)c);
	  ++col;
	  for (i = 1; i <= (useASCIIHex ? 1 : 4); ++i) {
	    do {
	      c = str2->getChar();
	    } while (c == '\n' || c == '\r');
	    if (c == (useASCIIHex ? '>' : '~') || c == EOF) {
	      break;
	    }
	    writePSChar((char)c);
	    ++col;
	  }
	}
	// each line is: "<~...data...~><eol>"
	// so max data length = 255 - 6 = 249
	// chunks are 1 or 5 bytes each, so we have to stop at 245
	// but make it 240 just to be safe
	if (col > 240) {
	  writePS((char *)(useASCIIHex ? ">\n<" : "~>\n<~"));
	  col = 0;
	}
      } while (c != (useASCIIHex ? '>' : '~') && c != EOF);
      writePS((char *)(useASCIIHex ? ">\n" : "~>\n"));
      // add an extra entry because the LZWDecode/RunLengthDecode
      // filter may read past the end
      writePS("<>]\n");
      writePS("0\n");
      str2->close();
      delete str2;
    } else {
      // set up to use the array already created by setupImages()
      writePSFmt("ImData_{0:d}_{1:d} 0\n", ref->getRefNum(), ref->getRefGen());
    }
  }

  // image dictionary
  writePS("<<\n  /ImageType 1\n");

  // width, height, matrix, bits per component
  writePSFmt("  /Width {0:d}\n", width);
  writePSFmt("  /Height {0:d}\n", height);
  writePSFmt("  /ImageMatrix [{0:d} 0 0 {1:d} 0 {2:d}]\n",
	     width, -height, height);
  if (colorMap && (colorMap->getColorSpace()->getMode() == csDeviceN ||
		   level == psLevel2Gray || level == psLevel3Gray)) {
    writePS("  /BitsPerComponent 8\n");
  } else {
    writePSFmt("  /BitsPerComponent {0:d}\n",
	       colorMap ? colorMap->getBits() : 1);
  }

  // decode 
  if (colorMap) {
    writePS("  /Decode [");
    if ((level == psLevel2Sep || level == psLevel3Sep) &&
	colorMap->getColorSpace()->getMode() == csSeparation) {
      // this matches up with the code in the pdfImSep operator
      n = (1 << colorMap->getBits()) - 1;
      writePSFmt("{0:.4g} {1:.4g}", colorMap->getDecodeLow(0) * n,
		 colorMap->getDecodeHigh(0) * n);
    } else if (level == psLevel2Gray || level == psLevel3Gray) {
      writePS("0 1");
    } else if (colorMap->getColorSpace()->getMode() == csDeviceN) {
      numComps = ((GfxDeviceNColorSpace *)colorMap->getColorSpace())->
	           getAlt()->getNComps();
      for (i = 0; i < numComps; ++i) {
	if (i > 0) {
	  writePS(" ");
	}
	writePS("0 1");
      }
    } else {
      numComps = colorMap->getNumPixelComps();
      for (i = 0; i < numComps; ++i) {
	if (i > 0) {
	  writePS(" ");
	}
	writePSFmt("{0:.4g} {1:.4g}",
		   colorMap->getDecodeLow(i), colorMap->getDecodeHigh(i));
      }
    }
    writePS("]\n");
  } else {
    writePSFmt("  /Decode [{0:d} {1:d}]\n", invert ? 1 : 0, invert ? 0 : 1);
  }

  // data source
  if (mode == psModeForm || inType3Char || preload) {
    writePS("  /DataSource { pdfImStr }\n");
  } else {
    writePS("  /DataSource currentfile\n");
  }

  // filters
  if ((mode == psModeForm || inType3Char || preload) &&
      globalParams->getPSUncompressPreloadedImages()) {
    s = NULL;
    useLZW = useRLE = gFalse;
    useCompressed = gFalse;
    useASCII = gFalse;
  } else {
    s = str->getPSFilter(level < psLevel2 ? 1 : level < psLevel3 ? 2 : 3,
			 "    ");
    if ((colorMap && (colorMap->getColorSpace()->getMode() == csDeviceN ||
		      level == psLevel2Gray || level == psLevel3Gray)) ||
	inlineImg || !s) {
      if (globalParams->getPSLZW()) {
	useLZW = gTrue;
	useRLE = gFalse;
      } else {
	useRLE = gTrue;
	useLZW = gFalse;
      }
      useASCII = !(mode == psModeForm || inType3Char || preload);
      useCompressed = gFalse;
    } else {
      useLZW = useRLE = gFalse;
      useASCII = str->isBinary() &&
	         !(mode == psModeForm || inType3Char || preload);
      useCompressed = gTrue;
    }
  }
  if (useASCII) {
    writePSFmt("    /ASCII{0:s}Decode filter\n",
	       useASCIIHex ? "Hex" : "85");
  }
  if (useLZW) {
    writePS("    /LZWDecode filter\n");
  } else if (useRLE) {
    writePS("    /RunLengthDecode filter\n");
  }
  if (useCompressed) {
    writePS(s->getCString());
  }
  if (s) {
    delete s;
  }

  if (mode == psModeForm || inType3Char || preload) {

    // end of image dictionary
    writePSFmt(">>\n{0:s}\n", colorMap ? "image" : "imagemask");

    // get rid of the array and index
    writePS("pop pop\n");

  } else {

    // cut off inline image streams at appropriate length
    if (inlineImg) {
      str = new FixedLengthEncoder(str, len);
    } else if (useCompressed) {
      str = str->getUndecodedStream();
    }

    // recode to grayscale
    if (colorMap && (level == psLevel2Gray || level == psLevel3Gray)) {
      str = new GrayRecoder(str, width, height, colorMap);

    // recode DeviceN data
    } else if (colorMap && colorMap->getColorSpace()->getMode() == csDeviceN) {
      str = new DeviceNRecoder(str, width, height, colorMap);
    }

    // add LZWEncode/RunLengthEncode and ASCIIHex/85 encode filters
    if (useLZW) {
      str = new LZWEncoder(str);
    } else if (useRLE) {
      str = new RunLengthEncoder(str);
    }
    if (useASCII) {
      if (useASCIIHex) {
	str = new ASCIIHexEncoder(str);
      } else {
	str = new ASCII85Encoder(str);
      }
    }

    // end of image dictionary
    writePS(">>\n");
#if OPI_SUPPORT
    if (opi13Nest) {
      if (inlineImg) {
	// this can't happen -- OPI dictionaries are in XObjects
	error(errSyntaxError, -1, "OPI in inline image");
	n = 0;
      } else {
	// need to read the stream to count characters -- the length
	// is data-dependent (because of ASCII and LZW/RunLength
	// filters)
	str->reset();
	n = 0;
	do {
	  i = str->discardChars(4096);
	  n += i;
	} while (i == 4096);
	str->close();
      }
      // +6/7 for "pdfIm\n" / "pdfImM\n"
      // +8 for newline + trailer
      n += colorMap ? 14 : 15;
      writePSFmt("%%BeginData: {0:d} Hex Bytes\n", n);
    }
#endif
    if ((level == psLevel2Sep || level == psLevel3Sep) && colorMap &&
	colorMap->getColorSpace()->getMode() == csSeparation) {
      color.c[0] = gfxColorComp1;
      sepCS = (GfxSeparationColorSpace *)colorMap->getColorSpace();
      sepCS->getCMYK(&color, &cmyk, state->getRenderingIntent());
      writePSFmt("{0:.4g} {1:.4g} {2:.4g} {3:.4g} ({4:t}) pdfImSep\n",
		 colToDbl(cmyk.c), colToDbl(cmyk.m),
		 colToDbl(cmyk.y), colToDbl(cmyk.k),
		 sepCS->getName());
    } else {
      writePSFmt("{0:s}\n", colorMap ? "pdfIm" : "pdfImM");
    }

    // copy the stream data
    str->reset();
    while ((n = str->getBlock(buf, sizeof(buf))) > 0) {
      writePSBlock(buf, n);
    }
    str->close();

    // add newline and trailer to the end
    writePSChar('\n');
    writePS("%-EOD-\n");
#if OPI_SUPPORT
    if (opi13Nest) {
      writePS("%%EndData\n");
    }
#endif

    // delete encoders
    if (useLZW || useRLE || useASCII || inlineImg) {
      delete str;
    }
  }

  if ((maskColors && colorMap && !inlineImg) || maskStr) {
    writePS("pdfImClipEnd\n");
  }
}

// Convert color key masking to a clipping region consisting of a
// sequence of clip rectangles.
void PSOutputDev::convertColorKeyMaskToClipRects(GfxImageColorMap *colorMap,
						 Stream *str,
						 int width, int height,
						 int *maskColors) {
  ImageStream *imgStr;
  Guchar *line;
  PSOutImgClipRect *rects0, *rects1, *rectsTmp, *rectsOut;
  int rects0Len, rects1Len, rectsSize, rectsOutLen, rectsOutSize;
  GBool emitRect, addRect, extendRect;
  int numComps, i, j, x0, x1, y;

  numComps = colorMap->getNumPixelComps();
  imgStr = new ImageStream(str, width, numComps, colorMap->getBits());
  imgStr->reset();
  rects0Len = rects1Len = rectsOutLen = 0;
  rectsSize = rectsOutSize = 64;
  rects0 = (PSOutImgClipRect *)gmallocn(rectsSize, sizeof(PSOutImgClipRect));
  rects1 = (PSOutImgClipRect *)gmallocn(rectsSize, sizeof(PSOutImgClipRect));
  rectsOut = (PSOutImgClipRect *)gmallocn(rectsOutSize,
					  sizeof(PSOutImgClipRect));
  for (y = 0; y < height; ++y) {
    if (!(line = imgStr->getLine())) {
      break;
    }
    i = 0;
    rects1Len = 0;
    for (x0 = 0; x0 < width; ++x0) {
      for (j = 0; j < numComps; ++j) {
	if (line[x0*numComps+j] < maskColors[2*j] ||
	    line[x0*numComps+j] > maskColors[2*j+1]) {
	  break;
	}
      }
      if (j < numComps) {
	break;
      }
    }
    for (x1 = x0; x1 < width; ++x1) {
      for (j = 0; j < numComps; ++j) {
	if (line[x1*numComps+j] < maskColors[2*j] ||
	    line[x1*numComps+j] > maskColors[2*j+1]) {
	  break;
	}
      }
      if (j == numComps) {
	break;
      }
    }
    while (x0 < width || i < rects0Len) {
      emitRect = addRect = extendRect = gFalse;
      if (x0 >= width) {
	emitRect = gTrue;
      } else if (i >= rects0Len) {
	addRect = gTrue;
      } else if (rects0[i].x0 < x0) {
	emitRect = gTrue;
      } else if (x0 < rects0[i].x0) {
	addRect = gTrue;
      } else if (rects0[i].x1 == x1) {
	extendRect = gTrue;
      } else {
	emitRect = addRect = gTrue;
      }
      if (emitRect) {
	if (rectsOutLen == rectsOutSize) {
	  rectsOutSize *= 2;
	  rectsOut = (PSOutImgClipRect *)greallocn(rectsOut, rectsOutSize,
						   sizeof(PSOutImgClipRect));
	}
	rectsOut[rectsOutLen].x0 = rects0[i].x0;
	rectsOut[rectsOutLen].x1 = rects0[i].x1;
	rectsOut[rectsOutLen].y0 = height - y - 1;
	rectsOut[rectsOutLen].y1 = height - rects0[i].y0 - 1;
	++rectsOutLen;
	++i;
      }
      if (addRect || extendRect) {
	if (rects1Len == rectsSize) {
	  rectsSize *= 2;
	  rects0 = (PSOutImgClipRect *)greallocn(rects0, rectsSize,
						 sizeof(PSOutImgClipRect));
	  rects1 = (PSOutImgClipRect *)greallocn(rects1, rectsSize,
						 sizeof(PSOutImgClipRect));
	}
	rects1[rects1Len].x0 = x0;
	rects1[rects1Len].x1 = x1;
	if (addRect) {
	  rects1[rects1Len].y0 = y;
	}
	if (extendRect) {
	  rects1[rects1Len].y0 = rects0[i].y0;
	  ++i;
	}
	++rects1Len;
	for (x0 = x1; x0 < width; ++x0) {
	  for (j = 0; j < numComps; ++j) {
	    if (line[x0*numComps+j] < maskColors[2*j] ||
		line[x0*numComps+j] > maskColors[2*j+1]) {
	      break;
	    }
	  }
	  if (j < numComps) {
	    break;
	  }
	}
	for (x1 = x0; x1 < width; ++x1) {
	  for (j = 0; j < numComps; ++j) {
	    if (line[x1*numComps+j] < maskColors[2*j] ||
		line[x1*numComps+j] > maskColors[2*j+1]) {
	      break;
	    }
	  }
	  if (j == numComps) {
	    break;
	  }
	}
      }
    }
    rectsTmp = rects0;
    rects0 = rects1;
    rects1 = rectsTmp;
    i = rects0Len;
    rects0Len = rects1Len;
    rects1Len = i;
  }
  for (i = 0; i < rects0Len; ++i) {
    if (rectsOutLen == rectsOutSize) {
      rectsOutSize *= 2;
      rectsOut = (PSOutImgClipRect *)greallocn(rectsOut, rectsOutSize,
					       sizeof(PSOutImgClipRect));
    }
    rectsOut[rectsOutLen].x0 = rects0[i].x0;
    rectsOut[rectsOutLen].x1 = rects0[i].x1;
    rectsOut[rectsOutLen].y0 = height - y - 1;
    rectsOut[rectsOutLen].y1 = height - rects0[i].y0 - 1;
    ++rectsOutLen;
  }
  writePSFmt("{0:d} {1:d}\n", width, height);
  for (i = 0; i < rectsOutLen; ++i) {
    writePSFmt("{0:d} {1:d} {2:d} {3:d} pr\n",
	       rectsOut[i].x0, rectsOut[i].y0,
	       rectsOut[i].x1 - rectsOut[i].x0,
	       rectsOut[i].y1 - rectsOut[i].y0);
  }
  writePS("pop pop pdfImClip\n");
  gfree(rectsOut);
  gfree(rects0);
  gfree(rects1);
  delete imgStr;
  str->close();
}

// Convert an explicit mask image to a clipping region consisting of a
// sequence of clip rectangles.
void PSOutputDev::convertExplicitMaskToClipRects(Stream *maskStr,
						 int maskWidth, int maskHeight,
						 GBool maskInvert) {
  ImageStream *imgStr;
  Guchar *line;
  PSOutImgClipRect *rects0, *rects1, *rectsTmp, *rectsOut;
  int rects0Len, rects1Len, rectsSize, rectsOutLen, rectsOutSize;
  GBool emitRect, addRect, extendRect;
  int i, x0, x1, y, maskXor;

  imgStr = new ImageStream(maskStr, maskWidth, 1, 1);
  imgStr->reset();
  rects0Len = rects1Len = rectsOutLen = 0;
  rectsSize = rectsOutSize = 64;
  rects0 = (PSOutImgClipRect *)gmallocn(rectsSize, sizeof(PSOutImgClipRect));
  rects1 = (PSOutImgClipRect *)gmallocn(rectsSize, sizeof(PSOutImgClipRect));
  rectsOut = (PSOutImgClipRect *)gmallocn(rectsOutSize,
					  sizeof(PSOutImgClipRect));
  maskXor = maskInvert ? 1 : 0;
  for (y = 0; y < maskHeight; ++y) {
    if (!(line = imgStr->getLine())) {
      break;
    }
    i = 0;
    rects1Len = 0;
    for (x0 = 0; x0 < maskWidth && (line[x0] ^ maskXor); ++x0) ;
    for (x1 = x0; x1 < maskWidth && !(line[x1] ^ maskXor); ++x1) ;
    while (x0 < maskWidth || i < rects0Len) {
      emitRect = addRect = extendRect = gFalse;
      if (x0 >= maskWidth) {
	emitRect = gTrue;
      } else if (i >= rects0Len) {
	addRect = gTrue;
      } else if (rects0[i].x0 < x0) {
	emitRect = gTrue;
      } else if (x0 < rects0[i].x0) {
	addRect = gTrue;
      } else if (rects0[i].x1 == x1) {
	extendRect = gTrue;
      } else {
	emitRect = addRect = gTrue;
      }
      if (emitRect) {
	if (rectsOutLen == rectsOutSize) {
	  rectsOutSize *= 2;
	  rectsOut = (PSOutImgClipRect *)greallocn(rectsOut, rectsOutSize,
						   sizeof(PSOutImgClipRect));
	}
	rectsOut[rectsOutLen].x0 = rects0[i].x0;
	rectsOut[rectsOutLen].x1 = rects0[i].x1;
	rectsOut[rectsOutLen].y0 = maskHeight - y - 1;
	rectsOut[rectsOutLen].y1 = maskHeight - rects0[i].y0 - 1;
	++rectsOutLen;
	++i;
      }
      if (addRect || extendRect) {
	if (rects1Len == rectsSize) {
	  rectsSize *= 2;
	  rects0 = (PSOutImgClipRect *)greallocn(rects0, rectsSize,
						 sizeof(PSOutImgClipRect));
	  rects1 = (PSOutImgClipRect *)greallocn(rects1, rectsSize,
						 sizeof(PSOutImgClipRect));
	}
	rects1[rects1Len].x0 = x0;
	rects1[rects1Len].x1 = x1;
	if (addRect) {
	  rects1[rects1Len].y0 = y;
	}
	if (extendRect) {
	  rects1[rects1Len].y0 = rects0[i].y0;
	  ++i;
	}
	++rects1Len;
	for (x0 = x1; x0 < maskWidth && (line[x0] ^ maskXor); ++x0) ;
	for (x1 = x0; x1 < maskWidth && !(line[x1] ^ maskXor); ++x1) ;
      }
    }
    rectsTmp = rects0;
    rects0 = rects1;
    rects1 = rectsTmp;
    i = rects0Len;
    rects0Len = rects1Len;
    rects1Len = i;
  }
  for (i = 0; i < rects0Len; ++i) {
    if (rectsOutLen == rectsOutSize) {
      rectsOutSize *= 2;
      rectsOut = (PSOutImgClipRect *)greallocn(rectsOut, rectsOutSize,
					       sizeof(PSOutImgClipRect));
    }
    rectsOut[rectsOutLen].x0 = rects0[i].x0;
    rectsOut[rectsOutLen].x1 = rects0[i].x1;
    rectsOut[rectsOutLen].y0 = maskHeight - y - 1;
    rectsOut[rectsOutLen].y1 = maskHeight - rects0[i].y0 - 1;
    ++rectsOutLen;
  }
  writePSFmt("{0:d} {1:d}\n", maskWidth, maskHeight);
  for (i = 0; i < rectsOutLen; ++i) {
    writePSFmt("{0:d} {1:d} {2:d} {3:d} pr\n",
	       rectsOut[i].x0, rectsOut[i].y0,
	       rectsOut[i].x1 - rectsOut[i].x0,
	       rectsOut[i].y1 - rectsOut[i].y0);
  }
  writePS("pop pop pdfImClip\n");
  gfree(rectsOut);
  gfree(rects0);
  gfree(rects1);
  delete imgStr;
  maskStr->close();
}

//~ this doesn't currently support OPI
void PSOutputDev::doImageL3(Object *ref, GfxState *state,
			    GfxImageColorMap *colorMap,
			    GBool invert, GBool inlineImg,
			    Stream *str, int width, int height, int len,
			    int *maskColors, Stream *maskStr,
			    int maskWidth, int maskHeight, GBool maskInvert) {
  Stream *str2;
  GString *s;
  int n, numComps;
  GBool useLZW, useRLE, useASCII, useASCIIHex, useCompressed;
  GBool maskUseLZW, maskUseRLE, maskUseASCII, maskUseCompressed;
  GString *maskFilters;
  GfxSeparationColorSpace *sepCS;
  GfxColor color;
  GfxCMYK cmyk;
  char buf[4096];
  int c;
  int col, i;

  useASCIIHex = globalParams->getPSASCIIHex();
  useLZW = useRLE = useASCII = useCompressed = gFalse; // make gcc happy
  maskUseLZW = maskUseRLE = maskUseASCII = gFalse; // make gcc happy
  maskUseCompressed = gFalse; // make gcc happy
  maskFilters = NULL; // make gcc happy

  // explicit masking
  // -- this also converts color key masking in grayscale mode
  if (maskStr || (maskColors && colorMap && level == psLevel3Gray)) {

    // mask data source
    if (maskColors && colorMap && level == psLevel3Gray) {
      s = NULL;
      if (mode == psModeForm || inType3Char || preload) {
	if (globalParams->getPSUncompressPreloadedImages()) {
	  maskUseLZW = maskUseRLE = gFalse;
	} else if (globalParams->getPSLZW()) {
	  maskUseLZW = gTrue;
	  maskUseRLE = gFalse;
	} else {
	  maskUseRLE = gTrue;
	  maskUseLZW = gFalse;
	}
	maskUseASCII = gFalse;
	maskUseCompressed = gFalse;
      } else {
	if (globalParams->getPSLZW()) {
	  maskUseLZW = gTrue;
	  maskUseRLE = gFalse;
	} else {
	  maskUseRLE = gTrue;
	  maskUseLZW = gFalse;
	}
	maskUseASCII = gTrue;
      }
      maskUseCompressed = gFalse;
      maskWidth = width;
      maskHeight = height;
      maskInvert = gFalse;
    } else if ((mode == psModeForm || inType3Char || preload) &&
	       globalParams->getPSUncompressPreloadedImages()) {
      s = NULL;
      maskUseLZW = maskUseRLE = gFalse;
      maskUseCompressed = gFalse;
      maskUseASCII = gFalse;
    } else {
      s = maskStr->getPSFilter(3, "  ");
      if (!s) {
	if (globalParams->getPSLZW()) {
	  maskUseLZW = gTrue;
	  maskUseRLE = gFalse;
	} else {
	  maskUseRLE = gTrue;
	  maskUseLZW = gFalse;
	}
	maskUseASCII = !(mode == psModeForm || inType3Char || preload);
	maskUseCompressed = gFalse;
      } else {
	maskUseLZW = maskUseRLE = gFalse;
	maskUseASCII = maskStr->isBinary() &&
	               !(mode == psModeForm || inType3Char || preload);
	maskUseCompressed = gTrue;
      }
    }
    maskFilters = new GString();
    if (maskUseASCII) {
      maskFilters->appendf("    /ASCII{0:s}Decode filter\n",
			   useASCIIHex ? "Hex" : "85");
    }
    if (maskUseLZW) {
      maskFilters->append("    /LZWDecode filter\n");
    } else if (maskUseRLE) {
      maskFilters->append("    /RunLengthDecode filter\n");
    }
    if (maskUseCompressed) {
      maskFilters->append(s);
    }
    if (s) {
      delete s;
    }
    if (mode == psModeForm || inType3Char || preload) {
      writePSFmt("MaskData_{0:d}_{1:d} pdfMaskInit\n",
		 ref->getRefNum(), ref->getRefGen());
    } else {
      writePS("currentfile\n");
      writePS(maskFilters->getCString());
      writePS("pdfMask\n");

      // add the ColorKeyToMask filter
      if (maskColors && colorMap && level == psLevel3Gray) {
	maskStr = new ColorKeyToMaskEncoder(str, width, height, colorMap,
					    maskColors);
      }

      // add LZWEncode/RunLengthEncode and ASCIIHex/85 encode filters
      if (maskUseCompressed) {
	maskStr = maskStr->getUndecodedStream();
      }
      if (maskUseLZW) {
	maskStr = new LZWEncoder(maskStr);
      } else if (maskUseRLE) {
	maskStr = new RunLengthEncoder(maskStr);
      }
      if (maskUseASCII) {
	if (useASCIIHex) {
	  maskStr = new ASCIIHexEncoder(maskStr);
	} else {
	  maskStr = new ASCII85Encoder(maskStr);
	}
      }

      // copy the stream data
      maskStr->reset();
      while ((n = maskStr->getBlock(buf, sizeof(buf))) > 0) {
	writePSBlock(buf, n);
      }
      maskStr->close();
      writePSChar('\n');
      writePS("%-EOD-\n");
      
      // delete encoders
      if (maskUseLZW || maskUseRLE || maskUseASCII) {
	delete maskStr;
      }
    }
  }

  // color space
  if (colorMap && level != psLevel3Gray) {
    dumpColorSpaceL2(state, colorMap->getColorSpace(), gFalse, gTrue, gFalse);
    writePS(" setcolorspace\n");
  }

  // set up the image data
  if (mode == psModeForm || inType3Char || preload) {
    if (inlineImg) {
      // create an array
      str2 = new FixedLengthEncoder(str, len);
      if (colorMap && level == psLevel3Gray) {
	str2 = new GrayRecoder(str2, width, height, colorMap);
      }
      if (globalParams->getPSLZW()) {
	str2 = new LZWEncoder(str2);
      } else {
	str2 = new RunLengthEncoder(str2);
      }
      if (useASCIIHex) {
	str2 = new ASCIIHexEncoder(str2);
      } else {
	str2 = new ASCII85Encoder(str2);
      }
      str2->reset();
      col = 0;
      writePS((char *)(useASCIIHex ? "[<" : "[<~"));
      do {
	do {
	  c = str2->getChar();
	} while (c == '\n' || c == '\r');
	if (c == (useASCIIHex ? '>' : '~') || c == EOF) {
	  break;
	}
	if (c == 'z') {
	  writePSChar((char)c);
	  ++col;
	} else {
	  writePSChar((char)c);
	  ++col;
	  for (i = 1; i <= (useASCIIHex ? 1 : 4); ++i) {
	    do {
	      c = str2->getChar();
	    } while (c == '\n' || c == '\r');
	    if (c == (useASCIIHex ? '>' : '~') || c == EOF) {
	      break;
	    }
	    writePSChar((char)c);
	    ++col;
	  }
	}
	// each line is: "<~...data...~><eol>"
	// so max data length = 255 - 6 = 249
	// chunks are 1 or 5 bytes each, so we have to stop at 245
	// but make it 240 just to be safe
	if (col > 240) {
	  writePS((char *)(useASCIIHex ? ">\n<" : "~>\n<~"));
	  col = 0;
	}
      } while (c != (useASCIIHex ? '>' : '~') && c != EOF);
      writePS((char *)(useASCIIHex ? ">\n" : "~>\n"));
      // add an extra entry because the LZWDecode/RunLengthDecode
      // filter may read past the end
      writePS("<>]\n");
      writePS("0\n");
      str2->close();
      delete str2;
    } else {
      // set up to use the array already created by setupImages()
      writePSFmt("ImData_{0:d}_{1:d} 0\n", ref->getRefNum(), ref->getRefGen());
    }
  }

  // explicit masking
  if (maskStr || (maskColors && colorMap && level == psLevel3Gray)) {
    writePS("<<\n  /ImageType 3\n");
    writePS("  /InterleaveType 3\n");
    writePS("  /DataDict\n");
  }

  // image (data) dictionary
  writePSFmt("<<\n  /ImageType {0:d}\n",
	     (maskColors && colorMap && level != psLevel3Gray) ? 4 : 1);

  // color key masking
  if (maskColors && colorMap && level != psLevel3Gray) {
    writePS("  /MaskColor [\n");
    numComps = colorMap->getNumPixelComps();
    for (i = 0; i < 2 * numComps; i += 2) {
      writePSFmt("    {0:d} {1:d}\n", maskColors[i], maskColors[i+1]);
    }
    writePS("  ]\n");
  }

  // width, height, matrix, bits per component
  writePSFmt("  /Width {0:d}\n", width);
  writePSFmt("  /Height {0:d}\n", height);
  writePSFmt("  /ImageMatrix [{0:d} 0 0 {1:d} 0 {2:d}]\n",
	     width, -height, height);
  if (colorMap && level == psLevel3Gray) {
    writePS("  /BitsPerComponent 8\n");
  } else {
    writePSFmt("  /BitsPerComponent {0:d}\n",
	       colorMap ? colorMap->getBits() : 1);
  }

  // decode 
  if (colorMap) {
    writePS("  /Decode [");
    if (level == psLevel3Sep &&
	colorMap->getColorSpace()->getMode() == csSeparation) {
      // this matches up with the code in the pdfImSep operator
      n = (1 << colorMap->getBits()) - 1;
      writePSFmt("{0:.4g} {1:.4g}", colorMap->getDecodeLow(0) * n,
		 colorMap->getDecodeHigh(0) * n);
    } else if (level == psLevel3Gray) {
      writePS("0 1");
    } else {
      numComps = colorMap->getNumPixelComps();
      for (i = 0; i < numComps; ++i) {
	if (i > 0) {
	  writePS(" ");
	}
	writePSFmt("{0:.4g} {1:.4g}", colorMap->getDecodeLow(i),
		   colorMap->getDecodeHigh(i));
      }
    }
    writePS("]\n");
  } else {
    writePSFmt("  /Decode [{0:d} {1:d}]\n", invert ? 1 : 0, invert ? 0 : 1);
  }

  // data source
  if (mode == psModeForm || inType3Char || preload) {
    writePS("  /DataSource { pdfImStr }\n");
  } else {
    writePS("  /DataSource currentfile\n");
  }

  // filters
  if ((mode == psModeForm || inType3Char || preload) &&
      globalParams->getPSUncompressPreloadedImages()) {
    s = NULL;
    useLZW = useRLE = gFalse;
    useCompressed = gFalse;
    useASCII = gFalse;
  } else {
    s = str->getPSFilter(3, "    ");
    if ((colorMap && level == psLevel3Gray) || inlineImg || !s) {
      if (globalParams->getPSLZW()) {
	useLZW = gTrue;
	useRLE = gFalse;
      } else {
	useRLE = gTrue;
	useLZW = gFalse;
      }
      useASCII = !(mode == psModeForm || inType3Char || preload);
      useCompressed = gFalse;
    } else {
      useLZW = useRLE = gFalse;
      useASCII = str->isBinary() &&
                 !(mode == psModeForm || inType3Char || preload);
      useCompressed = gTrue;
    }
  }
  if (useASCII) {
    writePSFmt("    /ASCII{0:s}Decode filter\n",
	       useASCIIHex ? "Hex" : "85");
  }
  if (useLZW) {
    writePS("    /LZWDecode filter\n");
  } else if (useRLE) {
    writePS("    /RunLengthDecode filter\n");
  }
  if (useCompressed) {
    writePS(s->getCString());
  }
  if (s) {
    delete s;
  }

  // end of image (data) dictionary
  writePS(">>\n");

  // explicit masking
  if (maskStr || (maskColors && colorMap && level == psLevel3Gray)) {
    writePS("  /MaskDict\n");
    writePS("<<\n");
    writePS("  /ImageType 1\n");
    writePSFmt("  /Width {0:d}\n", maskWidth);
    writePSFmt("  /Height {0:d}\n", maskHeight);
    writePSFmt("  /ImageMatrix [{0:d} 0 0 {1:d} 0 {2:d}]\n",
	       maskWidth, -maskHeight, maskHeight);
    writePS("  /BitsPerComponent 1\n");
    writePSFmt("  /Decode [{0:d} {1:d}]\n",
	       maskInvert ? 1 : 0, maskInvert ? 0 : 1);

    // mask data source
    if (mode == psModeForm || inType3Char || preload) {
      writePS("  /DataSource {pdfMaskSrc}\n");
      writePS(maskFilters->getCString());
    } else {
      writePS("  /DataSource maskStream\n");
    }
    delete maskFilters;

    writePS(">>\n");
    writePS(">>\n");
  }

  if (mode == psModeForm || inType3Char || preload) {

    // image command
    writePSFmt("{0:s}\n", colorMap ? "image" : "imagemask");

  } else {

    if (level == psLevel3Sep && colorMap &&
	colorMap->getColorSpace()->getMode() == csSeparation) {
      color.c[0] = gfxColorComp1;
      sepCS = (GfxSeparationColorSpace *)colorMap->getColorSpace();
      sepCS->getCMYK(&color, &cmyk, state->getRenderingIntent());
      writePSFmt("{0:.4g} {1:.4g} {2:.4g} {3:.4g} ({4:t}) pdfImSep\n",
		 colToDbl(cmyk.c), colToDbl(cmyk.m),
		 colToDbl(cmyk.y), colToDbl(cmyk.k),
		 sepCS->getName());
    } else {
      writePSFmt("{0:s}\n", colorMap ? "pdfIm" : "pdfImM");
    }

  }

  // get rid of the array and index
  if (mode == psModeForm || inType3Char || preload) {
    writePS("pop pop\n");

  // image data
  } else {

    // cut off inline image streams at appropriate length
    if (inlineImg) {
      str = new FixedLengthEncoder(str, len);
    } else if (useCompressed) {
      str = str->getUndecodedStream();
    }

    // recode to grayscale
    if (colorMap && level == psLevel3Gray) {
      str = new GrayRecoder(str, width, height, colorMap);
    }

    // add LZWEncode/RunLengthEncode and ASCIIHex/85 encode filters
    if (useLZW) {
      str = new LZWEncoder(str);
    } else if (useRLE) {
      str = new RunLengthEncoder(str);
    }
    if (useASCII) {
      if (useASCIIHex) {
	str = new ASCIIHexEncoder(str);
      } else {
	str = new ASCII85Encoder(str);
      }
    }

    // copy the stream data
    str->reset();
    while ((n = str->getBlock(buf, sizeof(buf))) > 0) {
      writePSBlock(buf, n);
    }
    str->close();

    // add newline and trailer to the end
    writePSChar('\n');
    writePS("%-EOD-\n");

    // delete encoders
    if (useLZW || useRLE || useASCII || inlineImg) {
      delete str;
    }
  }

  // close the mask stream
  if (maskStr || (maskColors && colorMap && level == psLevel3Gray)) {
    if (!(mode == psModeForm || inType3Char || preload)) {
      writePS("pdfMaskEnd\n");
    }
  }
}

void PSOutputDev::dumpColorSpaceL2(GfxState *state, GfxColorSpace *colorSpace,
				   GBool genXform, GBool updateColors,
				   GBool map01) {
  switch (colorSpace->getMode()) {
  case csDeviceGray:
    dumpDeviceGrayColorSpace((GfxDeviceGrayColorSpace *)colorSpace,
			     genXform, updateColors, map01);
    break;
  case csCalGray:
    dumpCalGrayColorSpace((GfxCalGrayColorSpace *)colorSpace,
			  genXform, updateColors, map01);
    break;
  case csDeviceRGB:
    dumpDeviceRGBColorSpace((GfxDeviceRGBColorSpace *)colorSpace,
			    genXform, updateColors, map01);
    break;
  case csCalRGB:
    dumpCalRGBColorSpace((GfxCalRGBColorSpace *)colorSpace,
			 genXform, updateColors, map01);
    break;
  case csDeviceCMYK:
    dumpDeviceCMYKColorSpace((GfxDeviceCMYKColorSpace *)colorSpace,
			     genXform, updateColors, map01);
    break;
  case csLab:
    dumpLabColorSpace((GfxLabColorSpace *)colorSpace,
		      genXform, updateColors, map01);
    break;
  case csICCBased:
    dumpICCBasedColorSpace(state, (GfxICCBasedColorSpace *)colorSpace,
			   genXform, updateColors, map01);
    break;
  case csIndexed:
    dumpIndexedColorSpace(state, (GfxIndexedColorSpace *)colorSpace,
			  genXform, updateColors, map01);
    break;
  case csSeparation:
    dumpSeparationColorSpace(state, (GfxSeparationColorSpace *)colorSpace,
			     genXform, updateColors, map01);
    break;
  case csDeviceN:
    if (level >= psLevel3) {
      dumpDeviceNColorSpaceL3(state, (GfxDeviceNColorSpace *)colorSpace,
			      genXform, updateColors, map01);
    } else {
      dumpDeviceNColorSpaceL2(state, (GfxDeviceNColorSpace *)colorSpace,
			      genXform, updateColors, map01);
    }
    break;
  case csPattern:
    //~ unimplemented
    break;
  }
}

void PSOutputDev::dumpDeviceGrayColorSpace(GfxDeviceGrayColorSpace *cs,
					   GBool genXform, GBool updateColors,
					   GBool map01) {
  writePS("/DeviceGray");
  if (genXform) {
    writePS(" {}");
  }
  if (updateColors) {
    processColors |= psProcessBlack;
  }
}

void PSOutputDev::dumpCalGrayColorSpace(GfxCalGrayColorSpace *cs,
					GBool genXform, GBool updateColors,
					GBool map01) {
  writePS("[/CIEBasedA <<\n");
  writePSFmt(" /DecodeA {{{0:.4g} exp}} bind\n", cs->getGamma());
  writePSFmt(" /MatrixA [{0:.4g} {1:.4g} {2:.4g}]\n",
	     cs->getWhiteX(), cs->getWhiteY(), cs->getWhiteZ());
  writePSFmt(" /WhitePoint [{0:.4g} {1:.4g} {2:.4g}]\n",
	     cs->getWhiteX(), cs->getWhiteY(), cs->getWhiteZ());
  writePSFmt(" /BlackPoint [{0:.4g} {1:.4g} {2:.4g}]\n",
	     cs->getBlackX(), cs->getBlackY(), cs->getBlackZ());
  writePS(">>]");
  if (genXform) {
    writePS(" {}");
  }
  if (updateColors) {
    processColors |= psProcessBlack;
  }
}

void PSOutputDev::dumpDeviceRGBColorSpace(GfxDeviceRGBColorSpace *cs,
					  GBool genXform, GBool updateColors,
					  GBool map01) {
  writePS("/DeviceRGB");
  if (genXform) {
    writePS(" {}");
  }
  if (updateColors) {
    processColors |= psProcessCMYK;
  }
}

void PSOutputDev::dumpCalRGBColorSpace(GfxCalRGBColorSpace *cs,
				       GBool genXform, GBool updateColors,
				       GBool map01) {
  writePS("[/CIEBasedABC <<\n");
  writePSFmt(" /DecodeABC [{{{0:.4g} exp}} bind {{{1:.4g} exp}} bind {{{2:.4g} exp}} bind]\n",
	     cs->getGammaR(), cs->getGammaG(), cs->getGammaB());
  writePSFmt(" /MatrixABC [{0:.4g} {1:.4g} {2:.4g} {3:.4g} {4:.4g} {5:.4g} {6:.4g} {7:.4g} {8:.4g}]\n",
	     cs->getMatrix()[0], cs->getMatrix()[1], cs->getMatrix()[2],
	     cs->getMatrix()[3], cs->getMatrix()[4], cs->getMatrix()[5],
	     cs->getMatrix()[6], cs->getMatrix()[7], cs->getMatrix()[8]);
  writePSFmt(" /WhitePoint [{0:.4g} {1:.4g} {2:.4g}]\n",
	     cs->getWhiteX(), cs->getWhiteY(), cs->getWhiteZ());
  writePSFmt(" /BlackPoint [{0:.4g} {1:.4g} {2:.4g}]\n",
	     cs->getBlackX(), cs->getBlackY(), cs->getBlackZ());
  writePS(">>]");
  if (genXform) {
    writePS(" {}");
  }
  if (updateColors) {
    processColors |= psProcessCMYK;
  }
}

void PSOutputDev::dumpDeviceCMYKColorSpace(GfxDeviceCMYKColorSpace *cs,
					   GBool genXform, GBool updateColors,
					   GBool map01) {
  writePS("/DeviceCMYK");
  if (genXform) {
    writePS(" {}");
  }
  if (updateColors) {
    processColors |= psProcessCMYK;
  }
}

void PSOutputDev::dumpLabColorSpace(GfxLabColorSpace *cs,
				    GBool genXform, GBool updateColors,
				    GBool map01) {
  writePS("[/CIEBasedABC <<\n");
  if (map01) {
    writePS(" /RangeABC [0 1 0 1 0 1]\n");
    writePSFmt(" /DecodeABC [{{100 mul 16 add 116 div}} bind {{{0:.4g} mul {1:.4g} add}} bind {{{2:.4g} mul {3:.4g} add}} bind]\n",
	       (cs->getAMax() - cs->getAMin()) / 500.0,
	       cs->getAMin() / 500.0,
	       (cs->getBMax() - cs->getBMin()) / 200.0,
	       cs->getBMin() / 200.0);
  } else {
    writePSFmt(" /RangeABC [0 100 {0:.4g} {1:.4g} {2:.4g} {3:.4g}]\n",
	       cs->getAMin(), cs->getAMax(),
	       cs->getBMin(), cs->getBMax());
    writePS(" /DecodeABC [{16 add 116 div} bind {500 div} bind {200 div} bind]\n");
  }
  writePS(" /MatrixABC [1 1 1 1 0 0 0 0 -1]\n");
  writePS(" /DecodeLMN\n");
  writePS("   [{dup 6 29 div ge {dup dup mul mul}\n");
  writePSFmt("     {{4 29 div sub 108 841 div mul }} ifelse {0:.4g} mul}} bind\n",
	     cs->getWhiteX());
  writePS("    {dup 6 29 div ge {dup dup mul mul}\n");
  writePSFmt("     {{4 29 div sub 108 841 div mul }} ifelse {0:.4g} mul}} bind\n",
	     cs->getWhiteY());
  writePS("    {dup 6 29 div ge {dup dup mul mul}\n");
  writePSFmt("     {{4 29 div sub 108 841 div mul }} ifelse {0:.4g} mul}} bind]\n",
	     cs->getWhiteZ());
  writePSFmt(" /WhitePoint [{0:.4g} {1:.4g} {2:.4g}]\n",
	     cs->getWhiteX(), cs->getWhiteY(), cs->getWhiteZ());
  writePSFmt(" /BlackPoint [{0:.4g} {1:.4g} {2:.4g}]\n",
	     cs->getBlackX(), cs->getBlackY(), cs->getBlackZ());
  writePS(">>]");
  if (genXform) {
    writePS(" {}");
  }
  if (updateColors) {
    processColors |= psProcessCMYK;
  }
}

void PSOutputDev::dumpICCBasedColorSpace(GfxState *state,
					 GfxICCBasedColorSpace *cs,
					 GBool genXform, GBool updateColors,
					 GBool map01) {
  // there is no transform function to the alternate color space, so
  // we can use it directly
  dumpColorSpaceL2(state, cs->getAlt(), genXform, updateColors, gFalse);
}


void PSOutputDev::dumpIndexedColorSpace(GfxState *state,
					GfxIndexedColorSpace *cs,
					GBool genXform, GBool updateColors,
					GBool map01) {
  GfxColorSpace *baseCS;
  GfxLabColorSpace *labCS;
  Guchar *lookup, *p;
  double x[gfxColorMaxComps], y[gfxColorMaxComps];
  double low[gfxColorMaxComps], range[gfxColorMaxComps];
  GfxColor color;
  GfxCMYK cmyk;
  Function *func;
  int n, numComps, numAltComps;
  int byte;
  int i, j, k;

  baseCS = cs->getBase();
  writePS("[/Indexed ");
  dumpColorSpaceL2(state, baseCS, gFalse, updateColors, gTrue);
  n = cs->getIndexHigh();
  numComps = baseCS->getNComps();
  lookup = cs->getLookup();
  writePSFmt(" {0:d} <\n", n);
  if (baseCS->getMode() == csDeviceN && level < psLevel3) {
    func = ((GfxDeviceNColorSpace *)baseCS)->getTintTransformFunc();
    baseCS->getDefaultRanges(low, range, cs->getIndexHigh());
    if (((GfxDeviceNColorSpace *)baseCS)->getAlt()->getMode() == csLab) {
      labCS = (GfxLabColorSpace *)((GfxDeviceNColorSpace *)baseCS)->getAlt();
    } else {
      labCS = NULL;
    }
    numAltComps = ((GfxDeviceNColorSpace *)baseCS)->getAlt()->getNComps();
    p = lookup;
    for (i = 0; i <= n; i += 8) {
      writePS("  ");
      for (j = i; j < i+8 && j <= n; ++j) {
	for (k = 0; k < numComps; ++k) {
	  x[k] = low[k] + (*p++ / 255.0) * range[k];
	}
	func->transform(x, y);
	if (labCS) {
	  y[0] /= 100.0;
	  y[1] = (y[1] - labCS->getAMin()) /
	         (labCS->getAMax() - labCS->getAMin());
	  y[2] = (y[2] - labCS->getBMin()) /
	         (labCS->getBMax() - labCS->getBMin());
	}
	for (k = 0; k < numAltComps; ++k) {
	  byte = (int)(y[k] * 255 + 0.5);
	  if (byte < 0) {
	    byte = 0;
	  } else if (byte > 255) {
	    byte = 255;
	  }
	  writePSFmt("{0:02x}", byte);
	}
	if (updateColors) {
	  color.c[0] = dblToCol(j);
	  cs->getCMYK(&color, &cmyk, state->getRenderingIntent());
	  addProcessColor(colToDbl(cmyk.c), colToDbl(cmyk.m),
			  colToDbl(cmyk.y), colToDbl(cmyk.k));
	}
      }
      writePS("\n");
    }
  } else {
    for (i = 0; i <= n; i += 8) {
      writePS("  ");
      for (j = i; j < i+8 && j <= n; ++j) {
	for (k = 0; k < numComps; ++k) {
	  writePSFmt("{0:02x}", lookup[j * numComps + k]);
	}
	if (updateColors) {
	  color.c[0] = dblToCol(j);
	  cs->getCMYK(&color, &cmyk, state->getRenderingIntent());
	  addProcessColor(colToDbl(cmyk.c), colToDbl(cmyk.m),
			  colToDbl(cmyk.y), colToDbl(cmyk.k));
	}
      }
      writePS("\n");
    }
  }
  writePS(">]");
  if (genXform) {
    writePS(" {}");
  }
}

void PSOutputDev::dumpSeparationColorSpace(GfxState *state,
					   GfxSeparationColorSpace *cs,
					   GBool genXform, GBool updateColors,
					   GBool map01) {
  writePS("[/Separation ");
  writePSString(cs->getName());
  writePS(" ");
  dumpColorSpaceL2(state, cs->getAlt(), gFalse, gFalse, gFalse);
  writePS("\n");
  cvtFunction(cs->getFunc());
  writePS("]");
  if (genXform) {
    writePS(" {}");
  }
  if (updateColors) {
    addCustomColor(state, cs);
  }
}

void PSOutputDev::dumpDeviceNColorSpaceL2(GfxState *state,
					  GfxDeviceNColorSpace *cs,
					  GBool genXform, GBool updateColors,
					  GBool map01) {
  dumpColorSpaceL2(state, cs->getAlt(), gFalse, updateColors, map01);
  if (genXform) {
    writePS(" ");
    cvtFunction(cs->getTintTransformFunc());
  }
}

void PSOutputDev::dumpDeviceNColorSpaceL3(GfxState *state,
					  GfxDeviceNColorSpace *cs,
					  GBool genXform, GBool updateColors,
					  GBool map01) {
  GString *tint;
  int i;

  writePS("[/DeviceN [\n");
  for (i = 0; i < cs->getNComps(); ++i) {
    writePSString(cs->getColorantName(i));
    writePS("\n");
  }
  writePS("]\n");
  if ((tint = createDeviceNTintFunc(cs))) {
    writePS("/DeviceCMYK\n");
    writePS(tint->getCString());
    delete tint;
  } else {
    dumpColorSpaceL2(state, cs->getAlt(), gFalse, gFalse, gFalse);
    writePS("\n");
    cvtFunction(cs->getTintTransformFunc());
  }
  writePS("]");
  if (genXform) {
    writePS(" {}");
  }
  if (updateColors) {
    addCustomColors(state, cs);
  }
}

// If the DeviceN color space has a Colorants dictionary, and all of
// the colorants are one of: "None", "Cyan", "Magenta", "Yellow",
// "Black", or have an entry in the Colorants dict that maps to
// DeviceCMYK, then build a new tint function; else use the existing
// tint function.
GString *PSOutputDev::createDeviceNTintFunc(GfxDeviceNColorSpace *cs) {
  Object *attrs;
  Object colorants, sepCSObj, funcObj, obj1;
  GString *name;
  Function *func;
  double sepIn;
  double cmyk[gfxColorMaxComps][4];
  GString *tint;
  GBool first;
  int i, j;

  attrs = cs->getAttrs();
  if (!attrs->isDict()) {
    return NULL;
  }
  if (!attrs->dictLookup("Colorants", &colorants)->isDict()) {
    colorants.free();
    return NULL;
  }
  for (i = 0; i < cs->getNComps(); ++i) {
    name = cs->getColorantName(i);
    if (!name->cmp("None")) {
      cmyk[i][0] = cmyk[i][1] = cmyk[i][2] = cmyk[i][3] = 0;
    } else if (!name->cmp("Cyan")) {
      cmyk[i][1] = cmyk[i][2] = cmyk[i][3] = 0;
      cmyk[i][0] = 1;
    } else if (!name->cmp("Magenta")) {
      cmyk[i][0] = cmyk[i][2] = cmyk[i][3] = 0;
      cmyk[i][1] = 1;
    } else if (!name->cmp("Yellow")) {
      cmyk[i][0] = cmyk[i][1] = cmyk[i][3] = 0;
      cmyk[i][2] = 1;
    } else if (!name->cmp("Black")) {
      cmyk[i][0] = cmyk[i][1] = cmyk[i][2] = 0;
      cmyk[i][3] = 1;
    } else {
      colorants.dictLookup(name->getCString(), &sepCSObj);
      if (!sepCSObj.isArray() || sepCSObj.arrayGetLength() != 4) {
	sepCSObj.free();
	colorants.free();
	return NULL;
      }
      if (!sepCSObj.arrayGet(0, &obj1)->isName("Separation")) { 
	obj1.free();
	sepCSObj.free();
	colorants.free();
	return NULL;
      }
      obj1.free();
      if (!sepCSObj.arrayGet(2, &obj1)->isName("DeviceCMYK")) { 
	obj1.free();
	sepCSObj.free();
	colorants.free();
	return NULL;
      }
      obj1.free();
      sepCSObj.arrayGet(3, &funcObj);
      if (!(func = Function::parse(&funcObj))) {
	funcObj.free();
	sepCSObj.free();
	colorants.free();
	return NULL;
      }
      funcObj.free();
      if (func->getInputSize() != 1 || func->getOutputSize() != 4) {
	delete func;
	sepCSObj.free();
	colorants.free();
	return NULL;
      }
      sepIn = 1;
      func->transform(&sepIn, cmyk[i]);
      delete func;
      sepCSObj.free();
    }
  }
  colorants.free();

  tint = new GString();
  tint->append("{\n");
  for (j = 0; j < 4; ++j) {  // C, M, Y, K
    first = gTrue;
    for (i = 0; i < cs->getNComps(); ++i) {
      if (cmyk[i][j] != 0) {
	tint->appendf("{0:d} index {1:.4f} mul{2:s}\n",
		      j + cs->getNComps() - 1 - i, cmyk[i][j],
		      first ? "" : " add");
	first = gFalse;
      }
    }
    if (first) {
      tint->append("0\n");
    }
  }
  tint->appendf("{0:d} 4 roll\n", cs->getNComps() + 4);
  for (i = 0; i < cs->getNComps(); ++i) {
    tint->append("pop\n");
  }
  tint->append("}\n");

  return tint;
}

#if OPI_SUPPORT
void PSOutputDev::opiBegin(GfxState *state, Dict *opiDict) {
  Object dict;

  if (globalParams->getPSOPI()) {
    opiDict->lookup("2.0", &dict);
    if (dict.isDict()) {
      opiBegin20(state, dict.getDict());
      dict.free();
    } else {
      dict.free();
      opiDict->lookup("1.3", &dict);
      if (dict.isDict()) {
	opiBegin13(state, dict.getDict());
      }
      dict.free();
    }
  }
}

void PSOutputDev::opiBegin20(GfxState *state, Dict *dict) {
  Object obj1, obj2, obj3, obj4;
  double width, height, left, right, top, bottom;
  int w, h;
  int i;

  writePS("%%BeginOPI: 2.0\n");
  writePS("%%Distilled\n");

  dict->lookup("F", &obj1);
  if (getFileSpec(&obj1, &obj2)) {
    writePSFmt("%%ImageFileName: {0:t}\n", obj2.getString());
    obj2.free();
  }
  obj1.free();

  dict->lookup("MainImage", &obj1);
  if (obj1.isString()) {
    writePSFmt("%%MainImage: {0:t}\n", obj1.getString());
  }
  obj1.free();

  //~ ignoring 'Tags' entry
  //~ need to use writePSString() and deal with >255-char lines

  dict->lookup("Size", &obj1);
  if (obj1.isArray() && obj1.arrayGetLength() == 2) {
    obj1.arrayGet(0, &obj2);
    width = obj2.getNum();
    obj2.free();
    obj1.arrayGet(1, &obj2);
    height = obj2.getNum();
    obj2.free();
    writePSFmt("%%ImageDimensions: {0:.6g} {1:.6g}\n", width, height);
  }
  obj1.free();

  dict->lookup("CropRect", &obj1);
  if (obj1.isArray() && obj1.arrayGetLength() == 4) {
    obj1.arrayGet(0, &obj2);
    left = obj2.getNum();
    obj2.free();
    obj1.arrayGet(1, &obj2);
    top = obj2.getNum();
    obj2.free();
    obj1.arrayGet(2, &obj2);
    right = obj2.getNum();
    obj2.free();
    obj1.arrayGet(3, &obj2);
    bottom = obj2.getNum();
    obj2.free();
    writePSFmt("%%ImageCropRect: {0:.6g} {1:.6g} {2:.6g} {3:.6g}\n",
	       left, top, right, bottom);
  }
  obj1.free();

  dict->lookup("Overprint", &obj1);
  if (obj1.isBool()) {
    writePSFmt("%%ImageOverprint: {0:s}\n", obj1.getBool() ? "true" : "false");
  }
  obj1.free();

  dict->lookup("Inks", &obj1);
  if (obj1.isName()) {
    writePSFmt("%%ImageInks: {0:s}\n", obj1.getName());
  } else if (obj1.isArray() && obj1.arrayGetLength() >= 1) {
    obj1.arrayGet(0, &obj2);
    if (obj2.isName()) {
      writePSFmt("%%ImageInks: {0:s} {1:d}",
		 obj2.getName(), (obj1.arrayGetLength() - 1) / 2);
      for (i = 1; i+1 < obj1.arrayGetLength(); i += 2) {
	obj1.arrayGet(i, &obj3);
	obj1.arrayGet(i+1, &obj4);
	if (obj3.isString() && obj4.isNum()) {
	  writePS(" ");
	  writePSString(obj3.getString());
	  writePSFmt(" {0:.4g}", obj4.getNum());
	}
	obj3.free();
	obj4.free();
      }
      writePS("\n");
    }
    obj2.free();
  }
  obj1.free();

  writePS("gsave\n");

  writePS("%%BeginIncludedImage\n");

  dict->lookup("IncludedImageDimensions", &obj1);
  if (obj1.isArray() && obj1.arrayGetLength() == 2) {
    obj1.arrayGet(0, &obj2);
    w = obj2.getInt();
    obj2.free();
    obj1.arrayGet(1, &obj2);
    h = obj2.getInt();
    obj2.free();
    writePSFmt("%%IncludedImageDimensions: {0:d} {1:d}\n", w, h);
  }
  obj1.free();

  dict->lookup("IncludedImageQuality", &obj1);
  if (obj1.isNum()) {
    writePSFmt("%%IncludedImageQuality: {0:.4g}\n", obj1.getNum());
  }
  obj1.free();

  ++opi20Nest;
}

void PSOutputDev::opiBegin13(GfxState *state, Dict *dict) {
  Object obj1, obj2;
  int left, right, top, bottom, samples, bits, width, height;
  double c, m, y, k;
  double llx, lly, ulx, uly, urx, ury, lrx, lry;
  double tllx, tlly, tulx, tuly, turx, tury, tlrx, tlry;
  double horiz, vert;
  int i, j;

  writePS("save\n");
  writePS("/opiMatrix2 matrix currentmatrix def\n");
  writePS("opiMatrix setmatrix\n");

  dict->lookup("F", &obj1);
  if (getFileSpec(&obj1, &obj2)) {
    writePSFmt("%ALDImageFileName: {0:t}\n", obj2.getString());
    obj2.free();
  }
  obj1.free();

  dict->lookup("CropRect", &obj1);
  if (obj1.isArray() && obj1.arrayGetLength() == 4) {
    obj1.arrayGet(0, &obj2);
    left = obj2.getInt();
    obj2.free();
    obj1.arrayGet(1, &obj2);
    top = obj2.getInt();
    obj2.free();
    obj1.arrayGet(2, &obj2);
    right = obj2.getInt();
    obj2.free();
    obj1.arrayGet(3, &obj2);
    bottom = obj2.getInt();
    obj2.free();
    writePSFmt("%ALDImageCropRect: {0:d} {1:d} {2:d} {3:d}\n",
	       left, top, right, bottom);
  }
  obj1.free();

  dict->lookup("Color", &obj1);
  if (obj1.isArray() && obj1.arrayGetLength() == 5) {
    obj1.arrayGet(0, &obj2);
    c = obj2.getNum();
    obj2.free();
    obj1.arrayGet(1, &obj2);
    m = obj2.getNum();
    obj2.free();
    obj1.arrayGet(2, &obj2);
    y = obj2.getNum();
    obj2.free();
    obj1.arrayGet(3, &obj2);
    k = obj2.getNum();
    obj2.free();
    obj1.arrayGet(4, &obj2);
    if (obj2.isString()) {
      writePSFmt("%ALDImageColor: {0:.4g} {1:.4g} {2:.4g} {3:.4g} ",
		 c, m, y, k);
      writePSString(obj2.getString());
      writePS("\n");
    }
    obj2.free();
  }
  obj1.free();

  dict->lookup("ColorType", &obj1);
  if (obj1.isName()) {
    writePSFmt("%ALDImageColorType: {0:s}\n", obj1.getName());
  }
  obj1.free();

  //~ ignores 'Comments' entry
  //~ need to handle multiple lines

  dict->lookup("CropFixed", &obj1);
  if (obj1.isArray()) {
    obj1.arrayGet(0, &obj2);
    ulx = obj2.getNum();
    obj2.free();
    obj1.arrayGet(1, &obj2);
    uly = obj2.getNum();
    obj2.free();
    obj1.arrayGet(2, &obj2);
    lrx = obj2.getNum();
    obj2.free();
    obj1.arrayGet(3, &obj2);
    lry = obj2.getNum();
    obj2.free();
    writePSFmt("%ALDImageCropFixed: {0:.4g} {1:.4g} {2:.4g} {3:.4g}\n",
	       ulx, uly, lrx, lry);
  }
  obj1.free();

  dict->lookup("GrayMap", &obj1);
  if (obj1.isArray()) {
    writePS("%ALDImageGrayMap:");
    for (i = 0; i < obj1.arrayGetLength(); i += 16) {
      if (i > 0) {
	writePS("\n%%+");
      }
      for (j = 0; j < 16 && i+j < obj1.arrayGetLength(); ++j) {
	obj1.arrayGet(i+j, &obj2);
	writePSFmt(" {0:d}", obj2.getInt());
	obj2.free();
      }
    }
    writePS("\n");
  }
  obj1.free();

  dict->lookup("ID", &obj1);
  if (obj1.isString()) {
    writePSFmt("%ALDImageID: {0:t}\n", obj1.getString());
  }
  obj1.free();

  dict->lookup("ImageType", &obj1);
  if (obj1.isArray() && obj1.arrayGetLength() == 2) {
    obj1.arrayGet(0, &obj2);
    samples = obj2.getInt();
    obj2.free();
    obj1.arrayGet(1, &obj2);
    bits = obj2.getInt();
    obj2.free();
    writePSFmt("%ALDImageType: {0:d} {1:d}\n", samples, bits);
  }
  obj1.free();

  dict->lookup("Overprint", &obj1);
  if (obj1.isBool()) {
    writePSFmt("%ALDImageOverprint: {0:s}\n",
	       obj1.getBool() ? "true" : "false");
  }
  obj1.free();

  dict->lookup("Position", &obj1);
  if (obj1.isArray() && obj1.arrayGetLength() == 8) {
    obj1.arrayGet(0, &obj2);
    llx = obj2.getNum();
    obj2.free();
    obj1.arrayGet(1, &obj2);
    lly = obj2.getNum();
    obj2.free();
    obj1.arrayGet(2, &obj2);
    ulx = obj2.getNum();
    obj2.free();
    obj1.arrayGet(3, &obj2);
    uly = obj2.getNum();
    obj2.free();
    obj1.arrayGet(4, &obj2);
    urx = obj2.getNum();
    obj2.free();
    obj1.arrayGet(5, &obj2);
    ury = obj2.getNum();
    obj2.free();
    obj1.arrayGet(6, &obj2);
    lrx = obj2.getNum();
    obj2.free();
    obj1.arrayGet(7, &obj2);
    lry = obj2.getNum();
    obj2.free();
    opiTransform(state, llx, lly, &tllx, &tlly);
    opiTransform(state, ulx, uly, &tulx, &tuly);
    opiTransform(state, urx, ury, &turx, &tury);
    opiTransform(state, lrx, lry, &tlrx, &tlry);
    writePSFmt("%ALDImagePosition: {0:.4g} {1:.4g} {2:.4g} {3:.4g} {4:.4g} {5:.4g} {6:.4g} {7:.4g}\n",
	       tllx, tlly, tulx, tuly, turx, tury, tlrx, tlry);
    obj2.free();
  }
  obj1.free();

  dict->lookup("Resolution", &obj1);
  if (obj1.isArray() && obj1.arrayGetLength() == 2) {
    obj1.arrayGet(0, &obj2);
    horiz = obj2.getNum();
    obj2.free();
    obj1.arrayGet(1, &obj2);
    vert = obj2.getNum();
    obj2.free();
    writePSFmt("%ALDImageResoution: {0:.4g} {1:.4g}\n", horiz, vert);
    obj2.free();
  }
  obj1.free();

  dict->lookup("Size", &obj1);
  if (obj1.isArray() && obj1.arrayGetLength() == 2) {
    obj1.arrayGet(0, &obj2);
    width = obj2.getInt();
    obj2.free();
    obj1.arrayGet(1, &obj2);
    height = obj2.getInt();
    obj2.free();
    writePSFmt("%ALDImageDimensions: {0:d} {1:d}\n", width, height);
  }
  obj1.free();

  //~ ignoring 'Tags' entry
  //~ need to use writePSString() and deal with >255-char lines

  dict->lookup("Tint", &obj1);
  if (obj1.isNum()) {
    writePSFmt("%ALDImageTint: {0:.4g}\n", obj1.getNum());
  }
  obj1.free();

  dict->lookup("Transparency", &obj1);
  if (obj1.isBool()) {
    writePSFmt("%ALDImageTransparency: {0:s}\n",
	       obj1.getBool() ? "true" : "false");
  }
  obj1.free();

  writePS("%%BeginObject: image\n");
  writePS("opiMatrix2 setmatrix\n");
  ++opi13Nest;
}

// Convert PDF user space coordinates to PostScript default user space
// coordinates.  This has to account for both the PDF CTM and the
// PSOutputDev page-fitting transform.
void PSOutputDev::opiTransform(GfxState *state, double x0, double y0,
			       double *x1, double *y1) {
  double t;

  state->transform(x0, y0, x1, y1);
  *x1 += tx;
  *y1 += ty;
  if (rotate == 90) {
    t = *x1;
    *x1 = -*y1;
    *y1 = t;
  } else if (rotate == 180) {
    *x1 = -*x1;
    *y1 = -*y1;
  } else if (rotate == 270) {
    t = *x1;
    *x1 = *y1;
    *y1 = -t;
  }
  *x1 *= xScale;
  *y1 *= yScale;
}

void PSOutputDev::opiEnd(GfxState *state, Dict *opiDict) {
  Object dict;

  if (globalParams->getPSOPI()) {
    opiDict->lookup("2.0", &dict);
    if (dict.isDict()) {
      writePS("%%EndIncludedImage\n");
      writePS("%%EndOPI\n");
      writePS("grestore\n");
      --opi20Nest;
      dict.free();
    } else {
      dict.free();
      opiDict->lookup("1.3", &dict);
      if (dict.isDict()) {
	writePS("%%EndObject\n");
	writePS("restore\n");
	--opi13Nest;
      }
      dict.free();
    }
  }
}

GBool PSOutputDev::getFileSpec(Object *fileSpec, Object *fileName) {
  if (fileSpec->isString()) {
    fileSpec->copy(fileName);
    return gTrue;
  }
  if (fileSpec->isDict()) {
    fileSpec->dictLookup("DOS", fileName);
    if (fileName->isString()) {
      return gTrue;
    }
    fileName->free();
    fileSpec->dictLookup("Mac", fileName);
    if (fileName->isString()) {
      return gTrue;
    }
    fileName->free();
    fileSpec->dictLookup("Unix", fileName);
    if (fileName->isString()) {
      return gTrue;
    }
    fileName->free();
    fileSpec->dictLookup("F", fileName);
    if (fileName->isString()) {
      return gTrue;
    }
    fileName->free();
  }
  return gFalse;
}
#endif // OPI_SUPPORT

void PSOutputDev::type3D0(GfxState *state, double wx, double wy) {
  writePSFmt("{0:.6g} {1:.6g} setcharwidth\n", wx, wy);
  writePS("q\n");
  t3NeedsRestore = gTrue;
  noStateChanges = gFalse;
}

void PSOutputDev::type3D1(GfxState *state, double wx, double wy,
			  double llx, double lly, double urx, double ury) {
  if (t3String) {
    error(errSyntaxError, -1, "Multiple 'd1' operators in Type 3 CharProc");
    return;
  }
  t3WX = wx;
  t3WY = wy;
  t3LLX = llx;
  t3LLY = lly;
  t3URX = urx;
  t3URY = ury;
  t3String = new GString();
  writePS("q\n");
  t3FillColorOnly = gTrue;
  t3Cacheable = gTrue;
  t3NeedsRestore = gTrue;
  noStateChanges = gFalse;
}

void PSOutputDev::drawForm(Ref id) {
  writePSFmt("f_{0:d}_{1:d}\n", id.num, id.gen);
  noStateChanges = gFalse;
}

void PSOutputDev::psXObject(Stream *psStream, Stream *level1Stream) {
  Stream *str;
  char buf[4096];
  int n;

  if ((level == psLevel1 || level == psLevel1Sep) && level1Stream) {
    str = level1Stream;
  } else {
    str = psStream;
  }
  str->reset();
  while ((n = str->getBlock(buf, sizeof(buf))) > 0) {
    writePSBlock(buf, n);
  }
  str->close();
  noStateChanges = gFalse;
}

//~ can nextFunc be reset to 0 -- maybe at the start of each page?
//~   or maybe at the start of each color space / pattern?
void PSOutputDev::cvtFunction(Function *func) {
  SampledFunction *func0;
  ExponentialFunction *func2;
  StitchingFunction *func3;
  PostScriptFunction *func4;
  int thisFunc, m, n, nSamples, i, j, k;

  switch (func->getType()) {

  case -1:			// identity
    writePS("{}\n");
    break;

  case 0:			// sampled
    func0 = (SampledFunction *)func;
    thisFunc = nextFunc++;
    m = func0->getInputSize();
    n = func0->getOutputSize();
    nSamples = n;
    for (i = 0; i < m; ++i) {
      nSamples *= func0->getSampleSize(i);
    }
    writePSFmt("/xpdfSamples{0:d} [\n", thisFunc);
    for (i = 0; i < nSamples; ++i) {
      writePSFmt("{0:.6g}\n", func0->getSamples()[i]);
    }
    writePS("] def\n");
    writePSFmt("{{ {0:d} array {1:d} array {2:d} 2 roll\n", 2*m, m, m+2);
    // [e01] [efrac] x0 x1 ... xm-1
    for (i = m-1; i >= 0; --i) {
      // [e01] [efrac] x0 x1 ... xi
      writePSFmt("{0:.6g} sub {1:.6g} mul {2:.6g} add\n",
	      func0->getDomainMin(i),
	      (func0->getEncodeMax(i) - func0->getEncodeMin(i)) /
	        (func0->getDomainMax(i) - func0->getDomainMin(i)),
	      func0->getEncodeMin(i));
      // [e01] [efrac] x0 x1 ... xi-1 xi'
      writePSFmt("dup 0 lt {{ pop 0 }} {{ dup {0:d} gt {{ pop {1:d} }} if }} ifelse\n",
		 func0->getSampleSize(i) - 1, func0->getSampleSize(i) - 1);
      // [e01] [efrac] x0 x1 ... xi-1 xi'
      writePS("dup floor cvi exch dup ceiling cvi exch 2 index sub\n");
      // [e01] [efrac] x0 x1 ... xi-1 floor(xi') ceiling(xi') xi'-floor(xi')
      writePSFmt("{0:d} index {1:d} 3 2 roll put\n", i+3, i);
      // [e01] [efrac] x0 x1 ... xi-1 floor(xi') ceiling(xi')
      writePSFmt("{0:d} index {1:d} 3 2 roll put\n", i+3, 2*i+1);
      // [e01] [efrac] x0 x1 ... xi-1 floor(xi')
      writePSFmt("{0:d} index {1:d} 3 2 roll put\n", i+2, 2*i);
      // [e01] [efrac] x0 x1 ... xi-1
    }
    // [e01] [efrac]
    for (i = 0; i < n; ++i) {
      // [e01] [efrac] y(0) ... y(i-1)
      for (j = 0; j < (1<<m); ++j) {
	// [e01] [efrac] y(0) ... y(i-1) s(0) s(1) ... s(j-1)
	writePSFmt("xpdfSamples{0:d}\n", thisFunc);
	k = m - 1;
	writePSFmt("{0:d} index {1:d} get\n", i+j+2, 2 * k + ((j >> k) & 1));
	for (k = m - 2; k >= 0; --k) {
	  writePSFmt("{0:d} mul {1:d} index {2:d} get add\n",
		     func0->getSampleSize(k),
		     i + j + 3,
		     2 * k + ((j >> k) & 1));
	}
	if (n > 1) {
	  writePSFmt("{0:d} mul {1:d} add ", n, i);
	}
	writePS("get\n");
      }
      // [e01] [efrac] y(0) ... y(i-1) s(0) s(1) ... s(2^m-1)
      for (j = 0; j < m; ++j) {
	// [e01] [efrac] y(0) ... y(i-1) s(0) s(1) ... s(2^(m-j)-1)
	for (k = 0; k < (1 << (m - j)); k += 2) {
	  // [e01] [efrac] y(0) ... y(i-1) <k/2 s' values> <2^(m-j)-k s values>
	  writePSFmt("{0:d} index {1:d} get dup\n",
		     i + k/2 + (1 << (m-j)) - k, j);
	  writePS("3 2 roll mul exch 1 exch sub 3 2 roll mul add\n");
	  writePSFmt("{0:d} 1 roll\n", k/2 + (1 << (m-j)) - k - 1);
	}
	// [e01] [efrac] s'(0) s'(1) ... s(2^(m-j-1)-1)
      }
      // [e01] [efrac] y(0) ... y(i-1) s
      writePSFmt("{0:.6g} mul {1:.6g} add\n",
		 func0->getDecodeMax(i) - func0->getDecodeMin(i),
		 func0->getDecodeMin(i));
      writePSFmt("dup {0:.6g} lt {{ pop {1:.6g} }} {{ dup {2:.6g} gt {{ pop {3:.6g} }} if }} ifelse\n",
		 func0->getRangeMin(i), func0->getRangeMin(i),
		 func0->getRangeMax(i), func0->getRangeMax(i));
      // [e01] [efrac] y(0) ... y(i-1) y(i)
    }
    // [e01] [efrac] y(0) ... y(n-1)
    writePSFmt("{0:d} {1:d} roll pop pop }}\n", n+2, n);
    break;

  case 2:			// exponential
    func2 = (ExponentialFunction *)func;
    n = func2->getOutputSize();
    writePSFmt("{{ dup {0:.6g} lt {{ pop {1:.6g} }} {{ dup {2:.6g} gt {{ pop {3:.6g} }} if }} ifelse\n",
	       func2->getDomainMin(0), func2->getDomainMin(0),
	       func2->getDomainMax(0), func2->getDomainMax(0));
    // x
    for (i = 0; i < n; ++i) {
      // x y(0) .. y(i-1)
      writePSFmt("{0:d} index {1:.6g} exp {2:.6g} mul {3:.6g} add\n",
		 i, func2->getE(), func2->getC1()[i] - func2->getC0()[i],
		 func2->getC0()[i]);
      if (func2->getHasRange()) {
	writePSFmt("dup {0:.6g} lt {{ pop {1:.6g} }} {{ dup {2:.6g} gt {{ pop {3:.6g} }} if }} ifelse\n",
		   func2->getRangeMin(i), func2->getRangeMin(i),
		   func2->getRangeMax(i), func2->getRangeMax(i));
      }
    }
    // x y(0) .. y(n-1)
    writePSFmt("{0:d} {1:d} roll pop }}\n", n+1, n);
    break;

  case 3:			// stitching
    func3 = (StitchingFunction *)func;
    thisFunc = nextFunc++;
    for (i = 0; i < func3->getNumFuncs(); ++i) {
      cvtFunction(func3->getFunc(i));
      writePSFmt("/xpdfFunc{0:d}_{1:d} exch def\n", thisFunc, i);
    }
    writePSFmt("{{ dup {0:.6g} lt {{ pop {1:.6g} }} {{ dup {2:.6g} gt {{ pop {3:.6g} }} if }} ifelse\n",
	       func3->getDomainMin(0), func3->getDomainMin(0),
	       func3->getDomainMax(0), func3->getDomainMax(0));
    for (i = 0; i < func3->getNumFuncs() - 1; ++i) {
      writePSFmt("dup {0:.6g} lt {{ {1:.6g} sub {2:.6g} mul {3:.6g} add xpdfFunc{4:d}_{5:d} }} {{\n",
		 func3->getBounds()[i+1],
		 func3->getBounds()[i],
		 func3->getScale()[i],
		 func3->getEncode()[2*i],
		 thisFunc, i);
    }
    writePSFmt("{0:.6g} sub {1:.6g} mul {2:.6g} add xpdfFunc{3:d}_{4:d}\n",
	       func3->getBounds()[i],
	       func3->getScale()[i],
	       func3->getEncode()[2*i],
	       thisFunc, i);
    for (i = 0; i < func3->getNumFuncs() - 1; ++i) {
      writePS("} ifelse\n");
    }
    writePS("}\n");
    break;

  case 4:			// PostScript
    func4 = (PostScriptFunction *)func;
    writePS(func4->getCodeString()->getCString());
    writePS("\n");
    break;
  }
}

void PSOutputDev::writePSChar(char c) {
  if (t3String) {
    t3String->append(c);
  } else {
    (*outputFunc)(outputStream, &c, 1);
  }
}

void PSOutputDev::writePSBlock(char *s, int len) {
  if (t3String) {
    t3String->append(s, len);
  } else {
    (*outputFunc)(outputStream, s, len);
  }
}

void PSOutputDev::writePS(const char *s) {
  if (t3String) {
    t3String->append(s);
  } else {
    (*outputFunc)(outputStream, s, (int)strlen(s));
  }
}

void PSOutputDev::writePSFmt(const char *fmt, ...) {
  va_list args;
  GString *buf;

  va_start(args, fmt);
  if (t3String) {
    t3String->appendfv((char *)fmt, args);
  } else {
    buf = GString::formatv((char *)fmt, args);
    (*outputFunc)(outputStream, buf->getCString(), buf->getLength());
    delete buf;
  }
  va_end(args);
}

void PSOutputDev::writePSString(GString *s) {
  Guchar *p;
  int n, line;
  char buf[8];

  writePSChar('(');
  line = 1;
  for (p = (Guchar *)s->getCString(), n = s->getLength(); n; ++p, --n) {
    if (line >= 64) {
      writePSChar('\\');
      writePSChar('\n');
      line = 0;
    }
    if (*p == '(' || *p == ')' || *p == '\\') {
      writePSChar('\\');
      writePSChar((char)*p);
      line += 2;
    } else if (*p < 0x20 || *p >= 0x80) {
      sprintf(buf, "\\%03o", *p);
      writePS(buf);
      line += 4;
    } else {
      writePSChar((char)*p);
      ++line;
    }
  }
  writePSChar(')');
}

void PSOutputDev::writePSName(const char *s) {
  const char *p;
  char c;

  p = s;
  while ((c = *p++)) {
    if (c <= (char)0x20 || c >= (char)0x7f ||
	c == '(' || c == ')' || c == '<' || c == '>' ||
	c == '[' || c == ']' || c == '{' || c == '}' ||
	c == '/' || c == '%') {
      writePSFmt("#{0:02x}", c & 0xff);
    } else {
      writePSChar(c);
    }
  }
}

GString *PSOutputDev::filterPSName(GString *name) {
  GString *name2;
  char buf[8];
  int i;
  char c;

  name2 = new GString();

  // ghostscript chokes on names that begin with out-of-limits
  // numbers, e.g., 1e4foo is handled correctly (as a name), but
  // 1e999foo generates a limitcheck error
  c = name->getChar(0);
  if (c >= '0' && c <= '9') {
    name2->append('f');
  }

  for (i = 0; i < name->getLength(); ++i) {
    c = name->getChar(i);
    if (c <= (char)0x20 || c >= (char)0x7f ||
	c == '(' || c == ')' || c == '<' || c == '>' ||
	c == '[' || c == ']' || c == '{' || c == '}' ||
	c == '/' || c == '%') {
      sprintf(buf, "#%02x", c & 0xff);
      name2->append(buf);
    } else {
      name2->append(c);
    }
  }
  return name2;
}

// Write a DSC-compliant <textline>.
void PSOutputDev::writePSTextLine(GString *s) {
  TextString *ts;
  Unicode *u;
  int i, j;
  int c;

  // - DSC comments must be printable ASCII; control chars and
  //   backslashes have to be escaped (we do cheap Unicode-to-ASCII
  //   conversion by simply ignoring the high byte)
  // - lines are limited to 255 chars (we limit to 200 here to allow
  //   for the keyword, which was emitted by the caller)
  // - lines that start with a left paren are treated as <text>
  //   instead of <textline>, so we escape a leading paren
  ts = new TextString(s);
  u = ts->getUnicode();
  for (i = 0, j = 0; i < ts->getLength() && j < 200; ++i) {
    c = u[i] & 0xff;
    if (c == '\\') {
      writePS("\\\\");
      j += 2;
    } else if (c < 0x20 || c > 0x7e || (j == 0 && c == '(')) {
      writePSFmt("\\{0:03o}", c);
      j += 4;
    } else {
      writePSChar((char)c);
      ++j;
    }
  }
  writePS("\n");
  delete ts;
}
