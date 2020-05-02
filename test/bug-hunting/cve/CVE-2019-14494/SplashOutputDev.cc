//========================================================================
//
// SplashOutputDev.cc
//
// Copyright 2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005 Takashi Iwai <tiwai@suse.de>
// Copyright (C) 2006 Stefan Schweizer <genstef@gentoo.org>
// Copyright (C) 2006-2019 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2006 Krzysztof Kowalczyk <kkowalczyk@gmail.com>
// Copyright (C) 2006 Scott Turner <scotty1024@mac.com>
// Copyright (C) 2007 Koji Otani <sho@bbr.jp>
// Copyright (C) 2009 Petr Gajdos <pgajdos@novell.com>
// Copyright (C) 2009-2016 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2009 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2009, 2014-2016, 2019 William Bader <williambader@hotmail.com>
// Copyright (C) 2010 Patrick Spendrin <ps_ml@gmx.de>
// Copyright (C) 2010 Brian Cameron <brian.cameron@oracle.com>
// Copyright (C) 2010 Paweł Wiejacha <pawel.wiejacha@gmail.com>
// Copyright (C) 2010 Christian Feuersänger <cfeuersaenger@googlemail.com>
// Copyright (C) 2011 Andreas Hartmetz <ahartmetz@gmail.com>
// Copyright (C) 2011 Andrea Canciani <ranma42@gmail.com>
// Copyright (C) 2011, 2012, 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2013 Lu Wang <coolwanglu@gmail.com>
// Copyright (C) 2013 Li Junling <lijunling@sina.com>
// Copyright (C) 2014 Ed Porras <ed@moto-research.com>
// Copyright (C) 2014 Richard PALO <richard@netbsd.org>
// Copyright (C) 2015 Tamas Szekeres <szekerest@gmail.com>
// Copyright (C) 2015 Kenji Uno <ku@digitaldolphins.jp>
// Copyright (C) 2016 Takahiro Hashimoto <kenya888.en@gmail.com>
// Copyright (C) 2017 Even Rouault <even.rouault@spatialys.com>
// Copyright (C) 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
// Copyright (C) 2018 Stefan Brüns <stefan.bruens@rwth-aachen.de>
// Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
// Copyright (C) 2019 Christian Persch <chpe@src.gnome.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#include <string.h>
#include <cmath>
#include "goo/gfile.h"
#include "GlobalParams.h"
#include "Error.h"
#include "Object.h"
#include "Gfx.h"
#include "GfxFont.h"
#include "Page.h"
#include "PDFDoc.h"
#include "Link.h"
#include "FontEncodingTables.h"
#include "fofi/FoFiTrueType.h"
#include "splash/SplashBitmap.h"
#include "splash/SplashGlyphBitmap.h"
#include "splash/SplashPattern.h"
#include "splash/SplashScreen.h"
#include "splash/SplashPath.h"
#include "splash/SplashState.h"
#include "splash/SplashErrorCodes.h"
#include "splash/SplashFontEngine.h"
#include "splash/SplashFont.h"
#include "splash/SplashFontFile.h"
#include "splash/SplashFontFileID.h"
#include "splash/Splash.h"
#include "SplashOutputDev.h"
#include <algorithm>

static const double s_minLineWidth = 0.0;

static inline void convertGfxColor(SplashColorPtr dest,
                                   SplashColorMode colorMode,
                                   GfxColorSpace *colorSpace,
                                   GfxColor *src) {
  SplashColor color;
  GfxGray gray;
  GfxRGB rgb;
#ifdef SPLASH_CMYK
  GfxCMYK cmyk;
  GfxColor deviceN;
#endif

  // make gcc happy
  color[0] = color[1] = color[2] = 0;
#ifdef SPLASH_CMYK
  color[3] = 0;
#endif
  switch (colorMode) {
    case splashModeMono1:
    case splashModeMono8:
      colorSpace->getGray(src, &gray);
      color[0] = colToByte(gray);
    break;
    case splashModeXBGR8:
      color[3] = 255;
      // fallthrough
    case splashModeBGR8:
    case splashModeRGB8:
      colorSpace->getRGB(src, &rgb);
      color[0] = colToByte(rgb.r);
      color[1] = colToByte(rgb.g);
      color[2] = colToByte(rgb.b);
    break;
#ifdef SPLASH_CMYK
    case splashModeCMYK8:
      colorSpace->getCMYK(src, &cmyk);
      color[0] = colToByte(cmyk.c);
      color[1] = colToByte(cmyk.m);
      color[2] = colToByte(cmyk.y);
      color[3] = colToByte(cmyk.k);
    break;
    case splashModeDeviceN8:
      colorSpace->getDeviceN(src, &deviceN);
      for (int i = 0; i < SPOT_NCOMPS + 4; i++)
        color[i] = colToByte(deviceN.c[i]);
    break;
#endif
  }
  splashColorCopy(dest, color);
}

// Copy a color according to the color mode.
// Use convertGfxShortColor() below when the destination is a bitmap
// to avoid overwriting cells.
// Calling this in SplashGouraudPattern::getParameterizedColor() fixes bug 90570.
// Use convertGfxColor() above when the destination is an array of SPOT_NCOMPS+4 bytes,
// to ensure that everything is initialized.

static inline void convertGfxShortColor(SplashColorPtr dest,
                                   SplashColorMode colorMode,
                                   GfxColorSpace *colorSpace,
                                   GfxColor *src) {
  switch (colorMode) {
    case splashModeMono1:
    case splashModeMono8:
    {
      GfxGray gray;
      colorSpace->getGray(src, &gray);
      dest[0] = colToByte(gray);
    }
    break;
    case splashModeXBGR8:
      dest[3] = 255;
      // fallthrough
    case splashModeBGR8:
    case splashModeRGB8:
    {
      GfxRGB rgb;
      colorSpace->getRGB(src, &rgb);
      dest[0] = colToByte(rgb.r);
      dest[1] = colToByte(rgb.g);
      dest[2] = colToByte(rgb.b);
    }
    break;
#ifdef SPLASH_CMYK
    case splashModeCMYK8:
    {
      GfxCMYK cmyk;
      colorSpace->getCMYK(src, &cmyk);
      dest[0] = colToByte(cmyk.c);
      dest[1] = colToByte(cmyk.m);
      dest[2] = colToByte(cmyk.y);
      dest[3] = colToByte(cmyk.k);
    }
    break;
    case splashModeDeviceN8:
    {
      GfxColor deviceN;
      colorSpace->getDeviceN(src, &deviceN);
      for (int i = 0; i < SPOT_NCOMPS + 4; i++)
        dest[i] = colToByte(deviceN.c[i]);
    }
    break;
#endif
  }
}

//------------------------------------------------------------------------
// SplashGouraudPattern
//------------------------------------------------------------------------
SplashGouraudPattern::SplashGouraudPattern(bool bDirectColorTranslationA,
                                           GfxState *stateA, GfxGouraudTriangleShading *shadingA) {
  state = stateA;
  shading = shadingA;
  bDirectColorTranslation = bDirectColorTranslationA;
  gfxMode = shadingA->getColorSpace()->getMode();
}

SplashGouraudPattern::~SplashGouraudPattern() {
}

void SplashGouraudPattern::getParameterizedColor(double colorinterp, SplashColorMode mode, SplashColorPtr dest) {
  GfxColor src;
  GfxColorSpace* srcColorSpace = shading->getColorSpace();
  int colorComps = 3;
#ifdef SPLASH_CMYK
  if (mode == splashModeCMYK8)
    colorComps=4;
  else if (mode == splashModeDeviceN8)
    colorComps=4 + SPOT_NCOMPS;
#endif

  shading->getParameterizedColor(colorinterp, &src);

  if (bDirectColorTranslation) {
    for (int m = 0; m < colorComps; ++m)
      dest[m] = colToByte(src.c[m]);
  } else {
    convertGfxShortColor(dest, mode, srcColorSpace, &src);
  }
}

//------------------------------------------------------------------------
// SplashFunctionPattern
//------------------------------------------------------------------------

SplashFunctionPattern::SplashFunctionPattern(SplashColorMode colorModeA, GfxState *stateA, GfxFunctionShading *shadingA)
{
  Matrix ctm;
  SplashColor defaultColor;
  GfxColor srcColor;
  const double *matrix = shadingA->getMatrix();

  shading = shadingA;
  state = stateA;
  colorMode = colorModeA;

  state->getCTM(&ctm);

  double a1 = ctm.m[0];
  double b1 = ctm.m[1];
  double c1 = ctm.m[2];
  double d1 = ctm.m[3];

  ctm.m[0] = matrix[0] * a1 + matrix[1] * c1;
  ctm.m[1] = matrix[0] * b1 + matrix[1] * d1;
  ctm.m[2] = matrix[2] * a1 + matrix[3] * c1;
  ctm.m[3] = matrix[2] * b1 + matrix[3] * d1;
  ctm.m[4] = matrix[4] * a1 + matrix[5] * c1 + ctm.m[4];
  ctm.m[5] = matrix[4] * b1 + matrix[5] * d1 + ctm.m[5];
  ctm.invertTo(&ictm);

  gfxMode = shadingA->getColorSpace()->getMode();
  shadingA->getColorSpace()->getDefaultColor(&srcColor);
  shadingA->getDomain(&xMin, &yMin, &xMax, &yMax);
  convertGfxColor(defaultColor, colorModeA, shadingA->getColorSpace(), &srcColor);
}

SplashFunctionPattern::~SplashFunctionPattern() {
}

bool SplashFunctionPattern::getColor(int x, int y, SplashColorPtr c) {
  GfxColor gfxColor;
  double xc, yc;

  ictm.transform(x, y, &xc, &yc);
  if (xc < xMin || xc > xMax || yc < yMin || yc > yMax) return false;
  shading->getColor(xc, yc, &gfxColor);
  convertGfxColor(c, colorMode, shading->getColorSpace(), &gfxColor);
  return true;
}

//------------------------------------------------------------------------
// SplashUnivariatePattern
//------------------------------------------------------------------------

SplashUnivariatePattern::SplashUnivariatePattern(SplashColorMode colorModeA, GfxState *stateA, GfxUnivariateShading *shadingA) {
  Matrix ctm;
  double xMin, yMin, xMax, yMax;

  shading = shadingA;
  state = stateA;
  colorMode = colorModeA;

  state->getCTM(&ctm);
  ctm.invertTo(&ictm);

  // get the function domain
  t0 = shading->getDomain0();
  t1 = shading->getDomain1();
  dt = t1 - t0;

  stateA->getUserClipBBox(&xMin, &yMin, &xMax, &yMax);
  shadingA->setupCache(&ctm, xMin, yMin, xMax, yMax);
  gfxMode = shadingA->getColorSpace()->getMode();
}

SplashUnivariatePattern::~SplashUnivariatePattern() {
}

bool SplashUnivariatePattern::getColor(int x, int y, SplashColorPtr c) {
  GfxColor gfxColor;
  double xc, yc, t;

  ictm.transform(x, y, &xc, &yc);
  if (! getParameter (xc, yc, &t))
      return false;

  const int filled = shading->getColor(t, &gfxColor);
  if (unlikely(filled < shading->getColorSpace()->getNComps())) {
    for (int i = filled; i < shading->getColorSpace()->getNComps(); ++i)
      gfxColor.c[i] = 0;
  }
  convertGfxColor(c, colorMode, shading->getColorSpace(), &gfxColor);
  return true;
}

bool SplashUnivariatePattern::testPosition(int x, int y) {
  double xc, yc, t;

  ictm.transform(x, y, &xc, &yc);
  if (! getParameter (xc, yc, &t))
      return false;
  return (t0 < t1) ? (t > t0 && t < t1) : (t > t1 && t < t0);
}


//------------------------------------------------------------------------
// SplashRadialPattern
//------------------------------------------------------------------------
#define RADIAL_EPSILON (1. / 1024 / 1024)

SplashRadialPattern::SplashRadialPattern(SplashColorMode colorModeA, GfxState *stateA, GfxRadialShading *shadingA):
  SplashUnivariatePattern(colorModeA, stateA, shadingA)
{
  SplashColor defaultColor;
  GfxColor srcColor;

  shadingA->getCoords(&x0, &y0, &r0, &dx, &dy, &dr);
  dx -= x0;
  dy -= y0;
  dr -= r0;
  a = dx*dx + dy*dy - dr*dr;
  if (fabs(a) > RADIAL_EPSILON)
    inva = 1.0 / a;
  shadingA->getColorSpace()->getDefaultColor(&srcColor);
  convertGfxColor(defaultColor, colorModeA, shadingA->getColorSpace(), &srcColor);
}

SplashRadialPattern::~SplashRadialPattern() {
}

bool SplashRadialPattern::getParameter(double xs, double ys, double *t) {
  double b, c, s0, s1;

  // We want to solve this system of equations:
  //
  // 1. (x - xc(s))^2 + (y -yc(s))^2 = rc(s)^2
  // 2. xc(s) = x0 + s * (x1 - xo)
  // 3. yc(s) = y0 + s * (y1 - yo)
  // 4. rc(s) = r0 + s * (r1 - ro)
  //
  // To simplify the system a little, we translate
  // our coordinates to have the origin in (x0,y0)

  xs -= x0;
  ys -= y0;

  // Then we have to solve the equation:
  //   A*s^2 - 2*B*s + C = 0
  // where
  //   A = dx^2  + dy^2  - dr^2
  //   B = xs*dx + ys*dy + r0*dr
  //   C = xs^2  + ys^2  - r0^2

  b = xs*dx + ys*dy + r0*dr;
  c = xs*xs + ys*ys - r0*r0;

  if (fabs(a) <= RADIAL_EPSILON) {
    // A is 0, thus the equation simplifies to:
    //   -2*B*s + C = 0
    // If B is 0, we can either have no solution or an indeterminate
    // equation, thus we behave as if we had an invalid solution
    if (fabs(b) <= RADIAL_EPSILON)
      return false;

    s0 = s1 = 0.5 * c / b;
  } else {
    double d;

    d = b*b - a*c;
    if (d < 0)
      return false;

    d = sqrt (d);
    s0 = b + d;
    s1 = b - d;

    // If A < 0, one of the two solutions will have negative radius,
    // thus it will be ignored. Otherwise we know that s1 <= s0
    // (because d >=0 implies b - d <= b + d), so if both are valid it
    // will be the true solution.
    s0 *= inva;
    s1 *= inva;
  }

  if (r0 + s0 * dr >= 0) {
    if (0 <= s0 && s0 <= 1) {
      *t = t0 + dt * s0;
      return true;
    } else if (s0 < 0 && shading->getExtend0()) {
      *t = t0;
      return true;
    } else if (s0 > 1 && shading->getExtend1()) {
      *t = t1;
      return true;
    }
  }

  if (r0 + s1 * dr >= 0) {
    if (0 <= s1 && s1 <= 1) {
      *t = t0 + dt * s1;
      return true;
    } else if (s1 < 0 && shading->getExtend0()) {
      *t = t0;
      return true;
    } else if (s1 > 1 && shading->getExtend1()) {
      *t = t1;
      return true;
    }
  }

  return false;
}

#undef RADIAL_EPSILON

//------------------------------------------------------------------------
// SplashAxialPattern
//------------------------------------------------------------------------

SplashAxialPattern::SplashAxialPattern(SplashColorMode colorModeA, GfxState *stateA, GfxAxialShading *shadingA):
    SplashUnivariatePattern(colorModeA, stateA, shadingA)
{
  SplashColor defaultColor;
  GfxColor srcColor;

  shadingA->getCoords(&x0, &y0, &x1, &y1);
  dx = x1 - x0;
  dy = y1 - y0;
  const double mul_denominator = (dx * dx + dy * dy);
  if (unlikely(mul_denominator == 0)) {
    mul = 0;
  } else {
    mul = 1 / mul_denominator;
  }
  shadingA->getColorSpace()->getDefaultColor(&srcColor);
  convertGfxColor(defaultColor, colorModeA, shadingA->getColorSpace(), &srcColor);
}

SplashAxialPattern::~SplashAxialPattern() {
}

bool SplashAxialPattern::getParameter(double xc, double yc, double *t) {
  double s;

  xc -= x0;
  yc -= y0;

  s = (xc * dx + yc * dy) * mul;
  if (0 <= s && s <= 1) {
    *t = t0 + dt * s;
  } else if (s < 0 && shading->getExtend0()) {
    *t = t0;
  } else if (s > 1 && shading->getExtend1()) {
    *t = t1;
  } else {
    return false;
  }

  return true;
}

//------------------------------------------------------------------------
// Type 3 font cache size parameters
#define type3FontCacheAssoc   8
#define type3FontCacheMaxSets 8
#define type3FontCacheSize    (128*1024)

//------------------------------------------------------------------------
// Divide a 16-bit value (in [0, 255*255]) by 255, returning an 8-bit result.
static inline unsigned char div255(int x) {
  return (unsigned char)((x + (x >> 8) + 0x80) >> 8);
}

//------------------------------------------------------------------------
// Blend functions
//------------------------------------------------------------------------

static void splashOutBlendMultiply(SplashColorPtr src, SplashColorPtr dest,
				   SplashColorPtr blend, SplashColorMode cm) {
  int i;

#ifdef SPLASH_CMYK
  if (cm == splashModeCMYK8 || cm == splashModeDeviceN8) {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      dest[i] = 255 - dest[i];
      src[i] = 255 - src[i];
    }
  }
#endif
  {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      blend[i] = (dest[i] * src[i]) / 255;
    }
  }
#ifdef SPLASH_CMYK
  if (cm == splashModeCMYK8 || cm == splashModeDeviceN8) {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      dest[i] = 255 - dest[i];
      src[i] = 255 - src[i];
      blend[i] = 255 - blend[i];
    }
  }
#endif
}

static void splashOutBlendScreen(SplashColorPtr src, SplashColorPtr dest,
				 SplashColorPtr blend, SplashColorMode cm) {
  int i;

#ifdef SPLASH_CMYK
  if (cm == splashModeCMYK8 || cm == splashModeDeviceN8) {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      dest[i] = 255 - dest[i];
      src[i] = 255 - src[i];
    }
  }
#endif
  {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      blend[i] = dest[i] + src[i] - (dest[i] * src[i]) / 255;
    }
  }
#ifdef SPLASH_CMYK
  if (cm == splashModeCMYK8 || cm == splashModeDeviceN8) {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      dest[i] = 255 - dest[i];
      src[i] = 255 - src[i];
      blend[i] = 255 - blend[i];
    }
  }
#endif
}

static void splashOutBlendOverlay(SplashColorPtr src, SplashColorPtr dest,
				  SplashColorPtr blend, SplashColorMode cm) {
  int i;

#ifdef SPLASH_CMYK
  if (cm == splashModeCMYK8 || cm == splashModeDeviceN8) {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      dest[i] = 255 - dest[i];
      src[i] = 255 - src[i];
    }
  }
#endif
  {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      blend[i] = dest[i] < 0x80
                   ? (src[i] * 2 * dest[i]) / 255
                   : 255 - 2 * ((255 - src[i]) * (255 - dest[i])) / 255;
    }
  }
#ifdef SPLASH_CMYK
  if (cm == splashModeCMYK8 || cm == splashModeDeviceN8) {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      dest[i] = 255 - dest[i];
      src[i] = 255 - src[i];
      blend[i] = 255 - blend[i];
    }
  }
#endif
}

static void splashOutBlendDarken(SplashColorPtr src, SplashColorPtr dest,
				 SplashColorPtr blend, SplashColorMode cm) {
  int i;

#ifdef SPLASH_CMYK
  if (cm == splashModeCMYK8 || cm == splashModeDeviceN8) {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      dest[i] = 255 - dest[i];
      src[i] = 255 - src[i];
    }
  }
#endif
  {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      blend[i] = dest[i] < src[i] ? dest[i] : src[i];
    }
  }
#ifdef SPLASH_CMYK
  if (cm == splashModeCMYK8 || cm == splashModeDeviceN8) {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      dest[i] = 255 - dest[i];
      src[i] = 255 - src[i];
      blend[i] = 255 - blend[i];
    }
  }
#endif
}

static void splashOutBlendLighten(SplashColorPtr src, SplashColorPtr dest,
				  SplashColorPtr blend, SplashColorMode cm) {
  int i;

#ifdef SPLASH_CMYK
  if (cm == splashModeCMYK8 || cm == splashModeDeviceN8) {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      dest[i] = 255 - dest[i];
      src[i] = 255 - src[i];
    }
  }
#endif
  {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      blend[i] = dest[i] > src[i] ? dest[i] : src[i];
    }
  }
#ifdef SPLASH_CMYK
  if (cm == splashModeCMYK8 || cm == splashModeDeviceN8) {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      dest[i] = 255 - dest[i];
      src[i] = 255 - src[i];
      blend[i] = 255 - blend[i];
    }
  }
#endif
}

static void splashOutBlendColorDodge(SplashColorPtr src, SplashColorPtr dest,
				     SplashColorPtr blend,
				     SplashColorMode cm) {
  int i, x;

#ifdef SPLASH_CMYK
  if (cm == splashModeCMYK8 || cm == splashModeDeviceN8) {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      dest[i] = 255 - dest[i];
      src[i] = 255 - src[i];
    }
  }
#endif
  {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      if (src[i] == 255) {
        blend[i] = 255;
      } else {
        x = (dest[i] * 255) / (255 - src[i]);
        blend[i] = x <= 255 ? x : 255;
      }
    }
  }
#ifdef SPLASH_CMYK
  if (cm == splashModeCMYK8 || cm == splashModeDeviceN8) {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      dest[i] = 255 - dest[i];
      src[i] = 255 - src[i];
      blend[i] = 255 - blend[i];
    }
  }
#endif
}

static void splashOutBlendColorBurn(SplashColorPtr src, SplashColorPtr dest,
				    SplashColorPtr blend, SplashColorMode cm) {
  int i, x;

#ifdef SPLASH_CMYK
  if (cm == splashModeCMYK8 || cm == splashModeDeviceN8) {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      dest[i] = 255 - dest[i];
      src[i] = 255 - src[i];
    }
  }
#endif
  {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      if (src[i] == 0) {
        blend[i] = 0;
      } else {
        x = ((255 - dest[i]) * 255) / src[i];
        blend[i] = x <= 255 ? 255 - x : 0;
      }
    }
  }
#ifdef SPLASH_CMYK
  if (cm == splashModeCMYK8 || cm == splashModeDeviceN8) {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      dest[i] = 255 - dest[i];
      src[i] = 255 - src[i];
      blend[i] = 255 - blend[i];
    }
  }
#endif
}

static void splashOutBlendHardLight(SplashColorPtr src, SplashColorPtr dest,
				    SplashColorPtr blend, SplashColorMode cm) {
  int i;

#ifdef SPLASH_CMYK
  if (cm == splashModeCMYK8 || cm == splashModeDeviceN8) {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      dest[i] = 255 - dest[i];
      src[i] = 255 - src[i];
    }
  }
#endif
  {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      blend[i] = src[i] < 0x80
                   ? (dest[i] * 2 * src[i]) / 255
                   : 255 - 2 * ((255 - dest[i]) * (255 - src[i])) / 255;
    }
  }
#ifdef SPLASH_CMYK
  if (cm == splashModeCMYK8 || cm == splashModeDeviceN8) {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      dest[i] = 255 - dest[i];
      src[i] = 255 - src[i];
      blend[i] = 255 - blend[i];
    }
  }
#endif
}

static void splashOutBlendSoftLight(SplashColorPtr src, SplashColorPtr dest,
				    SplashColorPtr blend, SplashColorMode cm) {
  int i, x;

#ifdef SPLASH_CMYK
  if (cm == splashModeCMYK8 || cm == splashModeDeviceN8) {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      dest[i] = 255 - dest[i];
      src[i] = 255 - src[i];
    }
  }
#endif
  {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      if (src[i] < 0x80) {
        blend[i] = dest[i] - (255 - 2 * src[i]) * dest[i] * (255 - dest[i]) / (255 * 255);
      } else {
        if (dest[i] < 0x40) {
          x = (((((16 * dest[i] - 12 * 255) * dest[i]) / 255) + 4 * 255) * dest[i]) / 255;
        } else {
          x = (int)sqrt(255.0 * dest[i]);
        }
        blend[i] = dest[i] + (2 * src[i] - 255) * (x - dest[i]) / 255;
      }
    }
  }
#ifdef SPLASH_CMYK
  if (cm == splashModeCMYK8 || cm == splashModeDeviceN8) {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      dest[i] = 255 - dest[i];
      src[i] = 255 - src[i];
      blend[i] = 255 - blend[i];
    }
  }
#endif
}

static void splashOutBlendDifference(SplashColorPtr src, SplashColorPtr dest,
				     SplashColorPtr blend,
				     SplashColorMode cm) {
  int i;

#ifdef SPLASH_CMYK
  if (cm == splashModeCMYK8 || cm == splashModeDeviceN8) {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      dest[i] = 255 - dest[i];
      src[i] = 255 - src[i];
    }
  }
#endif
  {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      blend[i] = dest[i] < src[i] ? src[i] - dest[i] : dest[i] - src[i];
    }
  }
#ifdef SPLASH_CMYK
  if (cm == splashModeCMYK8 || cm == splashModeDeviceN8) {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      dest[i] = 255 - dest[i];
      src[i] = 255 - src[i];
      blend[i] = 255 - blend[i];
    }
  }
  if (cm == splashModeDeviceN8) {
    for (i = 4; i < splashColorModeNComps[cm]; ++i) {
      if (dest[i] == 0 && src[i] == 0)
        blend[i] = 0;
    }
  }
#endif
}

static void splashOutBlendExclusion(SplashColorPtr src, SplashColorPtr dest,
				    SplashColorPtr blend, SplashColorMode cm) {
  int i;

#ifdef SPLASH_CMYK
  if (cm == splashModeCMYK8 || cm == splashModeDeviceN8) {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      dest[i] = 255 - dest[i];
      src[i] = 255 - src[i];
    }
  }
#endif
  {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      blend[i] = dest[i] + src[i] - (2 * dest[i] * src[i]) / 255;
    }
  }
#ifdef SPLASH_CMYK
  if (cm == splashModeCMYK8 || cm == splashModeDeviceN8) {
    for (i = 0; i < splashColorModeNComps[cm]; ++i) {
      dest[i] = 255 - dest[i];
      src[i] = 255 - src[i];
      blend[i] = 255 - blend[i];
    }
  }
  if (cm == splashModeDeviceN8) {
    for (i = 4; i < splashColorModeNComps[cm]; ++i) {
      if (dest[i] == 0 && src[i] == 0)
        blend[i] = 0;
    }
  }
#endif
}

static int getLum(int r, int g, int b) {
  return (int)(0.3 * r + 0.59 * g + 0.11 * b);
}

static int getSat(int r, int g, int b) {
  int rgbMin, rgbMax;

  rgbMin = rgbMax = r;
  if (g < rgbMin) {
    rgbMin = g;
  } else if (g > rgbMax) {
    rgbMax = g;
  }
  if (b < rgbMin) {
    rgbMin = b;
  } else if (b > rgbMax) {
    rgbMax = b;
  }
  return rgbMax - rgbMin;
}

static void clipColor(int rIn, int gIn, int bIn,
		      unsigned char *rOut, unsigned char *gOut, unsigned char *bOut) {
  int lum, rgbMin, rgbMax;

  lum = getLum(rIn, gIn, bIn);
  rgbMin = rgbMax = rIn;
  if (gIn < rgbMin) {
    rgbMin = gIn;
  } else if (gIn > rgbMax) {
    rgbMax = gIn;
  }
  if (bIn < rgbMin) {
    rgbMin = bIn;
  } else if (bIn > rgbMax) {
    rgbMax = bIn;
  }
  if (rgbMin < 0) {
    *rOut = (unsigned char)(lum + ((rIn - lum) * lum) / (lum - rgbMin));
    *gOut = (unsigned char)(lum + ((gIn - lum) * lum) / (lum - rgbMin));
    *bOut = (unsigned char)(lum + ((bIn - lum) * lum) / (lum - rgbMin));
  } else if (rgbMax > 255) {
    *rOut = (unsigned char)(lum + ((rIn - lum) * (255 - lum)) / (rgbMax - lum));
    *gOut = (unsigned char)(lum + ((gIn - lum) * (255 - lum)) / (rgbMax - lum));
    *bOut = (unsigned char)(lum + ((bIn - lum) * (255 - lum)) / (rgbMax - lum));
  } else {
    *rOut = rIn;
    *gOut = gIn;
    *bOut = bIn;
  }
}

static void setLum(unsigned char rIn, unsigned char gIn, unsigned char bIn, int lum,
		   unsigned char *rOut, unsigned char *gOut, unsigned char *bOut) {
  int d;

  d = lum - getLum(rIn, gIn, bIn);
  clipColor(rIn + d, gIn + d, bIn + d, rOut, gOut, bOut);
}

static void setSat(unsigned char rIn, unsigned char gIn, unsigned char bIn, int sat,
		   unsigned char *rOut, unsigned char *gOut, unsigned char *bOut) {
  int rgbMin, rgbMid, rgbMax;
  unsigned char *minOut, *midOut, *maxOut;

  if (rIn < gIn) {
    rgbMin = rIn;  minOut = rOut;
    rgbMid = gIn;  midOut = gOut;
  } else {
    rgbMin = gIn;  minOut = gOut;
    rgbMid = rIn;  midOut = rOut;
  }
  if (bIn > rgbMid) {
    rgbMax = bIn;  maxOut = bOut;
  } else if (bIn > rgbMin) {
    rgbMax = rgbMid;  maxOut = midOut;
    rgbMid = bIn;     midOut = bOut;
  } else {
    rgbMax = rgbMid;  maxOut = midOut;
    rgbMid = rgbMin;  midOut = minOut;
    rgbMin = bIn;     minOut = bOut;
  }
  if (rgbMax > rgbMin) {
    *midOut = (unsigned char)((rgbMid - rgbMin) * sat) / (rgbMax - rgbMin);
    *maxOut = (unsigned char)sat;
  } else {
    *midOut = *maxOut = 0;
  }
  *minOut = 0;
}

static void splashOutBlendHue(SplashColorPtr src, SplashColorPtr dest,
			      SplashColorPtr blend, SplashColorMode cm) {
  unsigned char r0, g0, b0;
#ifdef SPLASH_CMYK
  unsigned char r1, g1, b1;
  int i;
  SplashColor src2, dest2;
#endif

  switch (cm) {
  case splashModeMono1:
  case splashModeMono8:
    blend[0] = dest[0];
    break;
  case splashModeXBGR8:
    src[3] = 255;
    // fallthrough
  case splashModeRGB8:
  case splashModeBGR8:
    setSat(src[0], src[1], src[2], getSat(dest[0], dest[1], dest[2]),
	   &r0, &g0, &b0);
    setLum(r0, g0, b0, getLum(dest[0], dest[1], dest[2]),
	   &blend[0], &blend[1], &blend[2]);
    break;
#ifdef SPLASH_CMYK
  case splashModeCMYK8:
  case splashModeDeviceN8:
    for (i = 0; i < 4; i++) {
      // convert to additive
      src2[i] = 0xff - src[i];
      dest2[i] = 0xff - dest[i];
    }
    // NB: inputs have already been converted to additive mode
    setSat(src2[0], src2[1], src2[2], getSat(dest2[0], dest2[1], dest2[2]),
	   &r0, &g0, &b0);
    setLum(r0, g0, b0, getLum(dest2[0], dest2[1], dest2[2]),
	   &r1, &g1, &b1);
    blend[0] = r1;
    blend[1] = g1;
    blend[2] = b1;
    blend[3] = dest2[3];
    for (i = 0; i < 4; i++) {
      // convert back to subtractive
      blend[i] = 0xff - blend[i];
    }
    break;
#endif
  }
}

static void splashOutBlendSaturation(SplashColorPtr src, SplashColorPtr dest,
				     SplashColorPtr blend,
				     SplashColorMode cm) {
  unsigned char r0, g0, b0;
#ifdef SPLASH_CMYK
  unsigned char r1, g1, b1;
  int i;
  SplashColor src2, dest2;
#endif

  switch (cm) {
  case splashModeMono1:
  case splashModeMono8:
    blend[0] = dest[0];
    break;
  case splashModeXBGR8:
    src[3] = 255;
    // fallthrough
  case splashModeRGB8:
  case splashModeBGR8:
    setSat(dest[0], dest[1], dest[2], getSat(src[0], src[1], src[2]),
	   &r0, &g0, &b0);
    setLum(r0, g0, b0, getLum(dest[0], dest[1], dest[2]),
	   &blend[0], &blend[1], &blend[2]);
    break;
#ifdef SPLASH_CMYK
  case splashModeCMYK8:
  case splashModeDeviceN8:
    for (i = 0; i < 4; i++) {
      // convert to additive
      src2[i] = 0xff - src[i];
      dest2[i] = 0xff - dest[i];
    }
    setSat(dest2[0], dest2[1], dest2[2], getSat(src2[0], src2[1], src2[2]),
	   &r0, &g0, &b0);
    setLum(r0, g0, b0, getLum(dest2[0], dest2[1], dest2[2]),
	   &r1, &g1, &b1);
    blend[0] = r1;
    blend[1] = g1;
    blend[2] = b1;
    blend[3] = dest2[3];
    for (i = 0; i < 4; i++) {
      // convert back to subtractive
      blend[i] = 0xff - blend[i];
    }
    break;
#endif
  }
}

static void splashOutBlendColor(SplashColorPtr src, SplashColorPtr dest,
				SplashColorPtr blend, SplashColorMode cm) {
#ifdef SPLASH_CMYK
  unsigned char r, g, b;
  int i;
  SplashColor src2, dest2;
#endif

  switch (cm) {
  case splashModeMono1:
  case splashModeMono8:
    blend[0] = dest[0];
    break;
  case splashModeXBGR8:
    src[3] = 255;
    // fallthrough
  case splashModeRGB8:
  case splashModeBGR8:
    setLum(src[0], src[1], src[2], getLum(dest[0], dest[1], dest[2]),
	   &blend[0], &blend[1], &blend[2]);
    break;
#ifdef SPLASH_CMYK
  case splashModeCMYK8:
  case splashModeDeviceN8:
    for (i = 0; i < 4; i++) {
      // convert to additive
      src2[i] = 0xff - src[i];
      dest2[i] = 0xff - dest[i];
    }
    setLum(src2[0], src2[1], src2[2], getLum(dest2[0], dest2[1], dest2[2]),
	   &r, &g, &b);
    blend[0] = r;
    blend[1] = g;
    blend[2] = b;
    blend[3] = dest2[3];
    for (i = 0; i < 4; i++) {
      // convert back to subtractive
      blend[i] = 0xff - blend[i];
    }
    break;
#endif
  }
}

static void splashOutBlendLuminosity(SplashColorPtr src, SplashColorPtr dest,
				     SplashColorPtr blend,
				     SplashColorMode cm) {
#ifdef SPLASH_CMYK
  unsigned char r, g, b;
  int i;
  SplashColor src2, dest2;
#endif

  switch (cm) {
  case splashModeMono1:
  case splashModeMono8:
    blend[0] = dest[0];
    break;
  case splashModeXBGR8:
    src[3] = 255;
    // fallthrough
  case splashModeRGB8:
  case splashModeBGR8:
    setLum(dest[0], dest[1], dest[2], getLum(src[0], src[1], src[2]),
	   &blend[0], &blend[1], &blend[2]);
    break;
#ifdef SPLASH_CMYK
  case splashModeCMYK8:
  case splashModeDeviceN8:
    for (i = 0; i < 4; i++) {
      // convert to additive
      src2[i] = 0xff - src[i];
      dest2[i] = 0xff - dest[i];
    }
    setLum(dest2[0], dest2[1], dest2[2], getLum(src2[0], src2[1], src2[2]),
	   &r, &g, &b);
    blend[0] = r;
    blend[1] = g;
    blend[2] = b;
    blend[3] = src2[3];
    for (i = 0; i < 4; i++) {
      // convert back to subtractive
      blend[i] = 0xff - blend[i];
    }
    break;
#endif
  }
}

// NB: This must match the GfxBlendMode enum defined in GfxState.h.
static const SplashBlendFunc splashOutBlendFuncs[] = {
  nullptr,
  &splashOutBlendMultiply,
  &splashOutBlendScreen,
  &splashOutBlendOverlay,
  &splashOutBlendDarken,
  &splashOutBlendLighten,
  &splashOutBlendColorDodge,
  &splashOutBlendColorBurn,
  &splashOutBlendHardLight,
  &splashOutBlendSoftLight,
  &splashOutBlendDifference,
  &splashOutBlendExclusion,
  &splashOutBlendHue,
  &splashOutBlendSaturation,
  &splashOutBlendColor,
  &splashOutBlendLuminosity
};

//------------------------------------------------------------------------
// SplashOutFontFileID
//------------------------------------------------------------------------

class SplashOutFontFileID: public SplashFontFileID {
public:

  SplashOutFontFileID(const Ref *rA) { r = *rA; }

  ~SplashOutFontFileID() {}

  bool matches(SplashFontFileID *id) override {
    return ((SplashOutFontFileID *)id)->r == r;
  }

private:

  Ref r;
};

//------------------------------------------------------------------------
// T3FontCache
//------------------------------------------------------------------------

struct T3FontCacheTag {
  unsigned short code;
  unsigned short mru;			// valid bit (0x8000) and MRU index
};

class T3FontCache {
public:

  T3FontCache(const Ref *fontID, double m11A, double m12A,
	      double m21A, double m22A,
	      int glyphXA, int glyphYA, int glyphWA, int glyphHA,
	      bool aa, bool validBBoxA);
  ~T3FontCache();
  T3FontCache(const T3FontCache &) = delete;
  T3FontCache& operator=(const T3FontCache &) = delete;
  bool matches(const Ref *idA, double m11A, double m12A,
		double m21A, double m22A)
    { return fontID == *idA &&
	     m11 == m11A && m12 == m12A && m21 == m21A && m22 == m22A; }

  Ref fontID;			// PDF font ID
  double m11, m12, m21, m22;	// transform matrix
  int glyphX, glyphY;		// pixel offset of glyph bitmaps
  int glyphW, glyphH;		// size of glyph bitmaps, in pixels
  bool validBBox;		// false if the bbox was [0 0 0 0]
  int glyphSize;		// size of glyph bitmaps, in bytes
  int cacheSets;		// number of sets in cache
  int cacheAssoc;		// cache associativity (glyphs per set)
  unsigned char *cacheData;		// glyph pixmap cache
  T3FontCacheTag *cacheTags;	// cache tags, i.e., char codes
};

T3FontCache::T3FontCache(const Ref *fontIDA, double m11A, double m12A,
			 double m21A, double m22A,
			 int glyphXA, int glyphYA, int glyphWA, int glyphHA,
			 bool validBBoxA, bool aa) {

  fontID = *fontIDA;
  m11 = m11A;
  m12 = m12A;
  m21 = m21A;
  m22 = m22A;
  glyphX = glyphXA;
  glyphY = glyphYA;
  glyphW = glyphWA;
  glyphH = glyphHA;
  validBBox = validBBoxA;
  // sanity check for excessively large glyphs (which most likely
  // indicate an incorrect BBox)
  if (glyphW > INT_MAX / glyphH || glyphW <= 0 || glyphH <= 0 || glyphW * glyphH > 100000) {
    glyphW = glyphH = 100;
    validBBox = false;
  }
  if (aa) {
    glyphSize = glyphW * glyphH;
  } else {
    glyphSize = ((glyphW + 7) >> 3) * glyphH;
  }
  cacheAssoc = type3FontCacheAssoc;
  for (cacheSets = type3FontCacheMaxSets;
       cacheSets > 1 &&
	 cacheSets * cacheAssoc * glyphSize > type3FontCacheSize;
       cacheSets >>= 1) ;
  if (glyphSize < 10485760 / cacheAssoc / cacheSets) {
    cacheData = (unsigned char *)gmallocn_checkoverflow(cacheSets * cacheAssoc, glyphSize);
  } else {
    error(errSyntaxWarning, -1, "Not creating cacheData for T3FontCache, it asked for too much memory.\n"
              "       This could teoretically result in wrong rendering,\n"
              "       but most probably the document is bogus.\n"
              "       Please report a bug if you think the rendering may be wrong because of this.");
    cacheData = nullptr;
  }
  if (cacheData != nullptr)
  {
    cacheTags = (T3FontCacheTag *)gmallocn(cacheSets * cacheAssoc,
					 sizeof(T3FontCacheTag));
    for (int i = 0; i < cacheSets * cacheAssoc; ++i) {
      cacheTags[i].mru = i & (cacheAssoc - 1);
    }
  }
  else
  {
    cacheTags = nullptr;
  }
}

T3FontCache::~T3FontCache() {
  gfree(cacheData);
  gfree(cacheTags);
}

struct T3GlyphStack {
  unsigned short code;			// character code

  bool haveDx;			// set after seeing a d0/d1 operator
  bool doNotCache;		// set if we see a gsave/grestore before
				//   the d0/d1

  //----- cache info
  T3FontCache *cache;		// font cache for the current font
  T3FontCacheTag *cacheTag;	// pointer to cache tag for the glyph
  unsigned char *cacheData;		// pointer to cache data for the glyph

  //----- saved state
  SplashBitmap *origBitmap;
  Splash *origSplash;
  double origCTM4, origCTM5;

  T3GlyphStack *next;		// next object on stack
};

//------------------------------------------------------------------------
// SplashTransparencyGroup
//------------------------------------------------------------------------

struct SplashTransparencyGroup {
  int tx, ty;			// translation coordinates
  SplashBitmap *tBitmap;	// bitmap for transparency group
  SplashBitmap *softmask;	// bitmap for softmasks
  GfxColorSpace *blendingColorSpace;
  bool isolated;

  //----- for knockout
  SplashBitmap *shape;
  bool knockout;
  SplashCoord knockoutOpacity;
  bool fontAA;

  //----- saved state
  SplashBitmap *origBitmap;
  Splash *origSplash;

  SplashTransparencyGroup *next;
};

//------------------------------------------------------------------------
// SplashOutputDev
//------------------------------------------------------------------------

SplashOutputDev::SplashOutputDev(SplashColorMode colorModeA,
				 int bitmapRowPadA,
				 bool reverseVideoA,
				 SplashColorPtr paperColorA,
				 bool bitmapTopDownA,
				 SplashThinLineMode thinLineMode,
				 bool overprintPreviewA) {
  colorMode = colorModeA;
  bitmapRowPad = bitmapRowPadA;
  bitmapTopDown = bitmapTopDownA;
  bitmapUpsideDown = false;
  fontAntialias = true;
  vectorAntialias = true;
  overprintPreview = overprintPreviewA;
  enableFreeTypeHinting = false;
  enableSlightHinting = false;
  setupScreenParams(72.0, 72.0);
  reverseVideo = reverseVideoA;
  if (paperColorA != nullptr) {
    splashColorCopy(paperColor, paperColorA);
  } else {
    splashClearColor(paperColor);
  }
  skipHorizText = false;
  skipRotatedText = false;
  keepAlphaChannel = paperColorA == nullptr;

  doc = nullptr;

  bitmap = new SplashBitmap(1, 1, bitmapRowPad, colorMode,
			    colorMode != splashModeMono1, bitmapTopDown);
  splash = new Splash(bitmap, vectorAntialias, &screenParams);
  splash->setMinLineWidth(s_minLineWidth);
  splash->setThinLineMode(thinLineMode);
  splash->clear(paperColor, 0);

  fontEngine = nullptr;

  nT3Fonts = 0;
  t3GlyphStack = nullptr;

  font = nullptr;
  needFontUpdate = false;
  textClipPath = nullptr;
  transpGroupStack = nullptr;
  nestCount = 0;
  xref = nullptr;
}

void SplashOutputDev::setupScreenParams(double hDPI, double vDPI) {
  screenParams.size = -1;
  screenParams.dotRadius = -1;
  screenParams.gamma = (SplashCoord)1.0;
  screenParams.blackThreshold = (SplashCoord)0.0;
  screenParams.whiteThreshold = (SplashCoord)1.0;

  // use clustered dithering for resolution >= 300 dpi
  // (compare to 299.9 to avoid floating point issues)
  if (hDPI > 299.9 && vDPI > 299.9) {
    screenParams.type = splashScreenStochasticClustered;
    if (screenParams.size < 0) {
      screenParams.size = 64;
    }
    if (screenParams.dotRadius < 0) {
      screenParams.dotRadius = 2;
    }
  } else {
    screenParams.type = splashScreenDispersed;
    if (screenParams.size < 0) {
      screenParams.size = 4;
    }
  }
}

SplashOutputDev::~SplashOutputDev() {
  int i;

  for (i = 0; i < nT3Fonts; ++i) {
    delete t3FontCache[i];
  }
  if (fontEngine) {
    delete fontEngine;
  }
  if (splash) {
    delete splash;
  }
  if (bitmap) {
    delete bitmap;
  }
  delete textClipPath;
}

void SplashOutputDev::startDoc(PDFDoc *docA) {
  int i;

  doc = docA;
  if (fontEngine) {
    delete fontEngine;
  }
  fontEngine = new SplashFontEngine(
				    globalParams->getEnableFreeType(),
				    enableFreeTypeHinting,
				    enableSlightHinting,
				      getFontAntialias() &&
				      colorMode != splashModeMono1);
  for (i = 0; i < nT3Fonts; ++i) {
    delete t3FontCache[i];
  }
  nT3Fonts = 0;
}

void SplashOutputDev::startPage(int pageNum, GfxState *state, XRef *xrefA) {
  int w, h;
  SplashCoord mat[6];
  SplashColor color;

  xref = xrefA;
  if (state) {
    setupScreenParams(state->getHDPI(), state->getVDPI());
    w = (int)(state->getPageWidth() + 0.5);
    if (w <= 0) {
      w = 1;
    }
    h = (int)(state->getPageHeight() + 0.5);
    if (h <= 0) {
      h = 1;
    }
  } else {
    w = h = 1;
  }
  SplashThinLineMode thinLineMode = splashThinLineDefault;
  if (splash) {
    thinLineMode = splash->getThinLineMode();
    delete splash;
    splash = nullptr;
  }
  if (!bitmap || w != bitmap->getWidth() || h != bitmap->getHeight()) {
    if (bitmap) {
      delete bitmap;
      bitmap = nullptr;
    }
    bitmap = new SplashBitmap(w, h, bitmapRowPad, colorMode,
			      colorMode != splashModeMono1, bitmapTopDown);
    if (!bitmap->getDataPtr()) {
      delete bitmap;
      w = h = 1;
      bitmap = new SplashBitmap(w, h, bitmapRowPad, colorMode,
                              colorMode != splashModeMono1, bitmapTopDown);
    }
  }
  splash = new Splash(bitmap, vectorAntialias, &screenParams);
  splash->setThinLineMode(thinLineMode);
  splash->setMinLineWidth(s_minLineWidth);
  if (state) {
    const double *ctm = state->getCTM();
    mat[0] = (SplashCoord)ctm[0];
    mat[1] = (SplashCoord)ctm[1];
    mat[2] = (SplashCoord)ctm[2];
    mat[3] = (SplashCoord)ctm[3];
    mat[4] = (SplashCoord)ctm[4];
    mat[5] = (SplashCoord)ctm[5];
    splash->setMatrix(mat);
  }
  switch (colorMode) {
  case splashModeMono1:
  case splashModeMono8:
    color[0] = 0;
    break;
  case splashModeXBGR8:
    color[3] = 255;
    // fallthrough
  case splashModeRGB8:
  case splashModeBGR8:
    color[0] = color[1] = color[2] = 0;
    break;
#ifdef SPLASH_CMYK
  case splashModeCMYK8:
    color[0] = color[1] = color[2] = color[3] = 0;
    break;
  case splashModeDeviceN8:
    for (int i = 0; i < 4 + SPOT_NCOMPS; i++)
      color[i] = 0;
    break;
#endif
  }
  splash->setStrokePattern(new SplashSolidColor(color));
  splash->setFillPattern(new SplashSolidColor(color));
  splash->setLineCap(splashLineCapButt);
  splash->setLineJoin(splashLineJoinMiter);
  splash->setLineDash(nullptr, 0, 0);
  splash->setMiterLimit(10);
  splash->setFlatness(1);
  // the SA parameter supposedly defaults to false, but Acrobat
  // apparently hardwires it to true
  splash->setStrokeAdjust(true);
  splash->clear(paperColor, 0);
}

void SplashOutputDev::endPage() {
  if (colorMode != splashModeMono1 && !keepAlphaChannel) {
    splash->compositeBackground(paperColor);
  }
}

void SplashOutputDev::saveState(GfxState *state) {
  splash->saveState();
  if (t3GlyphStack && !t3GlyphStack->haveDx) {
    t3GlyphStack->doNotCache = true;
    error(errSyntaxWarning, -1,
	  "Save (q) operator before d0/d1 in Type 3 glyph");
  }
}

void SplashOutputDev::restoreState(GfxState *state) {
  splash->restoreState();
  needFontUpdate = true;
  if (t3GlyphStack && !t3GlyphStack->haveDx) {
    t3GlyphStack->doNotCache = true;
    error(errSyntaxWarning, -1,
	  "Restore (Q) operator before d0/d1 in Type 3 glyph");
  }
}

void SplashOutputDev::updateAll(GfxState *state) {
  updateLineDash(state);
  updateLineJoin(state);
  updateLineCap(state);
  updateLineWidth(state);
  updateFlatness(state);
  updateMiterLimit(state);
  updateStrokeAdjust(state);
  updateFillColorSpace(state);
  updateFillColor(state);
  updateStrokeColorSpace(state);
  updateStrokeColor(state);
  needFontUpdate = true;
}

void SplashOutputDev::updateCTM(GfxState *state, double m11, double m12,
				double m21, double m22,
				double m31, double m32) {
  SplashCoord mat[6];

  const double *ctm = state->getCTM();
  mat[0] = (SplashCoord)ctm[0];
  mat[1] = (SplashCoord)ctm[1];
  mat[2] = (SplashCoord)ctm[2];
  mat[3] = (SplashCoord)ctm[3];
  mat[4] = (SplashCoord)ctm[4];
  mat[5] = (SplashCoord)ctm[5];
  splash->setMatrix(mat);
}

void SplashOutputDev::updateLineDash(GfxState *state) {
  double *dashPattern;
  int dashLength;
  double dashStart;
  SplashCoord dash[20];
  int i;

  state->getLineDash(&dashPattern, &dashLength, &dashStart);
  if (dashLength > 20) {
    dashLength = 20;
  }
  for (i = 0; i < dashLength; ++i) {
    dash[i] = (SplashCoord)dashPattern[i];
    if (dash[i] < 0) {
      dash[i] = 0;
    }
  }
  splash->setLineDash(dash, dashLength, (SplashCoord)dashStart);
}

void SplashOutputDev::updateFlatness(GfxState *state) {
#if 0 // Acrobat ignores the flatness setting, and always renders curves
      // with a fairly small flatness value
   splash->setFlatness(state->getFlatness());
#endif
}

void SplashOutputDev::updateLineJoin(GfxState *state) {
  splash->setLineJoin(state->getLineJoin());
}

void SplashOutputDev::updateLineCap(GfxState *state) {
  splash->setLineCap(state->getLineCap());
}

void SplashOutputDev::updateMiterLimit(GfxState *state) {
  splash->setMiterLimit(state->getMiterLimit());
}

void SplashOutputDev::updateLineWidth(GfxState *state) {
  splash->setLineWidth(state->getLineWidth());
}

void SplashOutputDev::updateStrokeAdjust(GfxState * /*state*/) {
#if 0 // the SA parameter supposedly defaults to false, but Acrobat
      // apparently hardwires it to true
  splash->setStrokeAdjust(state->getStrokeAdjust());
#endif
}

void SplashOutputDev::updateFillColorSpace(GfxState *state) {
#ifdef SPLASH_CMYK
  if (colorMode == splashModeDeviceN8)
    state->getFillColorSpace()->createMapping(bitmap->getSeparationList(), SPOT_NCOMPS);
#endif
}

void SplashOutputDev::updateStrokeColorSpace(GfxState *state) {
#ifdef SPLASH_CMYK
  if (colorMode == splashModeDeviceN8)
    state->getStrokeColorSpace()->createMapping(bitmap->getSeparationList(), SPOT_NCOMPS);
#endif
}

void SplashOutputDev::updateFillColor(GfxState *state) {
  GfxGray gray;
  GfxRGB rgb;
#ifdef SPLASH_CMYK
  GfxCMYK cmyk;
  GfxColor deviceN;
#endif

  switch (colorMode) {
  case splashModeMono1:
  case splashModeMono8:
    state->getFillGray(&gray);
    splash->setFillPattern(getColor(gray));
    break;
  case splashModeXBGR8:
  case splashModeRGB8:
  case splashModeBGR8:
    state->getFillRGB(&rgb);
    splash->setFillPattern(getColor(&rgb));
    break;
#ifdef SPLASH_CMYK
  case splashModeCMYK8:
    state->getFillCMYK(&cmyk);
    splash->setFillPattern(getColor(&cmyk));
    break;
  case splashModeDeviceN8:
    state->getFillDeviceN(&deviceN);
    splash->setFillPattern(getColor(&deviceN));
    break;
#endif
  }
}

void SplashOutputDev::updateStrokeColor(GfxState *state) {
  GfxGray gray;
  GfxRGB rgb;
#ifdef SPLASH_CMYK
  GfxCMYK cmyk;
  GfxColor deviceN;
#endif

  switch (colorMode) {
  case splashModeMono1:
  case splashModeMono8:
    state->getStrokeGray(&gray);
    splash->setStrokePattern(getColor(gray));
    break;
  case splashModeXBGR8:
  case splashModeRGB8:
  case splashModeBGR8:
    state->getStrokeRGB(&rgb);
    splash->setStrokePattern(getColor(&rgb));
    break;
#ifdef SPLASH_CMYK
  case splashModeCMYK8:
    state->getStrokeCMYK(&cmyk);
    splash->setStrokePattern(getColor(&cmyk));
    break;
  case splashModeDeviceN8:
    state->getStrokeDeviceN(&deviceN);
    splash->setStrokePattern(getColor(&deviceN));
    break;
#endif
  }
}

SplashPattern *SplashOutputDev::getColor(GfxGray gray) {
  SplashColor color;

  if (reverseVideo) {
    gray = gfxColorComp1 - gray;
  }
  color[0] = colToByte(gray);
  return new SplashSolidColor(color);
}

SplashPattern *SplashOutputDev::getColor(GfxRGB *rgb) {
  GfxColorComp r, g, b;
  SplashColor color;

  if (reverseVideo) {
    r = gfxColorComp1 - rgb->r;
    g = gfxColorComp1 - rgb->g;
    b = gfxColorComp1 - rgb->b;
  } else {
    r = rgb->r;
    g = rgb->g;
    b = rgb->b;
  }
  color[0] = colToByte(r);
  color[1] = colToByte(g);
  color[2] = colToByte(b);
  if (colorMode == splashModeXBGR8) color[3] = 255;
  return new SplashSolidColor(color);
}

#ifdef SPLASH_CMYK
SplashPattern *SplashOutputDev::getColor(GfxCMYK *cmyk) {
  SplashColor color;

  color[0] = colToByte(cmyk->c);
  color[1] = colToByte(cmyk->m);
  color[2] = colToByte(cmyk->y);
  color[3] = colToByte(cmyk->k);
  return new SplashSolidColor(color);
}

SplashPattern *SplashOutputDev::getColor(GfxColor *deviceN) {
  SplashColor color;

  for (int i = 0; i < 4 + SPOT_NCOMPS; i++)
    color[i] = colToByte(deviceN->c[i]);
  return new SplashSolidColor(color);
}
#endif

void SplashOutputDev::getMatteColor(SplashColorMode colorMode, GfxImageColorMap *colorMap, const GfxColor *matteColorIn, SplashColor matteColor) {
  GfxGray gray;
  GfxRGB rgb;
#ifdef SPLASH_CMYK
  GfxCMYK cmyk;
  GfxColor deviceN;
#endif

  switch (colorMode) {
    case splashModeMono1:
    case splashModeMono8:
      colorMap->getColorSpace()->getGray(matteColorIn, &gray);
      matteColor[0] = colToByte(gray);
      break;
    case splashModeRGB8:
    case splashModeBGR8:
      colorMap->getColorSpace()->getRGB(matteColorIn, &rgb);
      matteColor[0] = colToByte(rgb.r);
      matteColor[1] = colToByte(rgb.g);
      matteColor[2] = colToByte(rgb.b);
      break;
    case splashModeXBGR8:
      colorMap->getColorSpace()->getRGB(matteColorIn, &rgb);
      matteColor[0] = colToByte(rgb.r);
      matteColor[1] = colToByte(rgb.g);
      matteColor[2] = colToByte(rgb.b);
      matteColor[3] = 255;
      break;
#ifdef SPLASH_CMYK
    case splashModeCMYK8:
      colorMap->getColorSpace()->getCMYK(matteColorIn, &cmyk);
      matteColor[0] = colToByte(cmyk.c);
      matteColor[1] = colToByte(cmyk.m);
      matteColor[2] = colToByte(cmyk.y);
      matteColor[3] = colToByte(cmyk.k);
      break;
    case splashModeDeviceN8:
      colorMap->getColorSpace()->getDeviceN(matteColorIn, &deviceN);
      for (int cp = 0; cp < SPOT_NCOMPS+4; cp++)
        matteColor[cp] = colToByte(deviceN.c[cp]);
      break;
#endif
  }
}

void SplashOutputDev::setOverprintMask(GfxColorSpace *colorSpace,
				       bool overprintFlag,
				       int overprintMode,
				       const GfxColor *singleColor,
				       bool grayIndexed) {
#ifdef SPLASH_CMYK
  unsigned int mask;
  GfxCMYK cmyk;
  bool additive = false;
  int i;

  if (colorSpace->getMode() == csIndexed) {
    setOverprintMask(((GfxIndexedColorSpace *)colorSpace)->getBase(),
		     overprintFlag,
		     overprintMode,
		     singleColor,
		     grayIndexed);
		return;
	}
  if (overprintFlag && overprintPreview) {
    mask = colorSpace->getOverprintMask();
    if (singleColor && overprintMode &&
	colorSpace->getMode() == csDeviceCMYK) {
      colorSpace->getCMYK(singleColor, &cmyk);
      if (cmyk.c == 0) {
	mask &= ~1;
      }
      if (cmyk.m == 0) {
	mask &= ~2;
      }
      if (cmyk.y == 0) {
	mask &= ~4;
      }
      if (cmyk.k == 0) {
	mask &= ~8;
      }
    }
    if (grayIndexed) {
      mask &= ~7;
    } else if (colorSpace->getMode() == csSeparation) {
      GfxSeparationColorSpace *deviceSep = (GfxSeparationColorSpace *)colorSpace;
      additive = deviceSep->getName()->cmp("All") != 0 && mask == 0x0f && !deviceSep->isNonMarking();
    } else if (colorSpace->getMode() == csDeviceN) {
      GfxDeviceNColorSpace *deviceNCS = (GfxDeviceNColorSpace *)colorSpace;
      additive = mask == 0x0f && !deviceNCS->isNonMarking();
      for (i = 0; i < deviceNCS->getNComps() && additive; i++) {
        if (deviceNCS->getColorantName(i)->cmp("Cyan") == 0) {
          additive = false;
        } else if (deviceNCS->getColorantName(i)->cmp("Magenta") == 0) {
          additive = false;
        } else if (deviceNCS->getColorantName(i)->cmp("Yellow") == 0) {
          additive = false;
        } else if (deviceNCS->getColorantName(i)->cmp("Black") == 0) {
          additive = false;
        }
      }
    }
  } else {
    mask = 0xffffffff;
  }
  splash->setOverprintMask(mask, additive);
#endif
}

void SplashOutputDev::updateBlendMode(GfxState *state) {
  splash->setBlendFunc(splashOutBlendFuncs[state->getBlendMode()]);
}

void SplashOutputDev::updateFillOpacity(GfxState *state) {
  splash->setFillAlpha((SplashCoord)state->getFillOpacity());
  if (transpGroupStack != nullptr && (SplashCoord)state->getFillOpacity() < transpGroupStack->knockoutOpacity) {
    transpGroupStack->knockoutOpacity = (SplashCoord)state->getFillOpacity();
  }
}

void SplashOutputDev::updateStrokeOpacity(GfxState *state) {
  splash->setStrokeAlpha((SplashCoord)state->getStrokeOpacity());
  if (transpGroupStack != nullptr && (SplashCoord)state->getStrokeOpacity() < transpGroupStack->knockoutOpacity) {
    transpGroupStack->knockoutOpacity = (SplashCoord)state->getStrokeOpacity();
  }
}

void SplashOutputDev::updatePatternOpacity(GfxState *state) {
  splash->setPatternAlpha((SplashCoord)state->getStrokeOpacity(), (SplashCoord)state->getFillOpacity());
}

void SplashOutputDev::clearPatternOpacity(GfxState *state) {
  splash->clearPatternAlpha();
}

void SplashOutputDev::updateFillOverprint(GfxState *state) {
  splash->setFillOverprint(state->getFillOverprint());
}

void SplashOutputDev::updateStrokeOverprint(GfxState *state) {
  splash->setStrokeOverprint(state->getStrokeOverprint());
}

void SplashOutputDev::updateOverprintMode(GfxState *state) {
  splash->setOverprintMode(state->getOverprintMode());
}

void SplashOutputDev::updateTransfer(GfxState *state) {
  Function **transfer;
  unsigned char red[256], green[256], blue[256], gray[256];
  double x, y;
  int i;

  transfer = state->getTransfer();
  if (transfer[0] &&
      transfer[0]->getInputSize() == 1 &&
      transfer[0]->getOutputSize() == 1) {
    if (transfer[1] &&
	transfer[1]->getInputSize() == 1 &&
	transfer[1]->getOutputSize() == 1 &&
	transfer[2] &&
	transfer[2]->getInputSize() == 1 &&
	transfer[2]->getOutputSize() == 1 &&
	transfer[3] &&
	transfer[3]->getInputSize() == 1 &&
	transfer[3]->getOutputSize() == 1) {
      for (i = 0; i < 256; ++i) {
	x = i / 255.0;
	transfer[0]->transform(&x, &y);
	red[i] = (unsigned char)(y * 255.0 + 0.5);
	transfer[1]->transform(&x, &y);
	green[i] = (unsigned char)(y * 255.0 + 0.5);
	transfer[2]->transform(&x, &y);
	blue[i] = (unsigned char)(y * 255.0 + 0.5);
	transfer[3]->transform(&x, &y);
	gray[i] = (unsigned char)(y * 255.0 + 0.5);
      }
    } else {
      for (i = 0; i < 256; ++i) {
	x = i / 255.0;
	transfer[0]->transform(&x, &y);
	red[i] = green[i] = blue[i] = gray[i] = (unsigned char)(y * 255.0 + 0.5);
      }
    }
  } else {
    for (i = 0; i < 256; ++i) {
      red[i] = green[i] = blue[i] = gray[i] = (unsigned char)i;
    }
  }
  splash->setTransfer(red, green, blue, gray);
}

void SplashOutputDev::updateFont(GfxState * /*state*/) {
  needFontUpdate = true;
}

void SplashOutputDev::doUpdateFont(GfxState *state) {
  GfxFont *gfxFont;
  GfxFontLoc *fontLoc;
  GfxFontType fontType;
  SplashOutFontFileID *id = nullptr;
  SplashFontFile *fontFile;
  SplashFontSrc *fontsrc = nullptr;
  FoFiTrueType *ff;
  GooString *fileName;
  char *tmpBuf;
  int tmpBufLen;
  int *codeToGID;
  const double *textMat;
  double m11, m12, m21, m22, fontSize;
  int faceIndex = 0;
  SplashCoord mat[4];
  int n, i;
  bool recreateFont = false;
  bool doAdjustFontMatrix = false;

  needFontUpdate = false;
  font = nullptr;
  fileName = nullptr;
  tmpBuf = nullptr;
  fontLoc = nullptr;

  if (!(gfxFont = state->getFont())) {
    goto err1;
  }
  fontType = gfxFont->getType();
  if (fontType == fontType3) {
    goto err1;
  }

  // sanity-check the font size - skip anything larger than 10 inches
  // (this avoids problems allocating memory for the font cache)
  if (state->getTransformedFontSize()
        > 10 * (state->getHDPI() + state->getVDPI())) {
    goto err1;
  }

  // check the font file cache
reload:
  delete id;
  delete fontLoc;
  fontLoc = nullptr;
  if (fontsrc && !fontsrc->isFile) {
      fontsrc->unref();
      fontsrc = nullptr;
  }

  id = new SplashOutFontFileID(gfxFont->getID());
  if ((fontFile = fontEngine->getFontFile(id))) {
    delete id;

  } else {

    if (!(fontLoc = gfxFont->locateFont((xref) ? xref : doc->getXRef(), nullptr))) {
      error(errSyntaxError, -1, "Couldn't find a font for '{0:s}'",
	    gfxFont->getName() ? gfxFont->getName()->c_str()
	                       : "(unnamed)");
      goto err2;
    }

    // embedded font
    if (fontLoc->locType == gfxFontLocEmbedded) {
      // if there is an embedded font, read it to memory
      tmpBuf = gfxFont->readEmbFontFile((xref) ? xref : doc->getXRef(), &tmpBufLen);
      if (! tmpBuf)
	goto err2;

    // external font
    } else { // gfxFontLocExternal
      fileName = fontLoc->path;
      fontType = fontLoc->fontType;
      doAdjustFontMatrix = true;
    }

    fontsrc = new SplashFontSrc;
    if (fileName)
      fontsrc->setFile(fileName, false);
    else
      fontsrc->setBuf(tmpBuf, tmpBufLen, true);

    // load the font file
    switch (fontType) {
    case fontType1:
      if (!(fontFile = fontEngine->loadType1Font(
			   id,
			   fontsrc,
			   (const char **)((Gfx8BitFont *)gfxFont)->getEncoding()))) {
	error(errSyntaxError, -1, "Couldn't create a font for '{0:s}'",
	      gfxFont->getName() ? gfxFont->getName()->c_str()
	                         : "(unnamed)");
	if (gfxFont->invalidateEmbeddedFont()) goto reload;
	goto err2;
      }
      break;
    case fontType1C:
      if (!(fontFile = fontEngine->loadType1CFont(
			   id,
			   fontsrc,
			   (const char **)((Gfx8BitFont *)gfxFont)->getEncoding()))) {
	error(errSyntaxError, -1, "Couldn't create a font for '{0:s}'",
	      gfxFont->getName() ? gfxFont->getName()->c_str()
	                         : "(unnamed)");
	if (gfxFont->invalidateEmbeddedFont()) goto reload;
	goto err2;
      }
      break;
    case fontType1COT:
      if (!(fontFile = fontEngine->loadOpenTypeT1CFont(
			   id,
			   fontsrc,
			   (const char **)((Gfx8BitFont *)gfxFont)->getEncoding()))) {
	error(errSyntaxError, -1, "Couldn't create a font for '{0:s}'",
	      gfxFont->getName() ? gfxFont->getName()->c_str()
	                         : "(unnamed)");
	if (gfxFont->invalidateEmbeddedFont()) goto reload;
	goto err2;
      }
      break;
    case fontTrueType:
    case fontTrueTypeOT:
	if (fileName)
	 ff = FoFiTrueType::load(fileName->c_str());
	else
	ff = FoFiTrueType::make(tmpBuf, tmpBufLen);
      if (ff) {
	codeToGID = ((Gfx8BitFont *)gfxFont)->getCodeToGIDMap(ff);
	n = 256;
	delete ff;
	// if we're substituting for a non-TrueType font, we need to mark
	// all notdef codes as "do not draw" (rather than drawing TrueType
	// notdef glyphs)
	if (gfxFont->getType() != fontTrueType &&
	    gfxFont->getType() != fontTrueTypeOT) {
	  for (i = 0; i < 256; ++i) {
	    if (codeToGID[i] == 0) {
	      codeToGID[i] = -1;
	    }
	  }
	}
      } else {
	codeToGID = nullptr;
	n = 0;
      }
      if (!(fontFile = fontEngine->loadTrueTypeFont(
			   id,
			   fontsrc,
			   codeToGID, n))) {
	error(errSyntaxError, -1, "Couldn't create a font for '{0:s}'",
	      gfxFont->getName() ? gfxFont->getName()->c_str()
	                         : "(unnamed)");
	if (gfxFont->invalidateEmbeddedFont()) goto reload;
	goto err2;
      }
      break;
    case fontCIDType0:
    case fontCIDType0C:
      if (!(fontFile = fontEngine->loadCIDFont(
			   id,
			   fontsrc))) {
	error(errSyntaxError, -1, "Couldn't create a font for '{0:s}'",
	      gfxFont->getName() ? gfxFont->getName()->c_str()
	                         : "(unnamed)");
	if (gfxFont->invalidateEmbeddedFont()) goto reload;
	goto err2;
      }
      break;
    case fontCIDType0COT:
      if (((GfxCIDFont *)gfxFont)->getCIDToGID()) {
	n = ((GfxCIDFont *)gfxFont)->getCIDToGIDLen();
	codeToGID = (int *)gmallocn(n, sizeof(int));
	memcpy(codeToGID, ((GfxCIDFont *)gfxFont)->getCIDToGID(),
	       n * sizeof(int));
      } else {
	codeToGID = nullptr;
	n = 0;
      }
      if (!(fontFile = fontEngine->loadOpenTypeCFFFont(
			   id,
			   fontsrc,
                           codeToGID, n))) {
	error(errSyntaxError, -1, "Couldn't create a font for '{0:s}'",
	      gfxFont->getName() ? gfxFont->getName()->c_str()
	                         : "(unnamed)");
	if (gfxFont->invalidateEmbeddedFont()) goto reload;
	goto err2;
      }
      break;
    case fontCIDType2:
    case fontCIDType2OT:
      codeToGID = nullptr;
      n = 0;
      if (((GfxCIDFont *)gfxFont)->getCIDToGID()) {
	n = ((GfxCIDFont *)gfxFont)->getCIDToGIDLen();
	if (n) {
	  codeToGID = (int *)gmallocn(n, sizeof(int));
	  memcpy(codeToGID, ((GfxCIDFont *)gfxFont)->getCIDToGID(),
		  n * sizeof(int));
	}
      } else {
	if (fileName)
	  ff = FoFiTrueType::load(fileName->c_str());
	else
	  ff = FoFiTrueType::make(tmpBuf, tmpBufLen);
	if (! ff)
	{
	error(errSyntaxError, -1, "Couldn't create a font for '{0:s}'",
	      gfxFont->getName() ? gfxFont->getName()->c_str()
	                         : "(unnamed)");
	  goto err2;
	}
	codeToGID = ((GfxCIDFont *)gfxFont)->getCodeToGIDMap(ff, &n);
	delete ff;
      }
      if (!(fontFile = fontEngine->loadTrueTypeFont(
			   id,
			   fontsrc,
			   codeToGID, n, faceIndex))) {
	error(errSyntaxError, -1, "Couldn't create a font for '{0:s}'",
	      gfxFont->getName() ? gfxFont->getName()->c_str()
	                         : "(unnamed)");
	if (gfxFont->invalidateEmbeddedFont()) goto reload;
	goto err2;
      }
      break;
    default:
      // this shouldn't happen
      goto err2;
    }
    fontFile->doAdjustMatrix = doAdjustFontMatrix;
  }

  // get the font matrix
  textMat = state->getTextMat();
  fontSize = state->getFontSize();
  m11 = textMat[0] * fontSize * state->getHorizScaling();
  m12 = textMat[1] * fontSize * state->getHorizScaling();
  m21 = textMat[2] * fontSize;
  m22 = textMat[3] * fontSize;

  // create the scaled font
  mat[0] = m11;  mat[1] = m12;
  mat[2] = m21;  mat[3] = m22;
  font = fontEngine->getFont(fontFile, mat, splash->getMatrix());

  // for substituted fonts: adjust the font matrix -- compare the
  // width of 'm' in the original font and the substituted font
  if (fontFile->doAdjustMatrix && !gfxFont->isCIDFont()) {
    double w1, w2, w3;
    CharCode code;
    const char *name;
    for (code = 0; code < 256; ++code) {
      if ((name = ((Gfx8BitFont *)gfxFont)->getCharName(code)) &&
          name[0] == 'm' && name[1] == '\0') {
        break;
      }
    }
    if (code < 256) {
      w1 = ((Gfx8BitFont *)gfxFont)->getWidth(code);
      w2 = font->getGlyphAdvance(code);
      w3 = ((Gfx8BitFont *)gfxFont)->getWidth(0);
      if (!gfxFont->isSymbolic() && w2 > 0 && w1 > w3) {
        // if real font is substantially narrower than substituted
        // font, reduce the font size accordingly
        if (w1 > 0.01 && w1 < 0.9 * w2) {
          w1 /= w2;
          m11 *= w1;
          m21 *= w1;
          recreateFont = true;
        }
      }
    }
  }

  if (recreateFont)
  {
    mat[0] = m11;  mat[1] = m12;
    mat[2] = m21;  mat[3] = m22;
    font = fontEngine->getFont(fontFile, mat, splash->getMatrix());
  }

  delete fontLoc;
  if (fontsrc && !fontsrc->isFile)
      fontsrc->unref();
  return;

 err2:
  delete id;
  delete fontLoc;
 err1:
  if (fontsrc && !fontsrc->isFile)
      fontsrc->unref();
  return;
}

void SplashOutputDev::stroke(GfxState *state) {
  if (state->getStrokeColorSpace()->isNonMarking()) {
    return;
  }
  setOverprintMask(state->getStrokeColorSpace(), state->getStrokeOverprint(),
		   state->getOverprintMode(), state->getStrokeColor());
  SplashPath path = convertPath(state, state->getPath(), false);
  splash->stroke(&path);
}

void SplashOutputDev::fill(GfxState *state) {
  if (state->getFillColorSpace()->isNonMarking()) {
    return;
  }
  setOverprintMask(state->getFillColorSpace(), state->getFillOverprint(),
		   state->getOverprintMode(), state->getFillColor());
  SplashPath path = convertPath(state, state->getPath(), true);
  splash->fill(&path, false);
}

void SplashOutputDev::eoFill(GfxState *state) {
  if (state->getFillColorSpace()->isNonMarking()) {
    return;
  }
  setOverprintMask(state->getFillColorSpace(), state->getFillOverprint(),
		   state->getOverprintMode(), state->getFillColor());
  SplashPath path = convertPath(state, state->getPath(), true);
  splash->fill(&path, true);
}

void SplashOutputDev::clip(GfxState *state) {
  SplashPath path = convertPath(state, state->getPath(), true);
  splash->clipToPath(&path, false);
}

void SplashOutputDev::eoClip(GfxState *state) {
  SplashPath path = convertPath(state, state->getPath(), true);
  splash->clipToPath(&path, true);
}

void SplashOutputDev::clipToStrokePath(GfxState *state) {
  SplashPath *path2;

  SplashPath path = convertPath(state, state->getPath(), false);
  path2 = splash->makeStrokePath(&path, state->getLineWidth());
  splash->clipToPath(path2, false);
  delete path2;
}

SplashPath SplashOutputDev::convertPath(GfxState *state, GfxPath *path,
					 bool dropEmptySubpaths) {
  SplashPath sPath;
  GfxSubpath *subpath;
  int n, i, j;

  n = dropEmptySubpaths ? 1 : 0;
  for (i = 0; i < path->getNumSubpaths(); ++i) {
    subpath = path->getSubpath(i);
    if (subpath->getNumPoints() > n) {
      sPath.reserve(subpath->getNumPoints() + 1);
      sPath.moveTo((SplashCoord)subpath->getX(0),
		    (SplashCoord)subpath->getY(0));
      j = 1;
      while (j < subpath->getNumPoints()) {
	if (subpath->getCurve(j)) {
	  sPath.curveTo((SplashCoord)subpath->getX(j),
			 (SplashCoord)subpath->getY(j),
			 (SplashCoord)subpath->getX(j+1),
			 (SplashCoord)subpath->getY(j+1),
			 (SplashCoord)subpath->getX(j+2),
			 (SplashCoord)subpath->getY(j+2));
	  j += 3;
	} else {
	  sPath.lineTo((SplashCoord)subpath->getX(j),
			(SplashCoord)subpath->getY(j));
	  ++j;
	}
      }
      if (subpath->isClosed()) {
	sPath.close();
      }
    }
  }
  return sPath;
}

void SplashOutputDev::drawChar(GfxState *state, double x, double y,
			       double dx, double dy,
			       double originX, double originY,
			       CharCode code, int nBytes,
			       Unicode *u, int uLen) {
  SplashPath *path;
  int render;
  bool doFill, doStroke, doClip, strokeAdjust;
  double m[4];
  bool horiz;

  if (skipHorizText || skipRotatedText) {
    state->getFontTransMat(&m[0], &m[1], &m[2], &m[3]);
    horiz = m[0] > 0 && fabs(m[1]) < 0.001 &&
            fabs(m[2]) < 0.001 && m[3] < 0;
    if ((skipHorizText && horiz) || (skipRotatedText && !horiz)) {
      return;
    }
  }

  // check for invisible text -- this is used by Acrobat Capture
  render = state->getRender();
  if (render == 3) {
    return;
  }

  if (needFontUpdate) {
    doUpdateFont(state);
  }
  if (!font) {
    return;
  }

  x -= originX;
  y -= originY;

  doFill = !(render & 1) && !state->getFillColorSpace()->isNonMarking();
  doStroke = ((render & 3) == 1 || (render & 3) == 2) &&
             !state->getStrokeColorSpace()->isNonMarking();
  doClip = render & 4;

  path = nullptr;
  SplashCoord lineWidth = splash->getLineWidth();
  if (doStroke && lineWidth == 0.0)
    splash->setLineWidth(1 / state->getVDPI());
  if (doStroke || doClip) {
    if ((path = font->getGlyphPath(code))) {
      path->offset((SplashCoord)x, (SplashCoord)y);
    }
  }

  // don't use stroke adjustment when stroking text -- the results
  // tend to be ugly (because characters with horizontal upper or
  // lower edges get misaligned relative to the other characters)
  strokeAdjust = false; // make gcc happy
  if (doStroke) {
    strokeAdjust = splash->getStrokeAdjust();
    splash->setStrokeAdjust(false);
  }

  // fill and stroke
  if (doFill && doStroke) {
    if (path) {
      setOverprintMask(state->getFillColorSpace(), state->getFillOverprint(),
		       state->getOverprintMode(), state->getFillColor());
      splash->fill(path, false);
      setOverprintMask(state->getStrokeColorSpace(),
		       state->getStrokeOverprint(),
		       state->getOverprintMode(),
		       state->getStrokeColor());
      splash->stroke(path);
    }

  // fill
  } else if (doFill) {
    setOverprintMask(state->getFillColorSpace(), state->getFillOverprint(),
		     state->getOverprintMode(), state->getFillColor());
    splash->fillChar((SplashCoord)x, (SplashCoord)y, code, font);

  // stroke
  } else if (doStroke) {
    if (path) {
      setOverprintMask(state->getStrokeColorSpace(),
		       state->getStrokeOverprint(),
		       state->getOverprintMode(),
		       state->getStrokeColor());
      splash->stroke(path);
    }
  }
  splash->setLineWidth(lineWidth);

  // clip
  if (doClip) {
    if (path) {
      if (textClipPath) {
	textClipPath->append(path);
      } else {
	textClipPath = path;
	path = nullptr;
      }
    }
  }

  if (doStroke) {
    splash->setStrokeAdjust(strokeAdjust);
  }

  if (path) {
    delete path;
  }
}

bool SplashOutputDev::beginType3Char(GfxState *state, double x, double y,
				      double dx, double dy,
				      CharCode code, Unicode *u, int uLen) {
  GfxFont *gfxFont;
  const Ref *fontID;
  const double *ctm, *bbox;
  T3FontCache *t3Font;
  T3GlyphStack *t3gs;
  bool validBBox;
  double m[4];
  bool horiz;
  double x1, y1, xMin, yMin, xMax, yMax, xt, yt;
  int i, j;

  // check for invisible text -- this is used by Acrobat Capture
  if (state->getRender() == 3) {
    // this is a bit of cheating, we say yes, font is already on cache
    // so we actually skip the rendering of it
    return true;
  }

  if (skipHorizText || skipRotatedText) {
    state->getFontTransMat(&m[0], &m[1], &m[2], &m[3]);
    horiz = m[0] > 0 && fabs(m[1]) < 0.001 &&
            fabs(m[2]) < 0.001 && m[3] < 0;
    if ((skipHorizText && horiz) || (skipRotatedText && !horiz)) {
      return true;
    }
  }

  if (!(gfxFont = state->getFont())) {
    return false;
  }
  fontID = gfxFont->getID();
  ctm = state->getCTM();
  state->transform(0, 0, &xt, &yt);

  // is it the first (MRU) font in the cache?
  if (!(nT3Fonts > 0 &&
	t3FontCache[0]->matches(fontID, ctm[0], ctm[1], ctm[2], ctm[3]))) {

    // is the font elsewhere in the cache?
    for (i = 1; i < nT3Fonts; ++i) {
      if (t3FontCache[i]->matches(fontID, ctm[0], ctm[1], ctm[2], ctm[3])) {
	t3Font = t3FontCache[i];
	for (j = i; j > 0; --j) {
	  t3FontCache[j] = t3FontCache[j - 1];
	}
	t3FontCache[0] = t3Font;
	break;
      }
    }
    if (i >= nT3Fonts) {

      // create new entry in the font cache
      if (nT3Fonts == splashOutT3FontCacheSize) {
	t3gs = t3GlyphStack;
	while (t3gs != nullptr) {
	  if (t3gs->cache == t3FontCache[nT3Fonts - 1]) {
	    error(errSyntaxWarning, -1, "t3FontCache reaches limit but font still on stack in SplashOutputDev::beginType3Char");
	    return true;
	  }
	  t3gs = t3gs->next;
	}
	delete t3FontCache[nT3Fonts - 1];
	--nT3Fonts;
      }
      for (j = nT3Fonts; j > 0; --j) {
	t3FontCache[j] = t3FontCache[j - 1];
      }
      ++nT3Fonts;
      bbox = gfxFont->getFontBBox();
      if (bbox[0] == 0 && bbox[1] == 0 && bbox[2] == 0 && bbox[3] == 0) {
	// unspecified bounding box -- just take a guess
	xMin = xt - 5;
	xMax = xMin + 30;
	yMax = yt + 15;
	yMin = yMax - 45;
	validBBox = false;
      } else {
	state->transform(bbox[0], bbox[1], &x1, &y1);
	xMin = xMax = x1;
	yMin = yMax = y1;
	state->transform(bbox[0], bbox[3], &x1, &y1);
	if (x1 < xMin) {
	  xMin = x1;
	} else if (x1 > xMax) {
	  xMax = x1;
	}
	if (y1 < yMin) {
	  yMin = y1;
	} else if (y1 > yMax) {
	  yMax = y1;
	}
	state->transform(bbox[2], bbox[1], &x1, &y1);
	if (x1 < xMin) {
	  xMin = x1;
	} else if (x1 > xMax) {
	  xMax = x1;
	}
	if (y1 < yMin) {
	  yMin = y1;
	} else if (y1 > yMax) {
	  yMax = y1;
	}
	state->transform(bbox[2], bbox[3], &x1, &y1);
	if (x1 < xMin) {
	  xMin = x1;
	} else if (x1 > xMax) {
	  xMax = x1;
	}
	if (y1 < yMin) {
	  yMin = y1;
	} else if (y1 > yMax) {
	  yMax = y1;
	}
	validBBox = true;
      }
      t3FontCache[0] = new T3FontCache(fontID, ctm[0], ctm[1], ctm[2], ctm[3],
	                               (int)floor(xMin - xt) - 2,
				       (int)floor(yMin - yt) - 2,
				       (int)ceil(xMax) - (int)floor(xMin) + 4,
				       (int)ceil(yMax) - (int)floor(yMin) + 4,
				       validBBox,
				       colorMode != splashModeMono1);
    }
  }
  t3Font = t3FontCache[0];

  // is the glyph in the cache?
  i = (code & (t3Font->cacheSets - 1)) * t3Font->cacheAssoc;
  for (j = 0; j < t3Font->cacheAssoc; ++j) {
    if (t3Font->cacheTags != nullptr) {
      if ((t3Font->cacheTags[i+j].mru & 0x8000) &&
	t3Font->cacheTags[i+j].code == code) {
        drawType3Glyph(state, t3Font, &t3Font->cacheTags[i+j],
		     t3Font->cacheData + (i+j) * t3Font->glyphSize);
        return true;
      }
    }
  }

  // push a new Type 3 glyph record
  t3gs = new T3GlyphStack();
  t3gs->next = t3GlyphStack;
  t3GlyphStack = t3gs;
  t3GlyphStack->code = code;
  t3GlyphStack->cache = t3Font;
  t3GlyphStack->cacheTag = nullptr;
  t3GlyphStack->cacheData = nullptr;
  t3GlyphStack->haveDx = false;
  t3GlyphStack->doNotCache = false;

  return false;
}

void SplashOutputDev::endType3Char(GfxState *state) {
  T3GlyphStack *t3gs;

  if (t3GlyphStack->cacheTag) {
    --nestCount;
    memcpy(t3GlyphStack->cacheData, bitmap->getDataPtr(),
	   t3GlyphStack->cache->glyphSize);
    delete bitmap;
    delete splash;
    bitmap = t3GlyphStack->origBitmap;
    splash = t3GlyphStack->origSplash;
    const double *ctm = state->getCTM();
    state->setCTM(ctm[0], ctm[1], ctm[2], ctm[3],
		  t3GlyphStack->origCTM4, t3GlyphStack->origCTM5);
    updateCTM(state, 0, 0, 0, 0, 0, 0);
    drawType3Glyph(state, t3GlyphStack->cache,
		   t3GlyphStack->cacheTag, t3GlyphStack->cacheData);
  }
  t3gs = t3GlyphStack;
  t3GlyphStack = t3gs->next;
  delete t3gs;
}

void SplashOutputDev::type3D0(GfxState *state, double wx, double wy) {
  if (likely(t3GlyphStack != nullptr)) {
    t3GlyphStack->haveDx = true;
  } else {
    error(errSyntaxWarning, -1, "t3GlyphStack was null in SplashOutputDev::type3D0");
  }
}

void SplashOutputDev::type3D1(GfxState *state, double wx, double wy,
			      double llx, double lly, double urx, double ury) {
  T3FontCache *t3Font;
  SplashColor color;
  double xt, yt, xMin, xMax, yMin, yMax, x1, y1;
  int i, j;

  // ignore multiple d0/d1 operators
  if (!t3GlyphStack || t3GlyphStack->haveDx) {
    return;
  }
  t3GlyphStack->haveDx = true;
  // don't cache if we got a gsave/grestore before the d1
  if (t3GlyphStack->doNotCache) {
    return;
  }

  if (unlikely(t3GlyphStack == nullptr)) {
    error(errSyntaxWarning, -1, "t3GlyphStack was null in SplashOutputDev::type3D1");
    return;
  }

  if (unlikely(t3GlyphStack->origBitmap != nullptr)) {
    error(errSyntaxWarning, -1, "t3GlyphStack origBitmap was not null in SplashOutputDev::type3D1");
    return;
  }

  if (unlikely(t3GlyphStack->origSplash != nullptr)) {
    error(errSyntaxWarning, -1, "t3GlyphStack origSplash was not null in SplashOutputDev::type3D1");
    return;
  }

  t3Font = t3GlyphStack->cache;

  // check for a valid bbox
  state->transform(0, 0, &xt, &yt);
  state->transform(llx, lly, &x1, &y1);
  xMin = xMax = x1;
  yMin = yMax = y1;
  state->transform(llx, ury, &x1, &y1);
  if (x1 < xMin) {
    xMin = x1;
  } else if (x1 > xMax) {
    xMax = x1;
  }
  if (y1 < yMin) {
    yMin = y1;
  } else if (y1 > yMax) {
    yMax = y1;
  }
  state->transform(urx, lly, &x1, &y1);
  if (x1 < xMin) {
    xMin = x1;
  } else if (x1 > xMax) {
    xMax = x1;
  }
  if (y1 < yMin) {
    yMin = y1;
  } else if (y1 > yMax) {
    yMax = y1;
  }
  state->transform(urx, ury, &x1, &y1);
  if (x1 < xMin) {
    xMin = x1;
  } else if (x1 > xMax) {
    xMax = x1;
  }
  if (y1 < yMin) {
    yMin = y1;
  } else if (y1 > yMax) {
    yMax = y1;
  }
  if (xMin - xt < t3Font->glyphX ||
      yMin - yt < t3Font->glyphY ||
      xMax - xt > t3Font->glyphX + t3Font->glyphW ||
      yMax - yt > t3Font->glyphY + t3Font->glyphH) {
    if (t3Font->validBBox) {
      error(errSyntaxWarning, -1, "Bad bounding box in Type 3 glyph");
    }
    return;
  }

  if (t3Font->cacheTags == nullptr)
    return;

  // allocate a cache entry
  i = (t3GlyphStack->code & (t3Font->cacheSets - 1)) * t3Font->cacheAssoc;
  for (j = 0; j < t3Font->cacheAssoc; ++j) {
    if ((t3Font->cacheTags[i+j].mru & 0x7fff) == t3Font->cacheAssoc - 1) {
      t3Font->cacheTags[i+j].mru = 0x8000;
      t3Font->cacheTags[i+j].code = t3GlyphStack->code;
      t3GlyphStack->cacheTag = &t3Font->cacheTags[i+j];
      t3GlyphStack->cacheData = t3Font->cacheData + (i+j) * t3Font->glyphSize;
    } else {
      ++t3Font->cacheTags[i+j].mru;
    }
  }

  // save state
  t3GlyphStack->origBitmap = bitmap;
  t3GlyphStack->origSplash = splash;
  const double *ctm = state->getCTM();
  t3GlyphStack->origCTM4 = ctm[4];
  t3GlyphStack->origCTM5 = ctm[5];

  // create the temporary bitmap
  if (colorMode == splashModeMono1) {
    bitmap = new SplashBitmap(t3Font->glyphW, t3Font->glyphH, 1,
			      splashModeMono1, false);
    splash = new Splash(bitmap, false,
			t3GlyphStack->origSplash->getScreen());
    color[0] = 0;
    splash->clear(color);
    color[0] = 0xff;
  } else {
    bitmap = new SplashBitmap(t3Font->glyphW, t3Font->glyphH, 1,
			      splashModeMono8, false);
    splash = new Splash(bitmap, vectorAntialias,
			t3GlyphStack->origSplash->getScreen());
    color[0] = 0x00;
    splash->clear(color);
    color[0] = 0xff;
  }
  splash->setMinLineWidth(s_minLineWidth);
  splash->setThinLineMode(splashThinLineDefault);
  splash->setFillPattern(new SplashSolidColor(color));
  splash->setStrokePattern(new SplashSolidColor(color));
  //~ this should copy other state from t3GlyphStack->origSplash?
  state->setCTM(ctm[0], ctm[1], ctm[2], ctm[3],
		-t3Font->glyphX, -t3Font->glyphY);
  updateCTM(state, 0, 0, 0, 0, 0, 0);
  ++nestCount;
}

void SplashOutputDev::drawType3Glyph(GfxState *state, T3FontCache *t3Font,
				     T3FontCacheTag * /*tag*/, unsigned char *data) {
  SplashGlyphBitmap glyph;

  setOverprintMask(state->getFillColorSpace(), state->getFillOverprint(),
		   state->getOverprintMode(), state->getFillColor());
  glyph.x = -t3Font->glyphX;
  glyph.y = -t3Font->glyphY;
  glyph.w = t3Font->glyphW;
  glyph.h = t3Font->glyphH;
  glyph.aa = colorMode != splashModeMono1;
  glyph.data = data;
  glyph.freeData = false;
  splash->fillGlyph(0, 0, &glyph);
}

void SplashOutputDev::beginTextObject(GfxState *state) {
}

void SplashOutputDev::endTextObject(GfxState *state) {
  if (textClipPath) {
    splash->clipToPath(textClipPath, false);
    delete textClipPath;
    textClipPath = nullptr;
  }
}

struct SplashOutImageMaskData {
  ImageStream *imgStr;
  bool invert;
  int width, height, y;
};

bool SplashOutputDev::imageMaskSrc(void *data, SplashColorPtr line) {
  SplashOutImageMaskData *imgMaskData = (SplashOutImageMaskData *)data;
  unsigned char *p;
  SplashColorPtr q;
  int x;

  if (imgMaskData->y == imgMaskData->height) {
    return false;
  }
  if (!(p = imgMaskData->imgStr->getLine())) {
    return false;
  }
  for (x = 0, q = line; x < imgMaskData->width; ++x) {
    *q++ = *p++ ^ imgMaskData->invert;
  }
  ++imgMaskData->y;
  return true;
}

void SplashOutputDev::drawImageMask(GfxState *state, Object *ref, Stream *str,
				    int width, int height, bool invert,
				    bool interpolate, bool inlineImg) {
  SplashCoord mat[6];
  SplashOutImageMaskData imgMaskData;

  if (state->getFillColorSpace()->isNonMarking()) {
    return;
  }
  setOverprintMask(state->getFillColorSpace(), state->getFillOverprint(),
		   state->getOverprintMode(), state->getFillColor());

  const double *ctm = state->getCTM();
  for (int i = 0; i < 6; ++i) {
    if (!std::isfinite(ctm[i])) return;
  }
  mat[0] = ctm[0];
  mat[1] = ctm[1];
  mat[2] = -ctm[2];
  mat[3] = -ctm[3];
  mat[4] = ctm[2] + ctm[4];
  mat[5] = ctm[3] + ctm[5];

  imgMaskData.imgStr = new ImageStream(str, width, 1, 1);
  imgMaskData.imgStr->reset();
  imgMaskData.invert = invert ? 0 : 1;
  imgMaskData.width = width;
  imgMaskData.height = height;
  imgMaskData.y = 0;

  splash->fillImageMask(&imageMaskSrc, &imgMaskData, width, height, mat, t3GlyphStack != nullptr);
  if (inlineImg) {
    while (imgMaskData.y < height) {
      imgMaskData.imgStr->getLine();
      ++imgMaskData.y;
    }
  }

  delete imgMaskData.imgStr;
  str->close();
}

void SplashOutputDev::setSoftMaskFromImageMask(GfxState *state,
					       Object *ref, Stream *str,
					       int width, int height,
					       bool invert,
					       bool inlineImg, double *baseMatrix) {
  const double *ctm;
  SplashCoord mat[6];
  SplashOutImageMaskData imgMaskData;
  Splash *maskSplash;
  SplashColor maskColor;
  double bbox[4] = {0, 0, 1, 1}; // default;

  if (state->getFillColorSpace()->isNonMarking()) {
    return;
  }

  ctm = state->getCTM();
  for (int i = 0; i < 6; ++i) {
    if (!std::isfinite(ctm[i])) return;
  }
  
  beginTransparencyGroup(state, bbox, nullptr, false, false, false);
  baseMatrix[4] -= transpGroupStack->tx;
  baseMatrix[5] -= transpGroupStack->ty;

  ctm = state->getCTM();
  mat[0] = ctm[0];
  mat[1] = ctm[1];
  mat[2] = -ctm[2];
  mat[3] = -ctm[3];
  mat[4] = ctm[2] + ctm[4];
  mat[5] = ctm[3] + ctm[5];
  imgMaskData.imgStr = new ImageStream(str, width, 1, 1);
  imgMaskData.imgStr->reset();
  imgMaskData.invert = invert ? 0 : 1;
  imgMaskData.width = width;
  imgMaskData.height = height;
  imgMaskData.y = 0;

  transpGroupStack->softmask = new SplashBitmap(bitmap->getWidth(), bitmap->getHeight(), 1, splashModeMono8, false);
  maskSplash = new Splash(transpGroupStack->softmask, vectorAntialias);
  maskColor[0] = 0;
  maskSplash->clear(maskColor);
  maskColor[0] = 0xff;
  maskSplash->setFillPattern(new SplashSolidColor(maskColor));
  maskSplash->fillImageMask(&imageMaskSrc, &imgMaskData,  width, height, mat, t3GlyphStack != nullptr);
  delete maskSplash;
  delete imgMaskData.imgStr;
  str->close();
}

void SplashOutputDev::unsetSoftMaskFromImageMask(GfxState *state, double *baseMatrix) {
  double bbox[4] = {0,0,1,1}; // dummy

  /* transfer mask to alpha channel! */
  // memcpy(maskBitmap->getAlphaPtr(), maskBitmap->getDataPtr(), bitmap->getRowSize() * bitmap->getHeight());
  // memset(maskBitmap->getDataPtr(), 0, bitmap->getRowSize() * bitmap->getHeight());
  if (transpGroupStack->softmask != nullptr) {
    unsigned char *dest = bitmap->getAlphaPtr();
    unsigned char *src = transpGroupStack->softmask->getDataPtr();
    for (int c= 0; c < transpGroupStack->softmask->getRowSize() * transpGroupStack->softmask->getHeight(); c++) {
      dest[c] = src[c];
    }
    delete transpGroupStack->softmask;
    transpGroupStack->softmask = nullptr;
  }
  endTransparencyGroup(state);
  baseMatrix[4] += transpGroupStack->tx;
  baseMatrix[5] += transpGroupStack->ty;
  paintTransparencyGroup(state, bbox);
}

struct SplashOutImageData {
  ImageStream *imgStr;
  GfxImageColorMap *colorMap;
  SplashColorPtr lookup;
  int *maskColors;
  SplashColorMode colorMode;
  int width, height, y;
  ImageStream *maskStr;
  GfxImageColorMap *maskColorMap;
  SplashColor matteColor;
};

#ifdef USE_CMS
bool SplashOutputDev::useIccImageSrc(void *data) {
  SplashOutImageData *imgData = (SplashOutImageData *)data;

  if (!imgData->lookup && imgData->colorMap->getColorSpace()->getMode() == csICCBased) {
    GfxICCBasedColorSpace *colorSpace = (GfxICCBasedColorSpace *) imgData->colorMap->getColorSpace();
    switch (imgData->colorMode) {
    case splashModeMono1:
    case splashModeMono8:
      if (colorSpace->getAlt() != nullptr && colorSpace->getAlt()->getMode() == csDeviceGray)
        return true;
      break;
    case splashModeXBGR8:
    case splashModeRGB8:
    case splashModeBGR8:
      if (colorSpace->getAlt() != nullptr && colorSpace->getAlt()->getMode() == csDeviceRGB)
        return true;
      break;
#ifdef SPLASH_CMYK
    case splashModeCMYK8:
      if (colorSpace->getAlt() != nullptr && colorSpace->getAlt()->getMode() == csDeviceCMYK)
        return true;
      break;
    case splashModeDeviceN8:
      if (colorSpace->getAlt() != nullptr && colorSpace->getAlt()->getMode() == csDeviceN)
        return true;
      break;
#endif
    }
  }

  return false;
}
#endif

// Clip x to lie in [0, 255].
static inline unsigned char clip255(int x) {
  return x < 0 ? 0 : x > 255 ? 255 : x;
}

bool SplashOutputDev::imageSrc(void *data, SplashColorPtr colorLine,
				unsigned char * /*alphaLine*/) {
  SplashOutImageData *imgData = (SplashOutImageData *)data;
  unsigned char *p;
  SplashColorPtr q, col;
  GfxRGB rgb;
  GfxGray gray;
#ifdef SPLASH_CMYK
  GfxCMYK cmyk;
  GfxColor deviceN;
#endif
  int nComps, x;

  if (imgData->y == imgData->height) {
    return false;
  }
  if (!(p = imgData->imgStr->getLine())) {
    int destComps = 1;
    if (imgData->colorMode == splashModeRGB8 || imgData->colorMode == splashModeBGR8)
        destComps = 3;
    else if (imgData->colorMode == splashModeXBGR8)
        destComps = 4;
#ifdef SPLASH_CMYK
    else if (imgData->colorMode == splashModeCMYK8)
        destComps = 4;
    else if (imgData->colorMode == splashModeDeviceN8)
        destComps = SPOT_NCOMPS + 4;
#endif
    memset(colorLine, 0, imgData->width * destComps);
    return false;
  }

  nComps = imgData->colorMap->getNumPixelComps();

  if (imgData->lookup) {
    switch (imgData->colorMode) {
    case splashModeMono1:
    case splashModeMono8:
      for (x = 0, q = colorLine; x < imgData->width; ++x, ++p) {
	*q++ = imgData->lookup[*p];
      }
      break;
    case splashModeRGB8:
    case splashModeBGR8:
      for (x = 0, q = colorLine; x < imgData->width; ++x, ++p) {
	col = &imgData->lookup[3 * *p];
	*q++ = col[0];
	*q++ = col[1];
	*q++ = col[2];
      }
      break;
  case splashModeXBGR8:
      for (x = 0, q = colorLine; x < imgData->width; ++x, ++p) {
	col = &imgData->lookup[4 * *p];
	*q++ = col[0];
	*q++ = col[1];
	*q++ = col[2];
	*q++ = col[3];
      }
      break;
#ifdef SPLASH_CMYK
    case splashModeCMYK8:
      for (x = 0, q = colorLine; x < imgData->width; ++x, ++p) {
	col = &imgData->lookup[4 * *p];
	*q++ = col[0];
	*q++ = col[1];
	*q++ = col[2];
	*q++ = col[3];
      }
      break;
    case splashModeDeviceN8:
      for (x = 0, q = colorLine; x < imgData->width; ++x, ++p) {
	col = &imgData->lookup[(SPOT_NCOMPS+4) * *p];
  for (int cp = 0; cp < SPOT_NCOMPS+4; cp++)
    *q++ = col[cp];
      }
      break;
#endif
    }
  } else {
    switch (imgData->colorMode) {
    case splashModeMono1:
    case splashModeMono8:
      for (x = 0, q = colorLine; x < imgData->width; ++x, p += nComps) {
	imgData->colorMap->getGray(p, &gray);
	*q++ = colToByte(gray);
      }
      break;
    case splashModeRGB8:
    case splashModeBGR8:
      if (imgData->colorMap->useRGBLine()) {
	imgData->colorMap->getRGBLine(p, (unsigned char *) colorLine, imgData->width);
      } else {
	for (x = 0, q = colorLine; x < imgData->width; ++x, p += nComps) {
	  imgData->colorMap->getRGB(p, &rgb);
	  *q++ = colToByte(rgb.r);
	  *q++ = colToByte(rgb.g);
	  *q++ = colToByte(rgb.b);
	}
      }
      break;
    case splashModeXBGR8:
      if (imgData->colorMap->useRGBLine()) {
	imgData->colorMap->getRGBXLine(p, (unsigned char *) colorLine, imgData->width);
      } else {
	for (x = 0, q = colorLine; x < imgData->width; ++x, p += nComps) {
	  imgData->colorMap->getRGB(p, &rgb);
	  *q++ = colToByte(rgb.r);
	  *q++ = colToByte(rgb.g);
	  *q++ = colToByte(rgb.b);
	  *q++ = 255;
	}
      }
      break;
#ifdef SPLASH_CMYK
    case splashModeCMYK8:
      if (imgData->colorMap->useCMYKLine()) {
	imgData->colorMap->getCMYKLine(p, (unsigned char *) colorLine, imgData->width);
      } else {
	for (x = 0, q = colorLine; x < imgData->width; ++x, p += nComps) {
	  imgData->colorMap->getCMYK(p, &cmyk);
	  *q++ = colToByte(cmyk.c);
	  *q++ = colToByte(cmyk.m);
	  *q++ = colToByte(cmyk.y);
	  *q++ = colToByte(cmyk.k);
	}
      }
      break;
    case splashModeDeviceN8:
      if (imgData->colorMap->useDeviceNLine()) {
	imgData->colorMap->getDeviceNLine(p, (unsigned char *) colorLine, imgData->width);
      } else {
        for (x = 0, q = colorLine; x < imgData->width; ++x, p += nComps) {
	  imgData->colorMap->getDeviceN(p, &deviceN);
          for (int cp = 0; cp < SPOT_NCOMPS+4; cp++)
            *q++ = colToByte(deviceN.c[cp]);
	}
      }
      break;
#endif
    }
  }

  if (imgData->maskStr != nullptr && (p = imgData->maskStr->getLine()) != nullptr) {
    int destComps = splashColorModeNComps[imgData->colorMode];
    int convComps = (imgData->colorMode == splashModeXBGR8) ? 3 : destComps;
    imgData->maskColorMap->getGrayLine(p, p, imgData->width);
    for (x = 0, q = colorLine; x < imgData->width; ++x, p++, q += destComps) {
      for (int cp = 0; cp < convComps; cp++) {
        q[cp] = (*p) ? clip255(imgData->matteColor[cp] + (int) (q[cp] - imgData->matteColor[cp]) * 255 / *p) : imgData->matteColor[cp];
      }
    }
  }
  ++imgData->y;
  return true;
}

#ifdef USE_CMS
bool SplashOutputDev::iccImageSrc(void *data, SplashColorPtr colorLine,
				unsigned char * /*alphaLine*/) {
  SplashOutImageData *imgData = (SplashOutImageData *)data;
  unsigned char *p;
  int nComps;

  if (imgData->y == imgData->height) {
    return false;
  }
  if (!(p = imgData->imgStr->getLine())) {
    int destComps = 1;
    if (imgData->colorMode == splashModeRGB8 || imgData->colorMode == splashModeBGR8)
        destComps = 3;
    else if (imgData->colorMode == splashModeXBGR8)
        destComps = 4;
#ifdef SPLASH_CMYK
    else if (imgData->colorMode == splashModeCMYK8)
        destComps = 4;
    else if (imgData->colorMode == splashModeDeviceN8)
        destComps = SPOT_NCOMPS + 4;
#endif
    memset(colorLine, 0, imgData->width * destComps);
    return false;
  }

  if (imgData->colorMode == splashModeXBGR8) {
    SplashColorPtr q;
    int x;
    for (x = 0, q = colorLine; x < imgData->width; ++x) {
      *q++ = *p++;
      *q++ = *p++;
      *q++ = *p++;
      *q++ = 255;
    }
  } else {
    nComps = imgData->colorMap->getNumPixelComps();
    memcpy(colorLine, p, imgData->width * nComps);
  }

  ++imgData->y;
  return true;
}

void SplashOutputDev::iccTransform(void *data, SplashBitmap *bitmap) {
  SplashOutImageData *imgData = (SplashOutImageData *)data;
  int nComps = imgData->colorMap->getNumPixelComps();

  unsigned char *colorLine = (unsigned char *) gmalloc(nComps * bitmap->getWidth());
  unsigned char *rgbxLine = (imgData->colorMode == splashModeXBGR8) ? (unsigned char *) gmalloc(3 * bitmap->getWidth()) : nullptr;
  for (int i = 0; i < bitmap->getHeight(); i++) {
    unsigned char *p = bitmap->getDataPtr() + i * bitmap->getRowSize();
    switch (imgData->colorMode) {
    case splashModeMono1:
    case splashModeMono8:
      imgData->colorMap->getGrayLine(p, colorLine, bitmap->getWidth());
      memcpy(p, colorLine, nComps * bitmap->getWidth());
      break;
    case splashModeRGB8:
    case splashModeBGR8:
      imgData->colorMap->getRGBLine(p, colorLine, bitmap->getWidth());
      memcpy(p, colorLine, nComps * bitmap->getWidth());
      break;
#ifdef SPLASH_CMYK
    case splashModeCMYK8:
      imgData->colorMap->getCMYKLine(p, colorLine, bitmap->getWidth());
      memcpy(p, colorLine, nComps * bitmap->getWidth());
      break;
    case splashModeDeviceN8:
      imgData->colorMap->getDeviceNLine(p, colorLine, bitmap->getWidth());
      memcpy(p, colorLine, nComps * bitmap->getWidth());
      break;
#endif
    case splashModeXBGR8:
      unsigned char *q;
      unsigned char *b = p;
      int x;
      for (x = 0, q = rgbxLine; x < bitmap->getWidth(); ++x, b+=4) {
        *q++ = b[2];
        *q++ = b[1];
        *q++ = b[0];
      }
      imgData->colorMap->getRGBLine(rgbxLine, colorLine, bitmap->getWidth());
      b = p;
      for (x = 0, q = colorLine; x < bitmap->getWidth(); ++x, b+=4) {
        b[2] = *q++;
        b[1] = *q++;
        b[0] = *q++;
      }
      break;
    }
  }
  gfree(colorLine);
  if (rgbxLine != nullptr) 
    gfree(rgbxLine);
}
#endif

bool SplashOutputDev::alphaImageSrc(void *data, SplashColorPtr colorLine,
				     unsigned char *alphaLine) {
  SplashOutImageData *imgData = (SplashOutImageData *)data;
  unsigned char *p, *aq;
  SplashColorPtr q, col;
  GfxRGB rgb;
  GfxGray gray;
#ifdef SPLASH_CMYK
  GfxCMYK cmyk;
  GfxColor deviceN;
#endif
  unsigned char alpha;
  int nComps, x, i;

  if (imgData->y == imgData->height) {
    return false;
  }
  if (!(p = imgData->imgStr->getLine())) {
    return false;
  }

  nComps = imgData->colorMap->getNumPixelComps();

  for (x = 0, q = colorLine, aq = alphaLine;
       x < imgData->width;
       ++x, p += nComps) {
    alpha = 0;
    for (i = 0; i < nComps; ++i) {
      if (p[i] < imgData->maskColors[2*i] ||
	  p[i] > imgData->maskColors[2*i+1]) {
	alpha = 0xff;
	break;
      }
    }
    if (imgData->lookup) {
      switch (imgData->colorMode) {
      case splashModeMono1:
      case splashModeMono8:
	*q++ = imgData->lookup[*p];
	break;
      case splashModeRGB8:
      case splashModeBGR8:
	col = &imgData->lookup[3 * *p];
	*q++ = col[0];
	*q++ = col[1];
	*q++ = col[2];
	break;
      case splashModeXBGR8:
	col = &imgData->lookup[4 * *p];
	*q++ = col[0];
	*q++ = col[1];
	*q++ = col[2];
	*q++ = 255;
	break;
#ifdef SPLASH_CMYK
      case splashModeCMYK8:
	col = &imgData->lookup[4 * *p];
	*q++ = col[0];
	*q++ = col[1];
	*q++ = col[2];
	*q++ = col[3];
	break;
      case splashModeDeviceN8:
	col = &imgData->lookup[(SPOT_NCOMPS+4) * *p];
  for (int cp = 0; cp < SPOT_NCOMPS+4; cp++)
    *q++ = col[cp];
  break;
#endif
      }
      *aq++ = alpha;
    } else {
      switch (imgData->colorMode) {
      case splashModeMono1:
      case splashModeMono8:
	imgData->colorMap->getGray(p, &gray);
	*q++ = colToByte(gray);
	break;
      case splashModeXBGR8:
      case splashModeRGB8:
      case splashModeBGR8:
	imgData->colorMap->getRGB(p, &rgb);
	*q++ = colToByte(rgb.r);
	*q++ = colToByte(rgb.g);
	*q++ = colToByte(rgb.b);
	if (imgData->colorMode == splashModeXBGR8) *q++ = 255;
	break;
#ifdef SPLASH_CMYK
      case splashModeCMYK8:
	imgData->colorMap->getCMYK(p, &cmyk);
	*q++ = colToByte(cmyk.c);
	*q++ = colToByte(cmyk.m);
	*q++ = colToByte(cmyk.y);
	*q++ = colToByte(cmyk.k);
	break;
    case splashModeDeviceN8:
	imgData->colorMap->getDeviceN(p, &deviceN);
  for (int cp = 0; cp < SPOT_NCOMPS+4; cp++)
    *q++ = colToByte(deviceN.c[cp]);
	break;
#endif
      }
      *aq++ = alpha;
    }
  }

  ++imgData->y;
  return true;
}

struct TilingSplashOutBitmap {
  SplashBitmap *bitmap;
  SplashPattern *pattern;
  SplashColorMode colorMode;
  int paintType;
  int repeatX;
  int repeatY;
  int y;
};

bool SplashOutputDev::tilingBitmapSrc(void *data, SplashColorPtr colorLine,
                                       unsigned char *alphaLine) {
  TilingSplashOutBitmap *imgData = (TilingSplashOutBitmap *)data;

  if (imgData->y == imgData->bitmap->getHeight()) {
    imgData->repeatY--;
    if (imgData->repeatY == 0)
      return false;
    imgData->y = 0;
  }

  if (imgData->paintType == 1) {
    const SplashColorMode cMode = imgData->bitmap->getMode();
    SplashColorPtr q = colorLine;
    // For splashModeBGR8 and splashModeXBGR8 we need to use getPixel
    // for the others we can use raw access
    if (cMode == splashModeBGR8 || cMode == splashModeXBGR8) {
      for (int m = 0; m < imgData->repeatX; m++) {
        for (int x = 0; x < imgData->bitmap->getWidth(); x++) {
          imgData->bitmap->getPixel(x, imgData->y, q);
          q += splashColorModeNComps[cMode];
        }
      }
    } else {
      const int n = imgData->bitmap->getRowSize();
      SplashColorPtr p;
      for (int m = 0; m < imgData->repeatX; m++) {
        p = imgData->bitmap->getDataPtr() + imgData->y * imgData->bitmap->getRowSize();
        for (int x = 0; x < n; ++x) {
          *q++ = *p++;
        }
      }
    }
    if (alphaLine != nullptr) {
      SplashColorPtr aq = alphaLine;
      SplashColorPtr p;
      const int n = imgData->bitmap->getWidth() - 1;
      for (int m = 0; m < imgData->repeatX; m++) {
        p = imgData->bitmap->getAlphaPtr() + imgData->y * imgData->bitmap->getWidth();
        for (int x = 0; x < n; ++x) {
          *aq++ = *p++;
        }
        // This is a hack, because of how Splash antialias works if we overwrite the
        // last alpha pixel of the tile most/all of the files look much better
        *aq++ = (n == 0) ? *p : *(p - 1);
      }
    }
  } else {
    SplashColor col, pat;
    SplashColorPtr dest = colorLine;
    for (int m = 0; m < imgData->repeatX; m++) {
      for (int x = 0; x < imgData->bitmap->getWidth(); x++) {
        imgData->bitmap->getPixel(x, imgData->y, col);
        imgData->pattern->getColor(x, imgData->y, pat);
        for (int i = 0; i < splashColorModeNComps[imgData->colorMode]; ++i) {
#ifdef SPLASH_CMYK
          if (imgData->colorMode == splashModeCMYK8 || imgData->colorMode == splashModeDeviceN8)
            dest[i] = div255(pat[i] * (255 - col[0]));
          else
#endif
            dest[i] = 255 - div255((255 - pat[i]) * (255 - col[0]));
        }
        dest += splashColorModeNComps[imgData->colorMode];
      }
    }
    if (alphaLine != nullptr) {
      const int y = (imgData->y == imgData->bitmap->getHeight() - 1 && imgData->y > 50) ? imgData->y - 1 : imgData->y;
      SplashColorPtr aq = alphaLine;
      SplashColorPtr p;
      const int n = imgData->bitmap->getWidth();
      for (int m = 0; m < imgData->repeatX; m++) {
        p = imgData->bitmap->getAlphaPtr() + y * imgData->bitmap->getWidth();
        for (int x = 0; x < n; ++x) {
          *aq++ = *p++;
        }
      }
    }
  }
  ++imgData->y;
  return true;
}

void SplashOutputDev::drawImage(GfxState *state, Object *ref, Stream *str,
				int width, int height,
				GfxImageColorMap *colorMap,
				bool interpolate,
				int *maskColors, bool inlineImg) {
  SplashCoord mat[6];
  SplashOutImageData imgData;
  SplashColorMode srcMode;
  SplashImageSource src;
  SplashICCTransform tf;
  GfxGray gray;
  GfxRGB rgb;
#ifdef SPLASH_CMYK
  GfxCMYK cmyk;
  bool grayIndexed = false;
  GfxColor deviceN;
#endif
  unsigned char pix;
  int n, i;

  const double *ctm = state->getCTM();
  for (i = 0; i < 6; ++i) {
    if (!std::isfinite(ctm[i])) return;
  }
  mat[0] = ctm[0];
  mat[1] = ctm[1];
  mat[2] = -ctm[2];
  mat[3] = -ctm[3];
  mat[4] = ctm[2] + ctm[4];
  mat[5] = ctm[3] + ctm[5];

  imgData.imgStr = new ImageStream(str, width,
				   colorMap->getNumPixelComps(),
				   colorMap->getBits());
  imgData.imgStr->reset();
  imgData.colorMap = colorMap;
  imgData.maskColors = maskColors;
  imgData.colorMode = colorMode;
  imgData.width = width;
  imgData.height = height;
  imgData.maskStr = nullptr;
  imgData.maskColorMap = nullptr;
  imgData.y = 0;

  // special case for one-channel (monochrome/gray/separation) images:
  // build a lookup table here
  imgData.lookup = nullptr;
  if (colorMap->getNumPixelComps() == 1) {
    n = 1 << colorMap->getBits();
    switch (colorMode) {
    case splashModeMono1:
    case splashModeMono8:
      imgData.lookup = (SplashColorPtr)gmalloc(n);
      for (i = 0; i < n; ++i) {
	pix = (unsigned char)i;
	colorMap->getGray(&pix, &gray);
	imgData.lookup[i] = colToByte(gray);
      }
      break;
    case splashModeRGB8:
    case splashModeBGR8:
      imgData.lookup = (SplashColorPtr)gmallocn(n, 3);
      for (i = 0; i < n; ++i) {
	pix = (unsigned char)i;
	colorMap->getRGB(&pix, &rgb);
	imgData.lookup[3*i] = colToByte(rgb.r);
	imgData.lookup[3*i+1] = colToByte(rgb.g);
	imgData.lookup[3*i+2] = colToByte(rgb.b);
      }
      break;
    case splashModeXBGR8:
      imgData.lookup = (SplashColorPtr)gmallocn_checkoverflow(n, 4);
      if (likely(imgData.lookup != nullptr)) {
	for (i = 0; i < n; ++i) {
	  pix = (unsigned char)i;
	  colorMap->getRGB(&pix, &rgb);
	  imgData.lookup[4*i] = colToByte(rgb.r);
	  imgData.lookup[4*i+1] = colToByte(rgb.g);
	  imgData.lookup[4*i+2] = colToByte(rgb.b);
	  imgData.lookup[4*i+3] = 255;
	}
      }
      break;
#ifdef SPLASH_CMYK
    case splashModeCMYK8:
      grayIndexed = colorMap->getColorSpace()->getMode() != csDeviceGray;
      imgData.lookup = (SplashColorPtr)gmallocn(n, 4);
      for (i = 0; i < n; ++i) {
	pix = (unsigned char)i;
	colorMap->getCMYK(&pix, &cmyk);
	if (cmyk.c != 0 || cmyk.m != 0 || cmyk.y != 0) {
	  grayIndexed = false;
	}
	imgData.lookup[4*i] = colToByte(cmyk.c);
	imgData.lookup[4*i+1] = colToByte(cmyk.m);
	imgData.lookup[4*i+2] = colToByte(cmyk.y);
	imgData.lookup[4*i+3] = colToByte(cmyk.k);
      }
      break;
    case splashModeDeviceN8:
      colorMap->getColorSpace()->createMapping(bitmap->getSeparationList(), SPOT_NCOMPS);
      grayIndexed = colorMap->getColorSpace()->getMode() != csDeviceGray;
      imgData.lookup = (SplashColorPtr)gmallocn(n, SPOT_NCOMPS+4);
      for (i = 0; i < n; ++i) {
        pix = (unsigned char)i;
        colorMap->getCMYK(&pix, &cmyk);
        if (cmyk.c != 0 || cmyk.m != 0 || cmyk.y != 0) {
          grayIndexed = false;
        }
        colorMap->getDeviceN(&pix, &deviceN);
        for (int cp = 0; cp < SPOT_NCOMPS+4; cp++)
          imgData.lookup[(SPOT_NCOMPS+4)*i +cp] = colToByte(deviceN.c[cp]);
      }
      break;
#endif
    }
  }

#ifdef SPLASH_CMYK
  setOverprintMask(colorMap->getColorSpace(), state->getFillOverprint(),
		   state->getOverprintMode(), nullptr, grayIndexed);
#else		   
  setOverprintMask(colorMap->getColorSpace(), state->getFillOverprint(),
		   state->getOverprintMode(), nullptr);
#endif		   

  if (colorMode == splashModeMono1) {
    srcMode = splashModeMono8;
  } else {
    srcMode = colorMode;
  }
#ifdef USE_CMS
  src = maskColors ? &alphaImageSrc : useIccImageSrc(&imgData) ? &iccImageSrc : &imageSrc;
  tf = maskColors == nullptr && useIccImageSrc(&imgData) ? &iccTransform : nullptr;
#else
  src = maskColors ? &alphaImageSrc : &imageSrc;
  tf = nullptr;
#endif
  splash->drawImage(src, tf, &imgData, srcMode, maskColors ? true : false,
		    width, height, mat, interpolate);
  if (inlineImg) {
    while (imgData.y < height) {
      imgData.imgStr->getLine();
      ++imgData.y;
    }
  }

  gfree(imgData.lookup);
  delete imgData.imgStr;
  str->close();
}

struct SplashOutMaskedImageData {
  ImageStream *imgStr;
  GfxImageColorMap *colorMap;
  SplashBitmap *mask;
  SplashColorPtr lookup;
  SplashColorMode colorMode;
  int width, height, y;
};

bool SplashOutputDev::maskedImageSrc(void *data, SplashColorPtr colorLine,
				      unsigned char *alphaLine) {
  SplashOutMaskedImageData *imgData = (SplashOutMaskedImageData *)data;
  unsigned char *p, *aq;
  SplashColorPtr q, col;
  GfxRGB rgb;
  GfxGray gray;
#ifdef SPLASH_CMYK
  GfxCMYK cmyk;
  GfxColor deviceN;
#endif
  unsigned char alpha;
  unsigned char *maskPtr;
  int maskBit;
  int nComps, x;

  if (imgData->y == imgData->height) {
    return false;
  }
  if (!(p = imgData->imgStr->getLine())) {
    return false;
  }

  nComps = imgData->colorMap->getNumPixelComps();

  maskPtr = imgData->mask->getDataPtr() +
              imgData->y * imgData->mask->getRowSize();
  maskBit = 0x80;
  for (x = 0, q = colorLine, aq = alphaLine;
       x < imgData->width;
       ++x, p += nComps) {
    alpha = (*maskPtr & maskBit) ? 0xff : 0x00;
    if (!(maskBit >>= 1)) {
      ++maskPtr;
      maskBit = 0x80;
    }
    if (imgData->lookup) {
      switch (imgData->colorMode) {
      case splashModeMono1:
      case splashModeMono8:
	*q++ = imgData->lookup[*p];
	break;
      case splashModeRGB8:
      case splashModeBGR8:
	col = &imgData->lookup[3 * *p];
	*q++ = col[0];
	*q++ = col[1];
	*q++ = col[2];
	break;
      case splashModeXBGR8:
	col = &imgData->lookup[4 * *p];
	*q++ = col[0];
	*q++ = col[1];
	*q++ = col[2];
	*q++ = 255;
	break;
#ifdef SPLASH_CMYK
      case splashModeCMYK8:
	col = &imgData->lookup[4 * *p];
	*q++ = col[0];
	*q++ = col[1];
	*q++ = col[2];
	*q++ = col[3];
	break;
      case splashModeDeviceN8:
	col = &imgData->lookup[(SPOT_NCOMPS+4) * *p];
  for (int cp = 0; cp < SPOT_NCOMPS+4; cp++)
    *q++ = col[cp];
	break;
#endif
      }
      *aq++ = alpha;
    } else {
      switch (imgData->colorMode) {
      case splashModeMono1:
      case splashModeMono8:
	imgData->colorMap->getGray(p, &gray);
	*q++ = colToByte(gray);
	break;
      case splashModeXBGR8:
      case splashModeRGB8:
      case splashModeBGR8:
	imgData->colorMap->getRGB(p, &rgb);
	*q++ = colToByte(rgb.r);
	*q++ = colToByte(rgb.g);
	*q++ = colToByte(rgb.b);
	if (imgData->colorMode == splashModeXBGR8) *q++ = 255;
	break;
#ifdef SPLASH_CMYK
      case splashModeCMYK8:
	imgData->colorMap->getCMYK(p, &cmyk);
	*q++ = colToByte(cmyk.c);
	*q++ = colToByte(cmyk.m);
	*q++ = colToByte(cmyk.y);
	*q++ = colToByte(cmyk.k);
	break;
      case splashModeDeviceN8:
	imgData->colorMap->getDeviceN(p, &deviceN);
  for (int cp = 0; cp < SPOT_NCOMPS+4; cp++)
    *q++ = colToByte(deviceN.c[cp]);
	break;
#endif
      }
      *aq++ = alpha;
    }
  }

  ++imgData->y;
  return true;
}

void SplashOutputDev::drawMaskedImage(GfxState *state, Object *ref,
				      Stream *str, int width, int height,
				      GfxImageColorMap *colorMap,
				      bool interpolate,
				      Stream *maskStr, int maskWidth,
				      int maskHeight, bool maskInvert,
				      bool maskInterpolate) {
  GfxImageColorMap *maskColorMap;
  SplashCoord mat[6];
  SplashOutMaskedImageData imgData;
  SplashOutImageMaskData imgMaskData;
  SplashColorMode srcMode;
  SplashBitmap *maskBitmap;
  Splash *maskSplash;
  SplashColor maskColor;
  GfxGray gray;
  GfxRGB rgb;
#ifdef SPLASH_CMYK
  GfxCMYK cmyk;
  GfxColor deviceN;
#endif
  unsigned char pix;
  int n, i;

#ifdef SPLASH_CMYK
  colorMap->getColorSpace()->createMapping(bitmap->getSeparationList(), SPOT_NCOMPS);
#endif
  setOverprintMask(colorMap->getColorSpace(), state->getFillOverprint(),
		   state->getOverprintMode(), nullptr);

  // If the mask is higher resolution than the image, use
  // drawSoftMaskedImage() instead.
  if (maskWidth > width || maskHeight > height) {
    Object maskDecode(new Array((xref) ? xref : doc->getXRef()));
    maskDecode.arrayAdd(Object(maskInvert ? 0 : 1));
    maskDecode.arrayAdd(Object(maskInvert ? 1 : 0));
    maskColorMap = new GfxImageColorMap(1, &maskDecode,
					new GfxDeviceGrayColorSpace());
    drawSoftMaskedImage(state, ref, str, width, height, colorMap, interpolate,
			maskStr, maskWidth, maskHeight, maskColorMap, maskInterpolate);
    delete maskColorMap;

  } else {
    //----- scale the mask image to the same size as the source image

    mat[0] = (SplashCoord)width;
    mat[1] = 0;
    mat[2] = 0;
    mat[3] = (SplashCoord)height;
    mat[4] = 0;
    mat[5] = 0;
    imgMaskData.imgStr = new ImageStream(maskStr, maskWidth, 1, 1);
    imgMaskData.imgStr->reset();
    imgMaskData.invert = maskInvert ? 0 : 1;
    imgMaskData.width = maskWidth;
    imgMaskData.height = maskHeight;
    imgMaskData.y = 0;
    maskBitmap = new SplashBitmap(width, height, 1, splashModeMono1, false);
    if (!maskBitmap->getDataPtr()) {
      delete maskBitmap;
      width = height = 1;
      maskBitmap = new SplashBitmap(width, height, 1, splashModeMono1, false);
    }
    maskSplash = new Splash(maskBitmap, false);
    maskColor[0] = 0;
    maskSplash->clear(maskColor);
    maskColor[0] = 0xff;
    maskSplash->setFillPattern(new SplashSolidColor(maskColor));
    maskSplash->fillImageMask(&imageMaskSrc, &imgMaskData,
			      maskWidth, maskHeight, mat, false);
    delete imgMaskData.imgStr;
    maskStr->close();
    delete maskSplash;

    //----- draw the source image

    const double *ctm = state->getCTM();
    for (i = 0; i < 6; ++i) {
      if (!std::isfinite(ctm[i])) {
        delete maskBitmap;
        return;
      }
    }
    mat[0] = ctm[0];
    mat[1] = ctm[1];
    mat[2] = -ctm[2];
    mat[3] = -ctm[3];
    mat[4] = ctm[2] + ctm[4];
    mat[5] = ctm[3] + ctm[5];

    imgData.imgStr = new ImageStream(str, width,
				     colorMap->getNumPixelComps(),
				     colorMap->getBits());
    imgData.imgStr->reset();
    imgData.colorMap = colorMap;
    imgData.mask = maskBitmap;
    imgData.colorMode = colorMode;
    imgData.width = width;
    imgData.height = height;
    imgData.y = 0;

    // special case for one-channel (monochrome/gray/separation) images:
    // build a lookup table here
    imgData.lookup = nullptr;
    if (colorMap->getNumPixelComps() == 1) {
      n = 1 << colorMap->getBits();
      switch (colorMode) {
      case splashModeMono1:
      case splashModeMono8:
	imgData.lookup = (SplashColorPtr)gmalloc(n);
	for (i = 0; i < n; ++i) {
	  pix = (unsigned char)i;
	  colorMap->getGray(&pix, &gray);
	  imgData.lookup[i] = colToByte(gray);
	}
	break;
      case splashModeRGB8:
      case splashModeBGR8:
	imgData.lookup = (SplashColorPtr)gmallocn(n, 3);
	for (i = 0; i < n; ++i) {
	  pix = (unsigned char)i;
	  colorMap->getRGB(&pix, &rgb);
	  imgData.lookup[3*i] = colToByte(rgb.r);
	  imgData.lookup[3*i+1] = colToByte(rgb.g);
	  imgData.lookup[3*i+2] = colToByte(rgb.b);
	}
	break;
      case splashModeXBGR8:
	imgData.lookup = (SplashColorPtr)gmallocn(n, 4);
	for (i = 0; i < n; ++i) {
	  pix = (unsigned char)i;
	  colorMap->getRGB(&pix, &rgb);
	  imgData.lookup[4*i] = colToByte(rgb.r);
	  imgData.lookup[4*i+1] = colToByte(rgb.g);
	  imgData.lookup[4*i+2] = colToByte(rgb.b);
	  imgData.lookup[4*i+3] = 255;
	}
	break;
#ifdef SPLASH_CMYK
      case splashModeCMYK8:
	imgData.lookup = (SplashColorPtr)gmallocn(n, 4);
	for (i = 0; i < n; ++i) {
	  pix = (unsigned char)i;
	  colorMap->getCMYK(&pix, &cmyk);
	  imgData.lookup[4*i] = colToByte(cmyk.c);
	  imgData.lookup[4*i+1] = colToByte(cmyk.m);
	  imgData.lookup[4*i+2] = colToByte(cmyk.y);
	  imgData.lookup[4*i+3] = colToByte(cmyk.k);
	}
	break;
      case splashModeDeviceN8:
	imgData.lookup = (SplashColorPtr)gmallocn(n, SPOT_NCOMPS+4);
	for (i = 0; i < n; ++i) {
	  pix = (unsigned char)i;
	  colorMap->getDeviceN(&pix, &deviceN);
    for (int cp = 0; cp < SPOT_NCOMPS+4; cp++)
      imgData.lookup[(SPOT_NCOMPS+4)*i + cp] = colToByte(deviceN.c[cp]);
	}
	break;
#endif
      }
    }

    if (colorMode == splashModeMono1) {
      srcMode = splashModeMono8;
    } else {
      srcMode = colorMode;
    }
    splash->drawImage(&maskedImageSrc, nullptr, &imgData, srcMode, true,
		      width, height, mat, interpolate);
    delete maskBitmap;
    gfree(imgData.lookup);
    delete imgData.imgStr;
    str->close();
  }
}

void SplashOutputDev::drawSoftMaskedImage(GfxState *state, Object * /* ref */,
					  Stream *str, int width, int height,
					  GfxImageColorMap *colorMap,
					  bool interpolate,
					  Stream *maskStr,
					  int maskWidth, int maskHeight,
					  GfxImageColorMap *maskColorMap,
					  bool maskInterpolate) {
  SplashCoord mat[6];
  SplashOutImageData imgData;
  SplashOutImageData imgMaskData;
  SplashColorMode srcMode;
  SplashBitmap *maskBitmap;
  Splash *maskSplash;
  SplashColor maskColor;
  GfxGray gray;
  GfxRGB rgb;
#ifdef SPLASH_CMYK
  GfxCMYK cmyk;
  GfxColor deviceN;
#endif
  unsigned char pix;

#ifdef SPLASH_CMYK
  colorMap->getColorSpace()->createMapping(bitmap->getSeparationList(), SPOT_NCOMPS);
#endif
  setOverprintMask(colorMap->getColorSpace(), state->getFillOverprint(),
		   state->getOverprintMode(), nullptr);

  const double *ctm = state->getCTM();
  for (int i = 0; i < 6; ++i) {
    if (!std::isfinite(ctm[i])) return;
  }
  mat[0] = ctm[0];
  mat[1] = ctm[1];
  mat[2] = -ctm[2];
  mat[3] = -ctm[3];
  mat[4] = ctm[2] + ctm[4];
  mat[5] = ctm[3] + ctm[5];

  //----- set up the soft mask

  if (maskColorMap->getMatteColor() != nullptr) {
    int maskChars;
    if (checkedMultiply(maskWidth, maskHeight, &maskChars)) {
      return;
    }
    unsigned char *data = (unsigned char *) gmalloc(maskChars);
    maskStr->reset();
    const int readChars = maskStr->doGetChars(maskChars, data);
    if (unlikely(readChars < maskChars)) {
      memset(&data[readChars], 0, maskChars - readChars);
    }
    maskStr->close();
    maskStr = new AutoFreeMemStream((char *)data, 0, maskChars, maskStr->getDictObject()->copy());
  }
  imgMaskData.imgStr = new ImageStream(maskStr, maskWidth,
				       maskColorMap->getNumPixelComps(),
				       maskColorMap->getBits());
  imgMaskData.imgStr->reset();
  imgMaskData.colorMap = maskColorMap;
  imgMaskData.maskColors = nullptr;
  imgMaskData.colorMode = splashModeMono8;
  imgMaskData.width = maskWidth;
  imgMaskData.height = maskHeight;
  imgMaskData.y = 0;
  imgMaskData.maskStr = nullptr;
  imgMaskData.maskColorMap = nullptr;
  const unsigned imgMaskDataLookupSize = 1 << maskColorMap->getBits();
  imgMaskData.lookup = (SplashColorPtr)gmalloc(imgMaskDataLookupSize);
  for (unsigned i = 0; i < imgMaskDataLookupSize; ++i) {
    pix = (unsigned char)i;
    maskColorMap->getGray(&pix, &gray);
    imgMaskData.lookup[i] = colToByte(gray);
  }
  maskBitmap = new SplashBitmap(bitmap->getWidth(), bitmap->getHeight(),
				1, splashModeMono8, false);
  maskSplash = new Splash(maskBitmap, vectorAntialias);
  maskColor[0] = 0;
  maskSplash->clear(maskColor);
  maskSplash->drawImage(&imageSrc, nullptr, &imgMaskData, splashModeMono8, false,
			maskWidth, maskHeight, mat, maskInterpolate);
  delete imgMaskData.imgStr;
  if (maskColorMap->getMatteColor() == nullptr) {
    maskStr->close();
  }
  gfree(imgMaskData.lookup);
  delete maskSplash;
  splash->setSoftMask(maskBitmap);

  //----- draw the source image

  imgData.imgStr = new ImageStream(str, width,
				   colorMap->getNumPixelComps(),
				   colorMap->getBits());
  imgData.imgStr->reset();
  imgData.colorMap = colorMap;
  imgData.maskColors = nullptr;
  imgData.colorMode = colorMode;
  imgData.width = width;
  imgData.height = height;
  imgData.maskStr = nullptr;
  imgData.maskColorMap = nullptr;
  if (maskColorMap->getMatteColor() != nullptr) {
    getMatteColor(colorMode, colorMap, maskColorMap->getMatteColor(), imgData.matteColor);
    imgData.maskColorMap = maskColorMap;
    imgData.maskStr = new ImageStream(maskStr, maskWidth,
				       maskColorMap->getNumPixelComps(),
				       maskColorMap->getBits());
    imgData.maskStr->reset();
  }
  imgData.y = 0;

  // special case for one-channel (monochrome/gray/separation) images:
  // build a lookup table here
  imgData.lookup = nullptr;
  if (colorMap->getNumPixelComps() == 1) {
    const unsigned n = 1 << colorMap->getBits();
    switch (colorMode) {
    case splashModeMono1:
    case splashModeMono8:
      imgData.lookup = (SplashColorPtr)gmalloc(n);
      for (unsigned i = 0; i < n; ++i) {
	pix = (unsigned char)i;
	colorMap->getGray(&pix, &gray);
	imgData.lookup[i] = colToByte(gray);
      }
      break;
    case splashModeRGB8:
    case splashModeBGR8:
      imgData.lookup = (SplashColorPtr)gmallocn_checkoverflow(n, 3);
      if (likely(imgData.lookup != nullptr)) {
	for (unsigned i = 0; i < n; ++i) {
	  pix = (unsigned char)i;
	  colorMap->getRGB(&pix, &rgb);
	  imgData.lookup[3*i] = colToByte(rgb.r);
	  imgData.lookup[3*i+1] = colToByte(rgb.g);
	  imgData.lookup[3*i+2] = colToByte(rgb.b);
	}
      }
      break;
    case splashModeXBGR8:
      imgData.lookup = (SplashColorPtr)gmallocn_checkoverflow(n, 4);
      if (likely(imgData.lookup != nullptr)) {
	for (unsigned i = 0; i < n; ++i) {
	  pix = (unsigned char)i;
	  colorMap->getRGB(&pix, &rgb);
	  imgData.lookup[4*i] = colToByte(rgb.r);
	  imgData.lookup[4*i+1] = colToByte(rgb.g);
	  imgData.lookup[4*i+2] = colToByte(rgb.b);
	  imgData.lookup[4*i+3] = 255;
	}
      }
      break;
#ifdef SPLASH_CMYK
    case splashModeCMYK8:
      imgData.lookup = (SplashColorPtr)gmallocn_checkoverflow(n, 4);
      if (likely(imgData.lookup != nullptr)) {
	for (unsigned i = 0; i < n; ++i) {
	  pix = (unsigned char)i;
	  colorMap->getCMYK(&pix, &cmyk);
	  imgData.lookup[4*i] = colToByte(cmyk.c);
	  imgData.lookup[4*i+1] = colToByte(cmyk.m);
	  imgData.lookup[4*i+2] = colToByte(cmyk.y);
	  imgData.lookup[4*i+3] = colToByte(cmyk.k);
	}
      }
      break;
    case splashModeDeviceN8:
      imgData.lookup = (SplashColorPtr)gmallocn_checkoverflow(n, SPOT_NCOMPS+4);
      if (likely(imgData.lookup != nullptr)) {
	for (unsigned i = 0; i < n; ++i) {
	  pix = (unsigned char)i;
	  colorMap->getDeviceN(&pix, &deviceN);
	  for (int cp = 0; cp < SPOT_NCOMPS+4; cp++)
	    imgData.lookup[(SPOT_NCOMPS+4)*i + cp] = colToByte(deviceN.c[cp]);
	}
      }
      break;
#endif
    }
  }

  if (colorMode == splashModeMono1) {
    srcMode = splashModeMono8;
  } else {
    srcMode = colorMode;
  }
  splash->drawImage(&imageSrc, nullptr, &imgData, srcMode, false, width, height, mat, interpolate);
  splash->setSoftMask(nullptr);
  gfree(imgData.lookup);
  delete imgData.maskStr;
  delete imgData.imgStr;
  if (maskColorMap->getMatteColor() != nullptr) {
    maskStr->close();
    delete maskStr;
  }
  str->close();
}

bool SplashOutputDev::checkTransparencyGroup(GfxState *state, bool knockout) {
  if (state->getFillOpacity() != 1 || 
    state->getStrokeOpacity() != 1 ||
    state->getAlphaIsShape() ||
    state->getBlendMode() != gfxBlendNormal ||
    splash->getSoftMask() != nullptr ||
    knockout) 
    return true;
  return transpGroupStack != nullptr && transpGroupStack->shape != nullptr;
}

void SplashOutputDev::beginTransparencyGroup(GfxState *state, const double *bbox,
					     GfxColorSpace *blendingColorSpace,
					     bool isolated, bool knockout,
					     bool forSoftMask) {
  SplashTransparencyGroup *transpGroup;
  SplashColor color;
  double xMin, yMin, xMax, yMax, x, y;
  int tx, ty, w, h, i;

  // transform the bbox
  state->transform(bbox[0], bbox[1], &x, &y);
  xMin = xMax = x;
  yMin = yMax = y;
  state->transform(bbox[0], bbox[3], &x, &y);
  if (x < xMin) {
    xMin = x;
  } else if (x > xMax) {
    xMax = x;
  }
  if (y < yMin) {
    yMin = y;
  } else if (y > yMax) {
    yMax = y;
  }
  state->transform(bbox[2], bbox[1], &x, &y);
  if (x < xMin) {
    xMin = x;
  } else if (x > xMax) {
    xMax = x;
  }
  if (y < yMin) {
    yMin = y;
  } else if (y > yMax) {
    yMax = y;
  }
  state->transform(bbox[2], bbox[3], &x, &y);
  if (x < xMin) {
    xMin = x;
  } else if (x > xMax) {
    xMax = x;
  }
  if (y < yMin) {
    yMin = y;
  } else if (y > yMax) {
    yMax = y;
  }
  tx = (int)floor(xMin);
  if (tx < 0) {
    tx = 0;
  } else if (tx >= bitmap->getWidth()) {
    tx = bitmap->getWidth() - 1;
  }
  ty = (int)floor(yMin);
  if (ty < 0) {
    ty = 0;
  } else if (ty >= bitmap->getHeight()) {
    ty = bitmap->getHeight() - 1;
  }
  w = (int)ceil(xMax) - tx + 1;
  if (tx + w > bitmap->getWidth()) {
    w = bitmap->getWidth() - tx;
  }
  if (w < 1) {
    w = 1;
  }
  h = (int)ceil(yMax) - ty + 1;
  if (ty + h > bitmap->getHeight()) {
    h = bitmap->getHeight() - ty;
  }
  if (h < 1) {
    h = 1;
  }

  // push a new stack entry
  transpGroup = new SplashTransparencyGroup();
  transpGroup->softmask = nullptr;
  transpGroup->tx = tx;
  transpGroup->ty = ty;
  transpGroup->blendingColorSpace = blendingColorSpace;
  transpGroup->isolated = isolated;
  transpGroup->shape = (knockout && !isolated) ? SplashBitmap::copy(bitmap) : nullptr;
  transpGroup->knockout = (knockout && isolated);
  transpGroup->knockoutOpacity = 1.0;
  transpGroup->next = transpGroupStack;
  transpGroupStack = transpGroup;

  // save state
  transpGroup->origBitmap = bitmap;
  transpGroup->origSplash = splash;
  transpGroup->fontAA = fontEngine->getAA();

  //~ this handles the blendingColorSpace arg for soft masks, but
  //~   not yet for transparency groups

  // switch to the blending color space
  if (forSoftMask && isolated && blendingColorSpace) {
    if (blendingColorSpace->getMode() == csDeviceGray ||
	blendingColorSpace->getMode() == csCalGray ||
	(blendingColorSpace->getMode() == csICCBased &&
	 blendingColorSpace->getNComps() == 1)) {
      colorMode = splashModeMono8;
    } else if (blendingColorSpace->getMode() == csDeviceRGB ||
	       blendingColorSpace->getMode() == csCalRGB ||
	       (blendingColorSpace->getMode() == csICCBased &&
		blendingColorSpace->getNComps() == 3)) {
      //~ does this need to use BGR8?
      colorMode = splashModeRGB8;
#ifdef SPLASH_CMYK
    } else if (blendingColorSpace->getMode() == csDeviceCMYK ||
	       (blendingColorSpace->getMode() == csICCBased &&
		blendingColorSpace->getNComps() == 4)) {
      colorMode = splashModeCMYK8;
#endif
    }
  }

  // create the temporary bitmap
  bitmap = new SplashBitmap(w, h, bitmapRowPad, colorMode, true,
			    bitmapTopDown, bitmap->getSeparationList());
  if (!bitmap->getDataPtr()) {
    delete bitmap;
    w = h = 1;
    bitmap = new SplashBitmap(w, h, bitmapRowPad, colorMode, true, bitmapTopDown);
  }
  splash = new Splash(bitmap, vectorAntialias,
		      transpGroup->origSplash->getScreen());
  if (transpGroup->next != nullptr && transpGroup->next->knockout) {
    fontEngine->setAA(false);
  }
  splash->setThinLineMode(transpGroup->origSplash->getThinLineMode());
  splash->setMinLineWidth(s_minLineWidth);
  //~ Acrobat apparently copies at least the fill and stroke colors, and
  //~ maybe other state(?) -- but not the clipping path (and not sure
  //~ what else)
  //~ [this is likely the same situation as in type3D1()]
  splash->setFillPattern(transpGroup->origSplash->getFillPattern()->copy());
  splash->setStrokePattern(
		         transpGroup->origSplash->getStrokePattern()->copy());
  if (isolated) {
    for (i = 0; i < splashMaxColorComps; ++i) {
      color[i] = 0;
    }
    if (colorMode == splashModeXBGR8) color[3] = 255;
    splash->clear(color, 0);
  } else {
    SplashBitmap *shape = (knockout) ? transpGroup->shape :
                                       (transpGroup->next != nullptr && transpGroup->next->shape != nullptr) ? transpGroup->next->shape : transpGroup->origBitmap;
    int shapeTx = (knockout) ? tx :
      (transpGroup->next != nullptr && transpGroup->next->shape != nullptr) ? transpGroup->next->tx + tx : tx;
    int shapeTy = (knockout) ? ty :
      (transpGroup->next != nullptr && transpGroup->next->shape != nullptr) ? transpGroup->next->ty + ty : ty;
    splash->blitTransparent(transpGroup->origBitmap, tx, ty, 0, 0, w, h);
    splash->setInNonIsolatedGroup(shape, shapeTx, shapeTy);
  }
  transpGroup->tBitmap = bitmap;
  state->shiftCTMAndClip(-tx, -ty);
  updateCTM(state, 0, 0, 0, 0, 0, 0);
  ++nestCount;
}

void SplashOutputDev::endTransparencyGroup(GfxState *state) {
  // restore state
  --nestCount;
  delete splash;
  bitmap = transpGroupStack->origBitmap;
  colorMode = bitmap->getMode();
  splash = transpGroupStack->origSplash;
  state->shiftCTMAndClip(transpGroupStack->tx, transpGroupStack->ty);
  updateCTM(state, 0, 0, 0, 0, 0, 0);
}

void SplashOutputDev::paintTransparencyGroup(GfxState *state, const double *bbox) {
  SplashBitmap *tBitmap;
  SplashTransparencyGroup *transpGroup;
  bool isolated;
  int tx, ty;

  tx = transpGroupStack->tx;
  ty = transpGroupStack->ty;
  tBitmap = transpGroupStack->tBitmap;
  isolated = transpGroupStack->isolated;

  // paint the transparency group onto the parent bitmap
  // - the clip path was set in the parent's state)
  if (tx < bitmap->getWidth() && ty < bitmap->getHeight()) {
    SplashCoord knockoutOpacity = (transpGroupStack->next != nullptr) ? transpGroupStack->next->knockoutOpacity
                                                                   : transpGroupStack->knockoutOpacity;
    splash->setOverprintMask(0xffffffff, false);
    splash->composite(tBitmap, 0, 0, tx, ty,
      tBitmap->getWidth(), tBitmap->getHeight(),
      false, !isolated, transpGroupStack->next != nullptr && transpGroupStack->next->knockout, knockoutOpacity);
    fontEngine->setAA(transpGroupStack->fontAA);
    if (transpGroupStack->next != nullptr && transpGroupStack->next->shape != nullptr) {
      transpGroupStack->next->knockout = true;
    }
  }

  // pop the stack
  transpGroup = transpGroupStack;
  transpGroupStack = transpGroup->next;
  if (transpGroupStack != nullptr && transpGroup->knockoutOpacity < transpGroupStack->knockoutOpacity) {
    transpGroupStack->knockoutOpacity = transpGroup->knockoutOpacity;
  }
  delete transpGroup->shape;
  delete transpGroup;

  delete tBitmap;
}

void SplashOutputDev::setSoftMask(GfxState *state, const double *bbox,
				  bool alpha, Function *transferFunc,
				  GfxColor *backdropColor) {
  SplashBitmap *softMask, *tBitmap;
  Splash *tSplash;
  SplashTransparencyGroup *transpGroup;
  SplashColor color;
  SplashColorPtr p;
  GfxGray gray;
  GfxRGB rgb;
#ifdef SPLASH_CMYK
  GfxCMYK cmyk;
  GfxColor deviceN;
#endif
  double lum, lum2;
  int tx, ty, x, y;

  tx = transpGroupStack->tx;
  ty = transpGroupStack->ty;
  tBitmap = transpGroupStack->tBitmap;

  // composite with backdrop color
  if (!alpha && tBitmap->getMode() != splashModeMono1) {
    //~ need to correctly handle the case where no blending color
    //~ space is given
    if (transpGroupStack->blendingColorSpace) {
      tSplash = new Splash(tBitmap, vectorAntialias,
			   transpGroupStack->origSplash->getScreen());
      switch (tBitmap->getMode()) {
      case splashModeMono1:
	// transparency is not supported in mono1 mode
	break;
      case splashModeMono8:
	transpGroupStack->blendingColorSpace->getGray(backdropColor, &gray);
	color[0] = colToByte(gray);
	tSplash->compositeBackground(color);
	break;
      case splashModeXBGR8:
	color[3] = 255;
	// fallthrough
      case splashModeRGB8:
      case splashModeBGR8:
	transpGroupStack->blendingColorSpace->getRGB(backdropColor, &rgb);
	color[0] = colToByte(rgb.r);
	color[1] = colToByte(rgb.g);
	color[2] = colToByte(rgb.b);
	tSplash->compositeBackground(color);
	break;
#ifdef SPLASH_CMYK
      case splashModeCMYK8:
	transpGroupStack->blendingColorSpace->getCMYK(backdropColor, &cmyk);
	color[0] = colToByte(cmyk.c);
	color[1] = colToByte(cmyk.m);
	color[2] = colToByte(cmyk.y);
	color[3] = colToByte(cmyk.k);
	tSplash->compositeBackground(color);
	break;
      case splashModeDeviceN8:
	transpGroupStack->blendingColorSpace->getDeviceN(backdropColor, &deviceN);
  for (int cp=0; cp < SPOT_NCOMPS+4; cp++)
    color[cp] = colToByte(deviceN.c[cp]);
	tSplash->compositeBackground(color);
	break;
#endif
      }
      delete tSplash;
    }
  }

  softMask = new SplashBitmap(bitmap->getWidth(), bitmap->getHeight(),
			      1, splashModeMono8, false);
  unsigned char fill = 0;
  if (transpGroupStack->blendingColorSpace) {
	transpGroupStack->blendingColorSpace->getGray(backdropColor, &gray);
	fill = colToByte(gray);
  }
  memset(softMask->getDataPtr(), fill,
	 softMask->getRowSize() * softMask->getHeight());
  p = softMask->getDataPtr() + ty * softMask->getRowSize() + tx;
  int xMax = tBitmap->getWidth();
  int yMax = tBitmap->getHeight();
  if (xMax > bitmap->getWidth() - tx) xMax = bitmap->getWidth() - tx;
  if (yMax > bitmap->getHeight() - ty) yMax = bitmap->getHeight() - ty;
  for (y = 0; y < yMax; ++y) {
    for (x = 0; x < xMax; ++x) {
      if (alpha) {
	if (transferFunc) {
	  lum = tBitmap->getAlpha(x, y) / 255.0;
	  transferFunc->transform(&lum, &lum2);
	  p[x] = (int)(lum2 * 255.0 + 0.5);
	} else 
	  p[x] = tBitmap->getAlpha(x, y);
      } else {
	  tBitmap->getPixel(x, y, color);
	  // convert to luminosity
	  switch (tBitmap->getMode()) {
	  case splashModeMono1:
	  case splashModeMono8:
	    lum = color[0] / 255.0;
	    break;
	  case splashModeXBGR8:
	  case splashModeRGB8:
	  case splashModeBGR8:
	    lum = (0.3 / 255.0) * color[0] +
	          (0.59 / 255.0) * color[1] +
	          (0.11 / 255.0) * color[2];
	    break;
#ifdef SPLASH_CMYK
	  case splashModeCMYK8:
    case splashModeDeviceN8:
	    lum = (1 - color[3] / 255.0)
	          - (0.3 / 255.0) * color[0]
	          - (0.59 / 255.0) * color[1]
	          - (0.11 / 255.0) * color[2];
	    if (lum < 0) {
	      lum = 0;
	    }
	    break;
#endif
	}
	if (transferFunc) {
	  transferFunc->transform(&lum, &lum2);
	} else {
	  lum2 = lum;
	}
	p[x] = (int)(lum2 * 255.0 + 0.5);
      }
    }
	p += softMask->getRowSize();
  }
  splash->setSoftMask(softMask);

  // pop the stack
  transpGroup = transpGroupStack;
  transpGroupStack = transpGroup->next;
  delete transpGroup;

  delete tBitmap;
}

void SplashOutputDev::clearSoftMask(GfxState *state) {
  splash->setSoftMask(nullptr);
}

void SplashOutputDev::setPaperColor(SplashColorPtr paperColorA) {
  splashColorCopy(paperColor, paperColorA);
}

int SplashOutputDev::getBitmapWidth() {
  return bitmap->getWidth();
}

int SplashOutputDev::getBitmapHeight() {
  return bitmap->getHeight();
}

SplashBitmap *SplashOutputDev::takeBitmap() {
  SplashBitmap *ret;

  ret = bitmap;
  bitmap = new SplashBitmap(1, 1, bitmapRowPad, colorMode,
			    colorMode != splashModeMono1, bitmapTopDown);
  return ret;
}

void SplashOutputDev::getModRegion(int *xMin, int *yMin,
				   int *xMax, int *yMax) {
  splash->getModRegion(xMin, yMin, xMax, yMax);
}

void SplashOutputDev::clearModRegion() {
  splash->clearModRegion();
}

#if 1 //~tmp: turn off anti-aliasing temporarily
bool SplashOutputDev::getVectorAntialias() {
  return splash->getVectorAntialias();
}

void SplashOutputDev::setVectorAntialias(bool vaa) {
  vaa = vaa && colorMode != splashModeMono1;
  vectorAntialias = vaa;
  splash->setVectorAntialias(vaa);
}
#endif

void SplashOutputDev::setFreeTypeHinting(bool enable, bool enableSlightHintingA)
{
  enableFreeTypeHinting = enable;
  enableSlightHinting = enableSlightHintingA;
}

bool SplashOutputDev::tilingPatternFill(GfxState *state, Gfx *gfxA, Catalog *catalog, Object *str,
					const double *ptm, int paintType, int /*tilingType*/, Dict *resDict,
					const double *mat, const double *bbox,
					int x0, int y0, int x1, int y1,
					double xStep, double yStep)
{
  PDFRectangle box;
  Gfx *gfx;
  Splash *formerSplash = splash;
  SplashBitmap *formerBitmap = bitmap;
  double width, height;
  int surface_width, surface_height, result_width, result_height, i;
  int repeatX, repeatY;
  SplashCoord matc[6];
  Matrix m1;
  const double *ctm;
  double savedCTM[6];
  double kx, ky, sx, sy;
  bool retValue = false;

  width = bbox[2] - bbox[0];
  height = bbox[3] - bbox[1];

  if (xStep != width || yStep != height)
    return false;

  // calculate offsets
  ctm = state->getCTM();
  for (i = 0; i < 6; ++i) {
    savedCTM[i] = ctm[i];
  }
  state->concatCTM(mat[0], mat[1], mat[2], mat[3], mat[4], mat[5]);
  state->concatCTM(1, 0, 0, 1, bbox[0], bbox[1]);
  ctm = state->getCTM();
  for (i = 0; i < 6; ++i) {
    if (!std::isfinite(ctm[i])) {
      state->setCTM(savedCTM[0], savedCTM[1], savedCTM[2], savedCTM[3], savedCTM[4], savedCTM[5]);
      return false;
    }
  }
  matc[4] = x0 * xStep * ctm[0] + y0 * yStep * ctm[2] + ctm[4];
  matc[5] = x0 * xStep * ctm[1] + y0 * yStep * ctm[3] + ctm[5];
  if (splashAbs(ctm[1]) > splashAbs(ctm[0])) {
    kx = -ctm[1];
    ky = ctm[2] - (ctm[0] * ctm[3]) / ctm[1];
  } else {
    kx = ctm[0];
    ky = ctm[3] - (ctm[1] * ctm[2]) / ctm[0];
  }
  result_width = (int) ceil(fabs(kx * width * (x1 - x0)));
  result_height = (int) ceil(fabs(ky * height * (y1 - y0)));
  kx = state->getHDPI() / 72.0;
  ky = state->getVDPI() / 72.0;
  m1.m[0] = (ptm[0] == 0) ? fabs(ptm[2]) * kx : fabs(ptm[0]) * kx;
  m1.m[1] = 0;
  m1.m[2] = 0;
  m1.m[3] = (ptm[3] == 0) ? fabs(ptm[1]) * ky : fabs(ptm[3]) * ky;
  m1.m[4] = 0;
  m1.m[5] = 0;
  m1.transform(width, height, &kx, &ky);
  surface_width = (int) ceil (fabs(kx));
  surface_height = (int) ceil (fabs(ky));

  sx = (double) result_width / (surface_width * (x1 - x0));
  sy = (double) result_height / (surface_height * (y1 - y0));
  m1.m[0] *= sx;
  m1.m[3] *= sy;
  m1.transform(width, height, &kx, &ky);

  if(fabs(kx) < 1 && fabs(ky) < 1) {
    kx = std::min<double>(kx, ky);
    ky = 2 / kx;
    m1.m[0] *= ky;
    m1.m[3] *= ky;
    m1.transform(width, height, &kx, &ky);
    surface_width = (int) ceil (fabs(kx));
    surface_height = (int) ceil (fabs(ky));
    repeatX = x1 - x0;
    repeatY = y1 - y0;
  } else {
    if ((unsigned long) surface_width * surface_height > 0x800000L) {
      state->setCTM(savedCTM[0], savedCTM[1], savedCTM[2], savedCTM[3], savedCTM[4], savedCTM[5]);
      return false;
    }
    while(fabs(kx) > 16384 || fabs(ky) > 16384) {
      // limit pattern bitmap size
      m1.m[0] /= 2;
      m1.m[3] /= 2;
      m1.transform(width, height, &kx, &ky);
    }
    surface_width = (int) ceil (fabs(kx));
    surface_height = (int) ceil (fabs(ky));
    // adjust repeat values to completely fill region
    repeatX = result_width / surface_width;
    repeatY = result_height / surface_height;
    if (surface_width * repeatX < result_width)
      repeatX++;
    if (surface_height * repeatY < result_height)
      repeatY++;
    if (x1 - x0 > repeatX)
      repeatX = x1 - x0;
    if (y1 - y0 > repeatY)
      repeatY = y1 - y0;
  }
  // restore CTM and calculate rotate and scale with rounded matrix
  state->setCTM(savedCTM[0], savedCTM[1], savedCTM[2], savedCTM[3], savedCTM[4], savedCTM[5]);
  state->concatCTM(mat[0], mat[1], mat[2], mat[3], mat[4], mat[5]);
  state->concatCTM(width * repeatX, 0, 0, height * repeatY, bbox[0], bbox[1]);
  ctm = state->getCTM();
  matc[0] = ctm[0];
  matc[1] = ctm[1];
  matc[2] = ctm[2];
  matc[3] = ctm[3];

  if (surface_width == 0 || surface_height == 0 || repeatX * repeatY <= 4) {
    state->setCTM(savedCTM[0], savedCTM[1], savedCTM[2], savedCTM[3], savedCTM[4], savedCTM[5]);
    return false;
  }
  m1.transform(bbox[0], bbox[1], &kx, &ky);
  m1.m[4] = -kx;
  m1.m[5] = -ky;

  bitmap = new SplashBitmap(surface_width, surface_height, 1,
                            (paintType == 1) ? colorMode : splashModeMono8, true);
  if (bitmap->getDataPtr() == nullptr) {
    SplashBitmap *tBitmap = bitmap;
    bitmap = formerBitmap;
    delete tBitmap;
    state->setCTM(savedCTM[0], savedCTM[1], savedCTM[2], savedCTM[3], savedCTM[4], savedCTM[5]);
    return false;
  }
  splash = new Splash(bitmap, true);
  if (paintType == 2) {
    SplashColor clearColor;
#ifdef SPLASH_CMYK
    clearColor[0] = (colorMode == splashModeCMYK8 || colorMode == splashModeDeviceN8) ? 0x00 : 0xFF;
#else
    clearColor[0] = 0xFF;
#endif
    splash->clear(clearColor, 0);
  } else {
    splash->clear(paperColor, 0);
  }
  splash->setThinLineMode(formerSplash->getThinLineMode());
  splash->setMinLineWidth(s_minLineWidth);

  box.x1 = bbox[0]; box.y1 = bbox[1];
  box.x2 = bbox[2]; box.y2 = bbox[3];
  gfx = new Gfx(doc, this, resDict, &box, nullptr, nullptr, nullptr, gfxA);
  // set pattern transformation matrix
  gfx->getState()->setCTM(m1.m[0], m1.m[1], m1.m[2], m1.m[3], m1.m[4], m1.m[5]);
  updateCTM(gfx->getState(), m1.m[0], m1.m[1], m1.m[2], m1.m[3], m1.m[4], m1.m[5]);
  gfx->display(str);
  delete splash;
  splash = formerSplash;
  TilingSplashOutBitmap imgData;
  imgData.bitmap = bitmap;
  imgData.paintType = paintType;
  imgData.pattern = splash->getFillPattern();
  imgData.colorMode = colorMode;
  imgData.y = 0;
  imgData.repeatX = repeatX;
  imgData.repeatY = repeatY;
  SplashBitmap *tBitmap = bitmap;
  bitmap = formerBitmap;
  result_width = tBitmap->getWidth() * imgData.repeatX;
  result_height = tBitmap->getHeight() * imgData.repeatY;

  if (splashAbs(matc[1]) > splashAbs(matc[0])) {
    kx = -matc[1];
    ky = matc[2] - (matc[0] * matc[3]) / matc[1];
  } else {
    kx = matc[0];
    ky = matc[3] - (matc[1] * matc[2]) / matc[0];
  }
  kx = result_width / (fabs(kx) + 1);
  ky = result_height / (fabs(ky) + 1);
  state->concatCTM(kx, 0, 0, ky, 0, 0);
  ctm = state->getCTM();
  matc[0] = ctm[0];
  matc[1] = ctm[1];
  matc[2] = ctm[2];
  matc[3] = ctm[3];
  bool minorAxisZero = matc[1] == 0 && matc[2] == 0;
  if (matc[0] > 0 && minorAxisZero && matc[3] > 0) {
    // draw the tiles
    for (int y = 0; y < imgData.repeatY; ++y) {
      for (int x = 0; x < imgData.repeatX; ++x) {
        x0 = splashFloor(matc[4]) + x * tBitmap->getWidth();
        y0 = splashFloor(matc[5]) + y * tBitmap->getHeight();
        splash->blitImage(tBitmap, true, x0, y0);
      }
    }
    retValue = true;
  } else {
    retValue = splash->drawImage(&tilingBitmapSrc, nullptr, &imgData, colorMode, true, result_width, result_height, matc, false, true) == splashOk;
  }
  delete tBitmap;
  delete gfx;
  return retValue;
}

bool SplashOutputDev::gouraudTriangleShadedFill(GfxState *state, GfxGouraudTriangleShading *shading)
{
  GfxColorSpaceMode shadingMode = shading->getColorSpace()->getMode();
  bool bDirectColorTranslation = false; // triggers an optimization.
  switch (colorMode) {
    case splashModeRGB8:
      bDirectColorTranslation = (shadingMode == csDeviceRGB);
    break;
#ifdef SPLASH_CMYK
    case splashModeCMYK8:
    case splashModeDeviceN8:
      bDirectColorTranslation = (shadingMode == csDeviceCMYK);
    break;
#endif
    default:
    break;
  }
  // restore vector antialias because we support it here
  if (shading->isParameterized()) {
    SplashGouraudColor *splashShading = new SplashGouraudPattern(bDirectColorTranslation, state, shading);
    bool vaa = getVectorAntialias();
    bool retVal = false;
    setVectorAntialias(true);
    retVal = splash->gouraudTriangleShadedFill(splashShading);
    setVectorAntialias(vaa);
    delete splashShading;
    return retVal;
  }
  return false;
}

bool SplashOutputDev::univariateShadedFill(GfxState *state, SplashUnivariatePattern *pattern, double tMin, double tMax) {
  double xMin, yMin, xMax, yMax;
  bool vaa = getVectorAntialias();
  // restore vector antialias because we support it here
  setVectorAntialias(true);

  bool retVal = false;
  // get the clip region bbox
  if (pattern->getShading()->getHasBBox()) {
    pattern->getShading()->getBBox(&xMin, &yMin, &xMax, &yMax);
  } else {
    state->getClipBBox(&xMin, &yMin, &xMax, &yMax);

    xMin = floor (xMin);
    yMin = floor (yMin);
    xMax = ceil (xMax);
    yMax = ceil (yMax);

    {
      Matrix ctm, ictm;
      double x[4], y[4];
      int i;

      state->getCTM(&ctm);
      ctm.invertTo(&ictm);

      ictm.transform(xMin, yMin, &x[0], &y[0]);
      ictm.transform(xMax, yMin, &x[1], &y[1]);
      ictm.transform(xMin, yMax, &x[2], &y[2]);
      ictm.transform(xMax, yMax, &x[3], &y[3]);

      xMin = xMax = x[0];
      yMin = yMax = y[0];
      for (i = 1; i < 4; i++) {
        xMin = std::min<double>(xMin, x[i]);
        yMin = std::min<double>(yMin, y[i]);
        xMax = std::max<double>(xMax, x[i]);
        yMax = std::max<double>(yMax, y[i]);
      }
    }
  }

  // fill the region
  state->moveTo(xMin, yMin);
  state->lineTo(xMax, yMin);
  state->lineTo(xMax, yMax);
  state->lineTo(xMin, yMax);
  state->closePath();
  SplashPath path = convertPath(state, state->getPath(), true);

#ifdef SPLASH_CMYK
  pattern->getShading()->getColorSpace()->createMapping(bitmap->getSeparationList(), SPOT_NCOMPS);
#endif
  setOverprintMask(pattern->getShading()->getColorSpace(), state->getFillOverprint(),
		   state->getOverprintMode(), nullptr);
  retVal = (splash->shadedFill(&path, pattern->getShading()->getHasBBox(), pattern) == splashOk);
  state->clearPath();
  setVectorAntialias(vaa);

  return retVal;
}

bool SplashOutputDev::functionShadedFill(GfxState *state, GfxFunctionShading *shading) {
  SplashFunctionPattern *pattern = new SplashFunctionPattern(colorMode, state, shading);
  double xMin, yMin, xMax, yMax;
  bool vaa = getVectorAntialias();
  // restore vector antialias because we support it here
  setVectorAntialias(true);

  bool retVal = false;
  // get the clip region bbox
  if (pattern->getShading()->getHasBBox()) {
    pattern->getShading()->getBBox(&xMin, &yMin, &xMax, &yMax);
  } else {
    state->getClipBBox(&xMin, &yMin, &xMax, &yMax);

    xMin = floor (xMin);
    yMin = floor (yMin);
    xMax = ceil (xMax);
    yMax = ceil (yMax);

    {
      Matrix ctm, ictm;
      double x[4], y[4];
      int i;

      state->getCTM(&ctm);
      ctm.invertTo(&ictm);

      ictm.transform(xMin, yMin, &x[0], &y[0]);
      ictm.transform(xMax, yMin, &x[1], &y[1]);
      ictm.transform(xMin, yMax, &x[2], &y[2]);
      ictm.transform(xMax, yMax, &x[3], &y[3]);

      xMin = xMax = x[0];
      yMin = yMax = y[0];
      for (i = 1; i < 4; i++) {
        xMin = std::min<double>(xMin, x[i]);
        yMin = std::min<double>(yMin, y[i]);
        xMax = std::max<double>(xMax, x[i]);
        yMax = std::max<double>(yMax, y[i]);
      }
    }
  }

  // fill the region
  state->moveTo(xMin, yMin);
  state->lineTo(xMax, yMin);
  state->lineTo(xMax, yMax);
  state->lineTo(xMin, yMax);
  state->closePath();
  SplashPath path = convertPath(state, state->getPath(), true);

#ifdef SPLASH_CMYK
  pattern->getShading()->getColorSpace()->createMapping(bitmap->getSeparationList(), SPOT_NCOMPS);
#endif
  setOverprintMask(pattern->getShading()->getColorSpace(), state->getFillOverprint(),
		   state->getOverprintMode(), nullptr);
  retVal = (splash->shadedFill(&path, pattern->getShading()->getHasBBox(), pattern) == splashOk);
  state->clearPath();
  setVectorAntialias(vaa);

  delete pattern;

  return retVal;
}

bool SplashOutputDev::axialShadedFill(GfxState *state, GfxAxialShading *shading, double tMin, double tMax) {
  SplashAxialPattern *pattern = new SplashAxialPattern(colorMode, state, shading);
  bool retVal = univariateShadedFill(state, pattern, tMin, tMax);

  delete pattern;

  return retVal;
}

bool SplashOutputDev::radialShadedFill(GfxState *state, GfxRadialShading *shading, double tMin, double tMax) {
  SplashRadialPattern *pattern = new SplashRadialPattern(colorMode, state, shading);
  bool retVal = univariateShadedFill(state, pattern, tMin, tMax);

  delete pattern;

  return retVal;
}
