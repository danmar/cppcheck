//========================================================================
//
// Splash.cc
//
// Copyright 2003-2013 Glyph & Cog, LLC
//
//========================================================================

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "gmem.h"
#include "gmempp.h"
#include "SplashErrorCodes.h"
#include "SplashMath.h"
#include "SplashBitmap.h"
#include "SplashState.h"
#include "SplashPath.h"
#include "SplashXPath.h"
#include "SplashXPathScanner.h"
#include "SplashPattern.h"
#include "SplashScreen.h"
#include "SplashFont.h"
#include "SplashGlyphBitmap.h"
#include "Splash.h"

// the MSVC math.h doesn't define this
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//------------------------------------------------------------------------

// distance of Bezier control point from center for circle approximation
// = (4 * (sqrt(2) - 1) / 3) * r
#define bezierCircle ((SplashCoord)0.55228475)
#define bezierCircle2 ((SplashCoord)(0.5 * 0.55228475))

// Divide a 16-bit value (in [0, 255*255]) by 255, returning an 8-bit result.
static inline Guchar div255(int x) {
  return (Guchar)((x + (x >> 8) + 0x80) >> 8);
}

// Clip x to lie in [0, 255].
static inline Guchar clip255(int x) {
  return x < 0 ? 0 : x > 255 ? 255 : (Guchar)x;
}

// Used by drawImage and fillImageMask to divide the target
// quadrilateral into sections.
struct ImageSection {
  int y0, y1;				// actual y range
  int ia0, ia1;				// vertex indices for edge A
  int ib0, ib1;				// vertex indices for edge B
  SplashCoord xa0, ya0, xa1, ya1;	// edge A
  SplashCoord dxdya;			// slope of edge A
  SplashCoord xb0, yb0, xb1, yb1;	// edge B
  SplashCoord dxdyb;			// slope of edge B
};

//------------------------------------------------------------------------
// SplashPipe
//------------------------------------------------------------------------

#define splashPipeMaxStages 9

struct SplashPipe {
  // source pattern
  SplashPattern *pattern;

  // source alpha and color
  Guchar aInput;
  SplashColor cSrcVal;

  // special cases and result color
  GBool noTransparency;
  GBool shapeOnly;
  SplashPipeResultColorCtrl resultColorCtrl;

  // non-isolated group correction
  // (this is only used when Splash::composite() is called to composite
  // a non-isolated group onto the backdrop)
  GBool nonIsolatedGroup;

  // the "run" function
  void (Splash::*run)(SplashPipe *pipe, int x0, int x1, int y,
		      Guchar *shapePtr, SplashColorPtr cSrcPtr);
};

SplashPipeResultColorCtrl Splash::pipeResultColorNoAlphaBlend[] = {
  splashPipeResultColorNoAlphaBlendMono,
  splashPipeResultColorNoAlphaBlendMono,
  splashPipeResultColorNoAlphaBlendRGB,
  splashPipeResultColorNoAlphaBlendRGB
#if SPLASH_CMYK
  ,
  splashPipeResultColorNoAlphaBlendCMYK
#endif
};

SplashPipeResultColorCtrl Splash::pipeResultColorAlphaNoBlend[] = {
  splashPipeResultColorAlphaNoBlendMono,
  splashPipeResultColorAlphaNoBlendMono,
  splashPipeResultColorAlphaNoBlendRGB,
  splashPipeResultColorAlphaNoBlendRGB
#if SPLASH_CMYK
  ,
  splashPipeResultColorAlphaNoBlendCMYK
#endif
};

SplashPipeResultColorCtrl Splash::pipeResultColorAlphaBlend[] = {
  splashPipeResultColorAlphaBlendMono,
  splashPipeResultColorAlphaBlendMono,
  splashPipeResultColorAlphaBlendRGB,
  splashPipeResultColorAlphaBlendRGB
#if SPLASH_CMYK
  ,
  splashPipeResultColorAlphaBlendCMYK
#endif
};

//------------------------------------------------------------------------
// modified region
//------------------------------------------------------------------------

void Splash::clearModRegion() {
  modXMin = bitmap->width;
  modYMin = bitmap->height;
  modXMax = -1;
  modYMax = -1;
}

inline void Splash::updateModX(int x) {
  if (x < modXMin) {
    modXMin = x;
  }
  if (x > modXMax) {
    modXMax = x;
  }
}

inline void Splash::updateModY(int y) {
  if (y < modYMin) {
    modYMin = y;
  }
  if (y > modYMax) {
    modYMax = y;
  }
}

//------------------------------------------------------------------------
// pipeline
//------------------------------------------------------------------------

inline void Splash::pipeInit(SplashPipe *pipe, SplashPattern *pattern,
			     Guchar aInput, GBool usesShape,
			     GBool nonIsolatedGroup) {
  SplashColorMode mode;

  mode = bitmap->mode;

  pipe->pattern = NULL;

  // source color
  if (pattern && pattern->isStatic()) {
    pattern->getColor(0, 0, pipe->cSrcVal);
    pipe->pattern = NULL;
  } else {
    pipe->pattern = pattern;
  }

  // source alpha
  pipe->aInput = aInput;

  // special cases
  pipe->noTransparency = aInput == 255 &&
                         !state->softMask &&
                         !usesShape &&
                         !state->inNonIsolatedGroup &&
			 !state->inKnockoutGroup &&
                         !nonIsolatedGroup &&
                         state->overprintMask == 0xffffffff;
  pipe->shapeOnly = aInput == 255 &&
                    !state->softMask &&
                    usesShape &&
                    !state->inNonIsolatedGroup &&
		    !state->inKnockoutGroup &&
                    !nonIsolatedGroup &&
                    state->overprintMask == 0xffffffff;

  // result color
  if (pipe->noTransparency) {
    // the !state->blendFunc case is handled separately in pipeRun
    pipe->resultColorCtrl = pipeResultColorNoAlphaBlend[mode];
  } else if (!state->blendFunc) {
    pipe->resultColorCtrl = pipeResultColorAlphaNoBlend[mode];
  } else {
    pipe->resultColorCtrl = pipeResultColorAlphaBlend[mode];
  }

  // non-isolated group correction
  pipe->nonIsolatedGroup = nonIsolatedGroup;

  // select the 'run' function
  pipe->run = &Splash::pipeRun;
  if (!pipe->pattern && pipe->noTransparency && !state->blendFunc) {
    if (mode == splashModeMono1 && !bitmap->alpha) {
      pipe->run = &Splash::pipeRunSimpleMono1;
    } else if (mode == splashModeMono8 && bitmap->alpha) {
      pipe->run = &Splash::pipeRunSimpleMono8;
    } else if (mode == splashModeRGB8 && bitmap->alpha) {
      pipe->run = &Splash::pipeRunSimpleRGB8;
    } else if (mode == splashModeBGR8 && bitmap->alpha) {
      pipe->run = &Splash::pipeRunSimpleBGR8;
#if SPLASH_CMYK
    } else if (mode == splashModeCMYK8 && bitmap->alpha) {
      pipe->run = &Splash::pipeRunSimpleCMYK8;
#endif
    }
  } else if (!pipe->pattern && pipe->shapeOnly && !state->blendFunc) {
    if (mode == splashModeMono1 && !bitmap->alpha) {
      pipe->run = &Splash::pipeRunShapeMono1;
    } else if (mode == splashModeMono8 && bitmap->alpha) {
      pipe->run = &Splash::pipeRunShapeMono8;
    } else if (mode == splashModeRGB8 && bitmap->alpha) {
      pipe->run = &Splash::pipeRunShapeRGB8;
    } else if (mode == splashModeBGR8 && bitmap->alpha) {
      pipe->run = &Splash::pipeRunShapeBGR8;
#if SPLASH_CMYK
    } else if (mode == splashModeCMYK8 && bitmap->alpha) {
      pipe->run = &Splash::pipeRunShapeCMYK8;
#endif
    }
  } else if (!pipe->pattern && !pipe->noTransparency && !state->softMask &&
	     usesShape &&
	     !(state->inNonIsolatedGroup && groupBackBitmap->alpha) &&
	     !state->inKnockoutGroup &&
	     !state->blendFunc && !pipe->nonIsolatedGroup) {
    if (mode == splashModeMono1 && !bitmap->alpha) {
      pipe->run = &Splash::pipeRunAAMono1;
    } else if (mode == splashModeMono8 && bitmap->alpha) {
      pipe->run = &Splash::pipeRunAAMono8;
    } else if (mode == splashModeRGB8 && bitmap->alpha) {
      pipe->run = &Splash::pipeRunAARGB8;
    } else if (mode == splashModeBGR8 && bitmap->alpha) {
      pipe->run = &Splash::pipeRunAABGR8;
#if SPLASH_CMYK
    } else if (mode == splashModeCMYK8 && bitmap->alpha) {
      pipe->run = &Splash::pipeRunAACMYK8;
#endif
    }
  }
}

// general case
void Splash::pipeRun(SplashPipe *pipe, int x0, int x1, int y,
		     Guchar *shapePtr, SplashColorPtr cSrcPtr) {
  Guchar *shapePtr2;
  Guchar shape, aSrc, aDest, alphaI, alphaIm1, alpha0, aResult;
  SplashColor cSrc, cDest, cBlend;
  Guchar shapeVal, cResult0, cResult1, cResult2, cResult3;
  int cSrcStride, shapeStride, x, lastX, t;
  SplashColorPtr destColorPtr;
  Guchar destColorMask;
  Guchar *destAlphaPtr;
  SplashColorPtr color0Ptr;
  Guchar color0Mask;
  Guchar *alpha0Ptr;
  SplashColorPtr softMaskPtr;
#if SPLASH_CMYK
  SplashColor cSrc2, cDest2;
#endif

  if (cSrcPtr && !pipe->pattern) {
    cSrcStride = bitmapComps;
  } else {
    cSrcPtr = pipe->cSrcVal;
    cSrcStride = 0;
  }

  if (shapePtr) {
    shapePtr2 = shapePtr;
    shapeStride = 1;
    for (; x0 <= x1; ++x0) {
      if (*shapePtr2) {
	break;
      }
      cSrcPtr += cSrcStride;
      ++shapePtr2;
    }
  } else {
    shapeVal = 0xff;
    shapePtr2 = &shapeVal;
    shapeStride = 0;
  }
  if (x0 > x1) {
    return;
  }
  updateModX(x0);
  updateModY(y);
  lastX = x0;

  if (bitmap->mode == splashModeMono1) {
    destColorPtr = &bitmap->data[y * bitmap->rowSize + (x0 >> 3)];
    destColorMask = (Guchar)(0x80 >> (x0 & 7));
  } else {
    destColorPtr = &bitmap->data[y * bitmap->rowSize + x0 * bitmapComps];
    destColorMask = 0; // make gcc happy
  }
  if (bitmap->alpha) {
    destAlphaPtr = &bitmap->alpha[y * bitmap->alphaRowSize + x0];
  } else {
    destAlphaPtr = NULL;
  }
  if (state->softMask) {
    softMaskPtr = &state->softMask->data[y * state->softMask->rowSize + x0];
  } else {
    softMaskPtr = NULL;
  }
  if (state->inKnockoutGroup) {
    if (bitmap->mode == splashModeMono1) {
      color0Ptr =
          &groupBackBitmap->data[(groupBackY + y) * groupBackBitmap->rowSize +
				 ((groupBackX + x0) >> 3)];
      color0Mask = (Guchar)(0x80 >> ((groupBackX + x0) & 7));
    } else {
      color0Ptr =
          &groupBackBitmap->data[(groupBackY + y) * groupBackBitmap->rowSize +
				 (groupBackX + x0) * bitmapComps];
      color0Mask = 0; // make gcc happy
    }
  } else {
    color0Ptr = NULL;
    color0Mask = 0; // make gcc happy
  }
  if (state->inNonIsolatedGroup && groupBackBitmap->alpha) {
    alpha0Ptr =
        &groupBackBitmap->alpha[(groupBackY + y)
				  * groupBackBitmap->alphaRowSize +
				(groupBackX + x0)];
  } else {
    alpha0Ptr = NULL;
  }

  for (x = x0; x <= x1; ++x) {

    //----- shape

    shape = *shapePtr2;
    if (!shape) {
      if (bitmap->mode == splashModeMono1) {
	destColorPtr += destColorMask & 1;
	destColorMask = (Guchar)((destColorMask << 7) | (destColorMask >> 1));
      } else {
	destColorPtr += bitmapComps;
      }
      if (destAlphaPtr) {
	++destAlphaPtr;
      }
      if (softMaskPtr) {
	++softMaskPtr;
      }
      if (color0Ptr) {
	if (bitmap->mode == splashModeMono1) {
	  color0Ptr += color0Mask & 1;
	  color0Mask = (Guchar)((color0Mask << 7) | (color0Mask >> 1));
	} else {
	  color0Ptr += bitmapComps;
	}
      }
      if (alpha0Ptr) {
	++alpha0Ptr;
      }
      cSrcPtr += cSrcStride;
      shapePtr2 += shapeStride;
      continue;
    }
    lastX = x;

    //----- source color

    // static pattern: handled in pipeInit
    // fixed color: handled in pipeInit

    // dynamic pattern
    if (pipe->pattern) {
      pipe->pattern->getColor(x, y, pipe->cSrcVal);
    }

    cResult0 = cResult1 = cResult2 = cResult3 = 0; // make gcc happy

    if (pipe->noTransparency && !state->blendFunc) {

      //----- result color

      switch (bitmap->mode) {
      case splashModeMono1:
      case splashModeMono8:
	cResult0 = state->grayTransfer[cSrcPtr[0]];
	break;
      case splashModeRGB8:
      case splashModeBGR8:
	cResult0 = state->rgbTransferR[cSrcPtr[0]];
	cResult1 = state->rgbTransferG[cSrcPtr[1]];
	cResult2 = state->rgbTransferB[cSrcPtr[2]];
	break;
#if SPLASH_CMYK
      case splashModeCMYK8:
	cResult0 = state->cmykTransferC[cSrcPtr[0]];
	cResult1 = state->cmykTransferM[cSrcPtr[1]];
	cResult2 = state->cmykTransferY[cSrcPtr[2]];
	cResult3 = state->cmykTransferK[cSrcPtr[3]];
	break;
#endif
      }
      aResult = 255;

    } else { // if (noTransparency && !blendFunc)

      //----- read destination pixel
      //      (or backdrop color, for knockout groups)

      if (color0Ptr) {

	switch (bitmap->mode) {
	case splashModeMono1:
	  cDest[0] = (*color0Ptr & color0Mask) ? 0xff : 0x00;
	  color0Ptr += color0Mask & 1;
	  color0Mask = (Guchar)((color0Mask << 7) | (color0Mask >> 1));
	  break;
	case splashModeMono8:
	  cDest[0] = *color0Ptr++;
	  break;
	case splashModeRGB8:
	  cDest[0] = color0Ptr[0];
	  cDest[1] = color0Ptr[1];
	  cDest[2] = color0Ptr[2];
	  color0Ptr += 3;
	  break;
	case splashModeBGR8:
	  cDest[2] = color0Ptr[0];
	  cDest[1] = color0Ptr[1];
	  cDest[0] = color0Ptr[2];
	  color0Ptr += 3;
	  break;
#if SPLASH_CMYK
	case splashModeCMYK8:
	  cDest[0] = color0Ptr[0];
	  cDest[1] = color0Ptr[1];
	  cDest[2] = color0Ptr[2];
	  cDest[3] = color0Ptr[3];
	  color0Ptr += 4;
	  break;
#endif
	}

      } else {

	switch (bitmap->mode) {
	case splashModeMono1:
	  cDest[0] = (*destColorPtr & destColorMask) ? 0xff : 0x00;
	  break;
	case splashModeMono8:
	  cDest[0] = *destColorPtr;
	  break;
	case splashModeRGB8:
	  cDest[0] = destColorPtr[0];
	  cDest[1] = destColorPtr[1];
	  cDest[2] = destColorPtr[2];
	  break;
	case splashModeBGR8:
	  cDest[0] = destColorPtr[2];
	  cDest[1] = destColorPtr[1];
	  cDest[2] = destColorPtr[0];
	  break;
#if SPLASH_CMYK
	case splashModeCMYK8:
	  cDest[0] = destColorPtr[0];
	  cDest[1] = destColorPtr[1];
	  cDest[2] = destColorPtr[2];
	  cDest[3] = destColorPtr[3];
	  break;
#endif
	}

      }

      if (destAlphaPtr) {
	aDest = *destAlphaPtr;
      } else {
	aDest = 0xff;
      }

      //----- read source color; handle overprint

      switch (bitmap->mode) {
      case splashModeMono1:
      case splashModeMono8:
	cSrc[0] = state->grayTransfer[cSrcPtr[0]];
	break;
      case splashModeRGB8:
      case splashModeBGR8:
	cSrc[0] = state->rgbTransferR[cSrcPtr[0]];
	cSrc[1] = state->rgbTransferG[cSrcPtr[1]];
	cSrc[2] = state->rgbTransferB[cSrcPtr[2]];
	break;
#if SPLASH_CMYK
      case splashModeCMYK8:
	if (state->overprintMask & 0x01) {
	  cSrc[0] = state->cmykTransferC[cSrcPtr[0]];
	} else {
	  cSrc[0] = div255(aDest * cDest[0]);
	}
	if (state->overprintMask & 0x02) {
	  cSrc[1] = state->cmykTransferM[cSrcPtr[1]];
	} else {
	  cSrc[1] = div255(aDest * cDest[1]);
	}
	if (state->overprintMask & 0x04) {
	  cSrc[2] = state->cmykTransferY[cSrcPtr[2]];
	} else {
	  cSrc[2] = div255(aDest * cDest[2]);
	}
	if (state->overprintMask & 0x08) {
	  cSrc[3] = state->cmykTransferK[cSrcPtr[3]];
	} else {
	  cSrc[3] = div255(aDest * cDest[3]);
	}
	break;
#endif
      }

      //----- source alpha

      if (softMaskPtr) {
	if (shapePtr) {
	  aSrc = div255(div255(pipe->aInput * *softMaskPtr++) * shape);
	} else {
	  aSrc = div255(pipe->aInput * *softMaskPtr++);
	}
      } else if (shapePtr) {
	aSrc = div255(pipe->aInput * shape);
      } else {
	aSrc = pipe->aInput;
      }

      //----- non-isolated group correction

      if (pipe->nonIsolatedGroup) {
	// This path is only used when Splash::composite() is called to
	// composite a non-isolated group onto the backdrop.  In this
	// case, shape is the source (group) alpha.
	t = (aDest * 255) / shape - aDest;
	switch (bitmap->mode) {
#if SPLASH_CMYK
	case splashModeCMYK8:
	  cSrc[3] = clip255(cSrc[3] + ((cSrc[3] - cDest[3]) * t) / 255);
#endif
	case splashModeRGB8:
	case splashModeBGR8:
	  cSrc[2] = clip255(cSrc[2] + ((cSrc[2] - cDest[2]) * t) / 255);
	  cSrc[1] = clip255(cSrc[1] + ((cSrc[1] - cDest[1]) * t) / 255);
	case splashModeMono1:
	case splashModeMono8:
	  cSrc[0] = clip255(cSrc[0] + ((cSrc[0] - cDest[0]) * t) / 255);
	  break;
	}
      }

      //----- blend function

      if (state->blendFunc) {
#if SPLASH_CMYK
	if (bitmap->mode == splashModeCMYK8) {
	  // convert colors to additive
	  cSrc2[0] = (Guchar)(0xff - cSrc[0]);
	  cSrc2[1] = (Guchar)(0xff - cSrc[1]);
	  cSrc2[2] = (Guchar)(0xff - cSrc[2]);
	  cSrc2[3] = (Guchar)(0xff - cSrc[3]);
	  cDest2[0] = (Guchar)(0xff - cDest[0]);
	  cDest2[1] = (Guchar)(0xff - cDest[1]);
	  cDest2[2] = (Guchar)(0xff - cDest[2]);
	  cDest2[3] = (Guchar)(0xff - cDest[3]);
	  (*state->blendFunc)(cSrc2, cDest2, cBlend, bitmap->mode);
	  // convert result back to subtractive
	  cBlend[0] = (Guchar)(0xff - cBlend[0]);
	  cBlend[1] = (Guchar)(0xff - cBlend[1]);
	  cBlend[2] = (Guchar)(0xff - cBlend[2]);
	  cBlend[3] = (Guchar)(0xff - cBlend[3]);
	} else
#endif
	(*state->blendFunc)(cSrc, cDest, cBlend, bitmap->mode);
      }

      //----- result alpha and non-isolated group element correction

      // alphaI = alpha_i
      // alphaIm1 = alpha_(i-1)

      if (pipe->noTransparency) {
	alphaI = alphaIm1 = aResult = 255;
      } else if (alpha0Ptr) {
	if (color0Ptr) {
	  // non-isolated, knockout
	  aResult = aSrc;
	  alpha0 = *alpha0Ptr++;
	  alphaI = (Guchar)(aSrc + alpha0 - div255(aSrc * alpha0));
	  alphaIm1 = alpha0;
	} else {
	  // non-isolated, non-knockout
	  aResult = (Guchar)(aSrc + aDest - div255(aSrc * aDest));
	  alpha0 = *alpha0Ptr++;
	  alphaI = (Guchar)(aResult + alpha0 - div255(aResult * alpha0));
	  alphaIm1 = (Guchar)(alpha0 + aDest - div255(alpha0 * aDest));
	}
      } else {
	if (color0Ptr) {
	  // isolated, knockout
	  aResult = aSrc;
	  alphaI = aSrc;
	  alphaIm1 = 0;
	} else {
	  // isolated, non-knockout
	  aResult = (Guchar)(aSrc + aDest - div255(aSrc * aDest));
	  alphaI = aResult;
	  alphaIm1 = aDest;
	}
      }

      //----- result color

      switch (pipe->resultColorCtrl) {

      case splashPipeResultColorNoAlphaBlendMono:
	cResult0 = div255((255 - aDest) * cSrc[0] + aDest * cBlend[0]);
	break;
      case splashPipeResultColorNoAlphaBlendRGB:
	cResult0 = div255((255 - aDest) * cSrc[0] + aDest * cBlend[0]);
	cResult1 = div255((255 - aDest) * cSrc[1] + aDest * cBlend[1]);
	cResult2 = div255((255 - aDest) * cSrc[2] + aDest * cBlend[2]);
	break;
#if SPLASH_CMYK
      case splashPipeResultColorNoAlphaBlendCMYK:
	cResult0 = div255((255 - aDest) * cSrc[0] + aDest * cBlend[0]);
	cResult1 = div255((255 - aDest) * cSrc[1] + aDest * cBlend[1]);
	cResult2 = div255((255 - aDest) * cSrc[2] + aDest * cBlend[2]);
	cResult3 = div255((255 - aDest) * cSrc[3] + aDest * cBlend[3]);
	break;
#endif

      case splashPipeResultColorAlphaNoBlendMono:
	if (alphaI == 0) {
	  cResult0 = 0;
	} else {
	  cResult0 = (Guchar)(((alphaI - aSrc) * cDest[0] + aSrc * cSrc[0])
			      / alphaI);
	}
	break;
      case splashPipeResultColorAlphaNoBlendRGB:
	if (alphaI == 0) {
	  cResult0 = 0;
	  cResult1 = 0;
	  cResult2 = 0;
	} else {
	  cResult0 = (Guchar)(((alphaI - aSrc) * cDest[0] + aSrc * cSrc[0])
			      / alphaI);
	  cResult1 = (Guchar)(((alphaI - aSrc) * cDest[1] + aSrc * cSrc[1])
			      / alphaI);
	  cResult2 = (Guchar)(((alphaI - aSrc) * cDest[2] + aSrc * cSrc[2])
			      / alphaI);
	}
	break;
#if SPLASH_CMYK
      case splashPipeResultColorAlphaNoBlendCMYK:
	if (alphaI == 0) {
	  cResult0 = 0;
	  cResult1 = 0;
	  cResult2 = 0;
	  cResult3 = 0;
	} else {
	  cResult0 = (Guchar)(((alphaI - aSrc) * cDest[0] + aSrc * cSrc[0])
			      / alphaI);
	  cResult1 = (Guchar)(((alphaI - aSrc) * cDest[1] + aSrc * cSrc[1])
			      / alphaI);
	  cResult2 = (Guchar)(((alphaI - aSrc) * cDest[2] + aSrc * cSrc[2])
			      / alphaI);
	  cResult3 = (Guchar)(((alphaI - aSrc) * cDest[3] + aSrc * cSrc[3])
			      / alphaI);
	}
	break;
#endif

      case splashPipeResultColorAlphaBlendMono:
	if (alphaI == 0) {
	  cResult0 = 0;
	} else {
	  cResult0 = (Guchar)(((alphaI - aSrc) * cDest[0] +
			       aSrc * ((255 - alphaIm1) * cSrc[0] +
				       alphaIm1 * cBlend[0]) / 255)
			      / alphaI);
	}
	break;
      case splashPipeResultColorAlphaBlendRGB:
	if (alphaI == 0) {
	  cResult0 = 0;
	  cResult1 = 0;
	  cResult2 = 0;
	} else {
	  cResult0 = (Guchar)(((alphaI - aSrc) * cDest[0] +
			       aSrc * ((255 - alphaIm1) * cSrc[0] +
				       alphaIm1 * cBlend[0]) / 255)
			      / alphaI);
	  cResult1 = (Guchar)(((alphaI - aSrc) * cDest[1] +
			       aSrc * ((255 - alphaIm1) * cSrc[1] +
				       alphaIm1 * cBlend[1]) / 255)
			      / alphaI);
	  cResult2 = (Guchar)(((alphaI - aSrc) * cDest[2] +
			       aSrc * ((255 - alphaIm1) * cSrc[2] +
				       alphaIm1 * cBlend[2]) / 255)
			      / alphaI);
	}
	break;
#if SPLASH_CMYK
      case splashPipeResultColorAlphaBlendCMYK:
	if (alphaI == 0) {
	  cResult0 = 0;
	  cResult1 = 0;
	  cResult2 = 0;
	  cResult3 = 0;
	} else {
	  cResult0 = (Guchar)(((alphaI - aSrc) * cDest[0] +
			       aSrc * ((255 - alphaIm1) * cSrc[0] +
				       alphaIm1 * cBlend[0]) / 255)
			      / alphaI);
	  cResult1 = (Guchar)(((alphaI - aSrc) * cDest[1] +
			       aSrc * ((255 - alphaIm1) * cSrc[1] +
				       alphaIm1 * cBlend[1]) / 255)
			      / alphaI);
	  cResult2 = (Guchar)(((alphaI - aSrc) * cDest[2] +
			       aSrc * ((255 - alphaIm1) * cSrc[2] +
				       alphaIm1 * cBlend[2]) / 255)
			      / alphaI);
	  cResult3 = (Guchar)(((alphaI - aSrc) * cDest[3] +
			       aSrc * ((255 - alphaIm1) * cSrc[3] +
				       alphaIm1 * cBlend[3]) / 255)
			      / alphaI);
	}
	break;
#endif
      }

    } // if (noTransparency && !blendFunc)

    //----- write destination pixel

    switch (bitmap->mode) {
    case splashModeMono1:
      if (state->screen->test(x, y, cResult0)) {
	*destColorPtr |= destColorMask;
      } else {
	*destColorPtr &= (Guchar)~destColorMask;
      }
      destColorPtr += destColorMask & 1;
      destColorMask = (Guchar)((destColorMask << 7) | (destColorMask >> 1));
      break;
    case splashModeMono8:
      *destColorPtr++ = cResult0;
      break;
    case splashModeRGB8:
      destColorPtr[0] = cResult0;
      destColorPtr[1] = cResult1;
      destColorPtr[2] = cResult2;
      destColorPtr += 3;
      break;
    case splashModeBGR8:
      destColorPtr[0] = cResult2;
      destColorPtr[1] = cResult1;
      destColorPtr[2] = cResult0;
      destColorPtr += 3;
      break;
#if SPLASH_CMYK
    case splashModeCMYK8:
      destColorPtr[0] = cResult0;
      destColorPtr[1] = cResult1;
      destColorPtr[2] = cResult2;
      destColorPtr[3] = cResult3;
      destColorPtr += 4;
      break;
#endif
    }
    if (destAlphaPtr) {
      *destAlphaPtr++ = aResult;
    }

    cSrcPtr += cSrcStride;
    shapePtr2 += shapeStride;
  } // for (x ...)

  updateModX(lastX);
}

// special case:
// !pipe->pattern && pipe->noTransparency && !state->blendFunc &&
// bitmap->mode == splashModeMono1 && !bitmap->alpha) {
void Splash::pipeRunSimpleMono1(SplashPipe *pipe, int x0, int x1, int y,
				Guchar *shapePtr, SplashColorPtr cSrcPtr) {
  Guchar cResult0;
  SplashColorPtr destColorPtr;
  Guchar destColorMask;
  SplashScreenCursor screenCursor;
  int cSrcStride, x;

  if (cSrcPtr) {
    cSrcStride = 1;
  } else {
    cSrcPtr = pipe->cSrcVal;
    cSrcStride = 0;
  }
  if (x0 > x1) {
    return;
  }
  updateModX(x0);
  updateModX(x1);
  updateModY(y);

  destColorPtr = &bitmap->data[y * bitmap->rowSize + (x0 >> 3)];
  destColorMask = (Guchar)(0x80 >> (x0 & 7));

  screenCursor = state->screen->getTestCursor(y);

  for (x = x0; x <= x1; ++x) {

    //----- write destination pixel
    cResult0 = state->grayTransfer[cSrcPtr[0]];
    if (state->screen->testWithCursor(screenCursor, x, cResult0)) {
      *destColorPtr |= destColorMask;
    } else {
      *destColorPtr &= (Guchar)~destColorMask;
    }
    destColorPtr += destColorMask & 1;
    destColorMask = (Guchar)((destColorMask << 7) | (destColorMask >> 1));

    cSrcPtr += cSrcStride;
  }
}

// special case:
// !pipe->pattern && pipe->noTransparency && !state->blendFunc &&
// bitmap->mode == splashModeMono8 && bitmap->alpha) {
void Splash::pipeRunSimpleMono8(SplashPipe *pipe, int x0, int x1, int y,
				Guchar *shapePtr, SplashColorPtr cSrcPtr) {
  SplashColorPtr destColorPtr;
  Guchar *destAlphaPtr;
  int cSrcStride, x;

  if (cSrcPtr) {
    cSrcStride = 1;
  } else {
    cSrcPtr = pipe->cSrcVal;
    cSrcStride = 0;
  }
  if (x0 > x1) {
    return;
  }
  updateModX(x0);
  updateModX(x1);
  updateModY(y);

  destColorPtr = &bitmap->data[y * bitmap->rowSize + x0];
  destAlphaPtr = &bitmap->alpha[y * bitmap->alphaRowSize + x0];

  for (x = x0; x <= x1; ++x) {

    //----- write destination pixel
    *destColorPtr++ = state->grayTransfer[cSrcPtr[0]];
    *destAlphaPtr++ = 255;

    cSrcPtr += cSrcStride;
  }
}

// special case:
// !pipe->pattern && pipe->noTransparency && !state->blendFunc &&
// bitmap->mode == splashModeRGB8 && bitmap->alpha) {
void Splash::pipeRunSimpleRGB8(SplashPipe *pipe, int x0, int x1, int y,
			       Guchar *shapePtr, SplashColorPtr cSrcPtr) {
  SplashColorPtr destColorPtr;
  Guchar *destAlphaPtr;
  int cSrcStride, x;

  if (cSrcPtr) {
    cSrcStride = 3;
  } else {
    cSrcPtr = pipe->cSrcVal;
    cSrcStride = 0;
  }
  if (x0 > x1) {
    return;
  }
  updateModX(x0);
  updateModX(x1);
  updateModY(y);

  destColorPtr = &bitmap->data[y * bitmap->rowSize + 3 * x0];
  destAlphaPtr = &bitmap->alpha[y * bitmap->alphaRowSize + x0];

  for (x = x0; x <= x1; ++x) {

    //----- write destination pixel
    destColorPtr[0] = state->rgbTransferR[cSrcPtr[0]];
    destColorPtr[1] = state->rgbTransferG[cSrcPtr[1]];
    destColorPtr[2] = state->rgbTransferB[cSrcPtr[2]];
    destColorPtr += 3;
    *destAlphaPtr++ = 255;

    cSrcPtr += cSrcStride;
  }
}

// special case:
// !pipe->pattern && pipe->noTransparency && !state->blendFunc &&
// bitmap->mode == splashModeBGR8 && bitmap->alpha) {
void Splash::pipeRunSimpleBGR8(SplashPipe *pipe, int x0, int x1, int y,
			       Guchar *shapePtr, SplashColorPtr cSrcPtr) {
  SplashColorPtr destColorPtr;
  Guchar *destAlphaPtr;
  int cSrcStride, x;

  if (cSrcPtr) {
    cSrcStride = 3;
  } else {
    cSrcPtr = pipe->cSrcVal;
    cSrcStride = 0;
  }
  if (x0 > x1) {
    return;
  }
  updateModX(x0);
  updateModX(x1);
  updateModY(y);

  destColorPtr = &bitmap->data[y * bitmap->rowSize + 3 * x0];
  destAlphaPtr = &bitmap->alpha[y * bitmap->alphaRowSize + x0];

  for (x = x0; x <= x1; ++x) {

    //----- write destination pixel
    destColorPtr[0] = state->rgbTransferB[cSrcPtr[2]];
    destColorPtr[1] = state->rgbTransferG[cSrcPtr[1]];
    destColorPtr[2] = state->rgbTransferR[cSrcPtr[0]];
    destColorPtr += 3;
    *destAlphaPtr++ = 255;

    cSrcPtr += cSrcStride;
  }
}

#if SPLASH_CMYK
// special case:
// !pipe->pattern && pipe->noTransparency && !state->blendFunc &&
// bitmap->mode == splashModeCMYK8 && bitmap->alpha) {
void Splash::pipeRunSimpleCMYK8(SplashPipe *pipe, int x0, int x1, int y,
				Guchar *shapePtr, SplashColorPtr cSrcPtr) {
  SplashColorPtr destColorPtr;
  Guchar *destAlphaPtr;
  int cSrcStride, x;

  if (cSrcPtr) {
    cSrcStride = 4;
  } else {
    cSrcPtr = pipe->cSrcVal;
    cSrcStride = 0;
  }
  if (x0 > x1) {
    return;
  }
  updateModX(x0);
  updateModX(x1);
  updateModY(y);

  destColorPtr = &bitmap->data[y * bitmap->rowSize + 4 * x0];
  destAlphaPtr = &bitmap->alpha[y * bitmap->alphaRowSize + x0];

  for (x = x0; x <= x1; ++x) {

    //----- write destination pixel
    destColorPtr[0] = state->cmykTransferC[cSrcPtr[0]];
    destColorPtr[1] = state->cmykTransferM[cSrcPtr[1]];
    destColorPtr[2] = state->cmykTransferY[cSrcPtr[2]];
    destColorPtr[3] = state->cmykTransferK[cSrcPtr[3]];
    destColorPtr += 4;
    *destAlphaPtr++ = 255;

    cSrcPtr += cSrcStride;
  }
}
#endif


// special case:
// !pipe->pattern && pipe->shapeOnly && !state->blendFunc &&
// bitmap->mode == splashModeMono1 && !bitmap->alpha
void Splash::pipeRunShapeMono1(SplashPipe *pipe, int x0, int x1, int y,
			       Guchar *shapePtr, SplashColorPtr cSrcPtr) {
  Guchar shape, aSrc, cSrc0, cDest0, cResult0;
  SplashColorPtr destColorPtr;
  Guchar destColorMask;
  SplashScreenCursor screenCursor;
  int cSrcStride, x, lastX;

  if (cSrcPtr) {
    cSrcStride = 1;
  } else {
    cSrcPtr = pipe->cSrcVal;
    cSrcStride = 0;
  }
  for (; x0 <= x1; ++x0) {
    if (*shapePtr) {
      break;
    }
    cSrcPtr += cSrcStride;
    ++shapePtr;
  }
  if (x0 > x1) {
    return;
  }
  updateModX(x0);
  updateModY(y);
  lastX = x0;

  destColorPtr = &bitmap->data[y * bitmap->rowSize + (x0 >> 3)];
  destColorMask = (Guchar)(0x80 >> (x0 & 7));

  screenCursor = state->screen->getTestCursor(y);

  for (x = x0; x <= x1; ++x) {

    //----- shape
    shape = *shapePtr;
    if (!shape) {
      destColorPtr += destColorMask & 1;
      destColorMask = (Guchar)((destColorMask << 7) | (destColorMask >> 1));
      cSrcPtr += cSrcStride;
      ++shapePtr;
      continue;
    }
    lastX = x;

    //----- source color
    cSrc0 = state->grayTransfer[cSrcPtr[0]];

    //----- source alpha
    aSrc = shape;

    //----- special case for aSrc = 255
    if (aSrc == 255) {
      cResult0 = cSrc0;
    } else {

      //----- read destination pixel
      cDest0 = (*destColorPtr & destColorMask) ? 0xff : 0x00;

      //----- result color
      // note: aDest = alphaI = aResult = 0xff
      cResult0 = (Guchar)div255((0xff - aSrc) * cDest0 + aSrc * cSrc0);
    }

    //----- write destination pixel
    if (state->screen->testWithCursor(screenCursor, x, cResult0)) {
      *destColorPtr |= destColorMask;
    } else {
      *destColorPtr &= (Guchar)~destColorMask;
    }
    destColorPtr += destColorMask & 1;
    destColorMask = (Guchar)((destColorMask << 7) | (destColorMask >> 1));

    cSrcPtr += cSrcStride;
    ++shapePtr;
  }

  updateModX(lastX);
}

// special case:
// !pipe->pattern && pipe->shapeOnly && !state->blendFunc &&
// bitmap->mode == splashModeMono8 && bitmap->alpha
void Splash::pipeRunShapeMono8(SplashPipe *pipe, int x0, int x1, int y,
			       Guchar *shapePtr, SplashColorPtr cSrcPtr) {
  Guchar shape, aSrc, aDest, alphaI, aResult, cSrc0, cDest0, cResult0;
  SplashColorPtr destColorPtr;
  Guchar *destAlphaPtr;
  int cSrcStride, x, lastX;

  if (cSrcPtr) {
    cSrcStride = 1;
  } else {
    cSrcPtr = pipe->cSrcVal;
    cSrcStride = 0;
  }
  for (; x0 <= x1; ++x0) {
    if (*shapePtr) {
      break;
    }
    cSrcPtr += cSrcStride;
    ++shapePtr;
  }
  if (x0 > x1) {
    return;
  }
  updateModX(x0);
  updateModY(y);
  lastX = x0;

  destColorPtr = &bitmap->data[y * bitmap->rowSize + x0];
  destAlphaPtr = &bitmap->alpha[y * bitmap->alphaRowSize + x0];

  for (x = x0; x <= x1; ++x) {

    //----- shape
    shape = *shapePtr;
    if (!shape) {
      ++destColorPtr;
      ++destAlphaPtr;
      cSrcPtr += cSrcStride;
      ++shapePtr;
      continue;
    }
    lastX = x;

    //----- source color
    cSrc0 = state->grayTransfer[cSrcPtr[0]];

    //----- source alpha
    aSrc = shape;

    //----- special case for aSrc = 255
    if (aSrc == 255) {
      aResult = 255;
      cResult0 = cSrc0;
    } else {

      //----- read destination alpha
      aDest = *destAlphaPtr;

      //----- special case for aDest = 0
      if (aDest == 0) {
	aResult = aSrc;
	cResult0 = cSrc0;
      } else {

	//----- read destination pixel
	cDest0 = *destColorPtr;

	//----- result alpha and non-isolated group element correction
	aResult = (Guchar)(aSrc + aDest - div255(aSrc * aDest));
	alphaI = aResult;

	//----- result color
	cResult0 = (Guchar)(((alphaI - aSrc) * cDest0 + aSrc * cSrc0) / alphaI);
      }
    }

    //----- write destination pixel
    *destColorPtr++ = cResult0;
    *destAlphaPtr++ = aResult;

    cSrcPtr += cSrcStride;
    ++shapePtr;
  }

  updateModX(lastX);
}

// special case:
// !pipe->pattern && pipe->shapeOnly && !state->blendFunc &&
// bitmap->mode == splashModeRGB8 && bitmap->alpha
void Splash::pipeRunShapeRGB8(SplashPipe *pipe, int x0, int x1, int y,
			      Guchar *shapePtr, SplashColorPtr cSrcPtr) {
  Guchar shape, aSrc, aDest, alphaI, aResult;
  Guchar cSrc0, cSrc1, cSrc2;
  Guchar cDest0, cDest1, cDest2;
  Guchar cResult0, cResult1, cResult2;
  SplashColorPtr destColorPtr;
  Guchar *destAlphaPtr;
  int cSrcStride, x, lastX;

  if (cSrcPtr) {
    cSrcStride = 3;
  } else {
    cSrcPtr = pipe->cSrcVal;
    cSrcStride = 0;
  }
  for (; x0 <= x1; ++x0) {
    if (*shapePtr) {
      break;
    }
    cSrcPtr += cSrcStride;
    ++shapePtr;
  }
  if (x0 > x1) {
    return;
  }
  updateModX(x0);
  updateModY(y);
  lastX = x0;

  destColorPtr = &bitmap->data[y * bitmap->rowSize + 3 * x0];
  destAlphaPtr = &bitmap->alpha[y * bitmap->alphaRowSize + x0];

  for (x = x0; x <= x1; ++x) {

    //----- shape
    shape = *shapePtr;
    if (!shape) {
      destColorPtr += 3;
      ++destAlphaPtr;
      cSrcPtr += cSrcStride;
      ++shapePtr;
      continue;
    }
    lastX = x;

    //----- source color
    cSrc0 = state->rgbTransferR[cSrcPtr[0]];
    cSrc1 = state->rgbTransferG[cSrcPtr[1]];
    cSrc2 = state->rgbTransferB[cSrcPtr[2]];

    //----- source alpha
    aSrc = shape;

    //----- special case for aSrc = 255
    if (aSrc == 255) {
      aResult = 255;
      cResult0 = cSrc0;
      cResult1 = cSrc1;
      cResult2 = cSrc2;
    } else {

      //----- read destination alpha
      aDest = *destAlphaPtr;

      //----- special case for aDest = 0
      if (aDest == 0) {
	aResult = aSrc;
	cResult0 = cSrc0;
	cResult1 = cSrc1;
	cResult2 = cSrc2;
      } else {

	//----- read destination pixel
	cDest0 = destColorPtr[0];
	cDest1 = destColorPtr[1];
	cDest2 = destColorPtr[2];

	//----- result alpha and non-isolated group element correction
	aResult = (Guchar)(aSrc + aDest - div255(aSrc * aDest));
	alphaI = aResult;

	//----- result color
	cResult0 = (Guchar)(((alphaI - aSrc) * cDest0 + aSrc * cSrc0) / alphaI);
	cResult1 = (Guchar)(((alphaI - aSrc) * cDest1 + aSrc * cSrc1) / alphaI);
	cResult2 = (Guchar)(((alphaI - aSrc) * cDest2 + aSrc * cSrc2) / alphaI);
      }
    }

    //----- write destination pixel
    destColorPtr[0] = cResult0;
    destColorPtr[1] = cResult1;
    destColorPtr[2] = cResult2;
    destColorPtr += 3;
    *destAlphaPtr++ = aResult;

    cSrcPtr += cSrcStride;
    ++shapePtr;
  }

  updateModX(lastX);
}

// special case:
// !pipe->pattern && pipe->shapeOnly && !state->blendFunc &&
// bitmap->mode == splashModeBGR8 && bitmap->alpha
void Splash::pipeRunShapeBGR8(SplashPipe *pipe, int x0, int x1, int y,
			      Guchar *shapePtr, SplashColorPtr cSrcPtr) {
  Guchar shape, aSrc, aDest, alphaI, aResult;
  Guchar cSrc0, cSrc1, cSrc2;
  Guchar cDest0, cDest1, cDest2;
  Guchar cResult0, cResult1, cResult2;
  SplashColorPtr destColorPtr;
  Guchar *destAlphaPtr;
  int cSrcStride, x, lastX;

  if (cSrcPtr) {
    cSrcStride = 3;
  } else {
    cSrcPtr = pipe->cSrcVal;
    cSrcStride = 0;
  }
  for (; x0 <= x1; ++x0) {
    if (*shapePtr) {
      break;
    }
    cSrcPtr += cSrcStride;
    ++shapePtr;
  }
  if (x0 > x1) {
    return;
  }
  updateModX(x0);
  updateModY(y);
  lastX = x0;

  destColorPtr = &bitmap->data[y * bitmap->rowSize + 3 * x0];
  destAlphaPtr = &bitmap->alpha[y * bitmap->alphaRowSize + x0];

  for (x = x0; x <= x1; ++x) {

    //----- shape
    shape = *shapePtr;
    if (!shape) {
      destColorPtr += 3;
      ++destAlphaPtr;
      cSrcPtr += cSrcStride;
      ++shapePtr;
      continue;
    }
    lastX = x;

    //----- source color
    cSrc0 = state->rgbTransferR[cSrcPtr[0]];
    cSrc1 = state->rgbTransferG[cSrcPtr[1]];
    cSrc2 = state->rgbTransferB[cSrcPtr[2]];

    //----- source alpha
    aSrc = shape;

    //----- special case for aSrc = 255
    if (aSrc == 255) {
      aResult = 255;
      cResult0 = cSrc0;
      cResult1 = cSrc1;
      cResult2 = cSrc2;
    } else {

      //----- read destination alpha
      aDest = *destAlphaPtr;

      //----- special case for aDest = 0
      if (aDest == 0) {
	aResult = aSrc;
	cResult0 = cSrc0;
	cResult1 = cSrc1;
	cResult2 = cSrc2;
      } else {

	//----- read destination pixel
	cDest0 = destColorPtr[2];
	cDest1 = destColorPtr[1];
	cDest2 = destColorPtr[0];

	//----- result alpha and non-isolated group element correction
	aResult = (Guchar)(aSrc + aDest - div255(aSrc * aDest));
	alphaI = aResult;

	//----- result color
	cResult0 = (Guchar)(((alphaI - aSrc) * cDest0 + aSrc * cSrc0) / alphaI);
	cResult1 = (Guchar)(((alphaI - aSrc) * cDest1 + aSrc * cSrc1) / alphaI);
	cResult2 = (Guchar)(((alphaI - aSrc) * cDest2 + aSrc * cSrc2) / alphaI);
      }
    }

    //----- write destination pixel
    destColorPtr[0] = cResult2;
    destColorPtr[1] = cResult1;
    destColorPtr[2] = cResult0;
    destColorPtr += 3;
    *destAlphaPtr++ = aResult;

    cSrcPtr += cSrcStride;
    ++shapePtr;
  }

  updateModX(lastX);
}

#if SPLASH_CMYK
// special case:
// !pipe->pattern && pipe->shapeOnly && !state->blendFunc &&
// bitmap->mode == splashModeCMYK8 && bitmap->alpha
void Splash::pipeRunShapeCMYK8(SplashPipe *pipe, int x0, int x1, int y,
			       Guchar *shapePtr, SplashColorPtr cSrcPtr) {
  Guchar shape, aSrc, aDest, alphaI, aResult;
  Guchar cSrc0, cSrc1, cSrc2, cSrc3;
  Guchar cDest0, cDest1, cDest2, cDest3;
  Guchar cResult0, cResult1, cResult2, cResult3;
  SplashColorPtr destColorPtr;
  Guchar *destAlphaPtr;
  int cSrcStride, x, lastX;

  if (cSrcPtr) {
    cSrcStride = 4;
  } else {
    cSrcPtr = pipe->cSrcVal;
    cSrcStride = 0;
  }
  for (; x0 <= x1; ++x0) {
    if (*shapePtr) {
      break;
    }
    cSrcPtr += cSrcStride;
    ++shapePtr;
  }
  if (x0 > x1) {
    return;
  }
  updateModX(x0);
  updateModY(y);
  lastX = x0;

  destColorPtr = &bitmap->data[y * bitmap->rowSize + 4 * x0];
  destAlphaPtr = &bitmap->alpha[y * bitmap->alphaRowSize + x0];

  for (x = x0; x <= x1; ++x) {

    //----- shape
    shape = *shapePtr;
    if (!shape) {
      destColorPtr += 4;
      ++destAlphaPtr;
      cSrcPtr += cSrcStride;
      ++shapePtr;
      continue;
    }
    lastX = x;

    //----- read destination pixel
    cDest0 = destColorPtr[0];
    cDest1 = destColorPtr[1];
    cDest2 = destColorPtr[2];
    cDest3 = destColorPtr[3];
    aDest = *destAlphaPtr;

    //----- overprint
    if (state->overprintMask & 1) {
      cSrc0 = state->cmykTransferC[cSrcPtr[0]];
    } else {
      cSrc0 = div255(aDest * cDest0);
    }
    if (state->overprintMask & 2) {
      cSrc1 = state->cmykTransferM[cSrcPtr[1]];
    } else {
      cSrc1 = div255(aDest * cDest1);
    }
    if (state->overprintMask & 4) {
      cSrc2 = state->cmykTransferY[cSrcPtr[2]];
    } else {
      cSrc2 = div255(aDest * cDest2);
    }
    if (state->overprintMask & 8) {
      cSrc3 = state->cmykTransferK[cSrcPtr[3]];
    } else {
      cSrc3 = div255(aDest * cDest3);
    }

    //----- source alpha
    aSrc = shape;

    //----- special case for aSrc = 255
    if (aSrc == 255) {
      aResult = 255;
      cResult0 = cSrc0;
      cResult1 = cSrc1;
      cResult2 = cSrc2;
      cResult3 = cSrc3;
    } else {

      //----- special case for aDest = 0
      if (aDest == 0) {
	aResult = aSrc;
	cResult0 = cSrc0;
	cResult1 = cSrc1;
	cResult2 = cSrc2;
	cResult3 = cSrc3;
      } else {

	//----- result alpha and non-isolated group element correction
	aResult = (Guchar)(aSrc + aDest - div255(aSrc * aDest));
	alphaI = aResult;

	//----- result color
	cResult0 = (Guchar)(((alphaI - aSrc) * cDest0 + aSrc * cSrc0) / alphaI);
	cResult1 = (Guchar)(((alphaI - aSrc) * cDest1 + aSrc * cSrc1) / alphaI);
	cResult2 = (Guchar)(((alphaI - aSrc) * cDest2 + aSrc * cSrc2) / alphaI);
	cResult3 = (Guchar)(((alphaI - aSrc) * cDest3 + aSrc * cSrc3) / alphaI);
      }
    }

    //----- write destination pixel
    destColorPtr[0] = cResult0;
    destColorPtr[1] = cResult1;
    destColorPtr[2] = cResult2;
    destColorPtr[3] = cResult3;
    destColorPtr += 4;
    *destAlphaPtr++ = aResult;

    cSrcPtr += cSrcStride;
    ++shapePtr;
  }

  updateModX(lastX);
}
#endif


// special case:
// !pipe->pattern && !pipe->noTransparency && !state->softMask &&
// pipe->usesShape && !pipe->alpha0Ptr && !state->blendFunc &&
// !pipe->nonIsolatedGroup &&
// bitmap->mode == splashModeMono1 && !bitmap->alpha
void Splash::pipeRunAAMono1(SplashPipe *pipe, int x0, int x1, int y,
			    Guchar *shapePtr, SplashColorPtr cSrcPtr) {
  Guchar shape, aSrc, cSrc0, cDest0, cResult0;
  SplashColorPtr destColorPtr;
  Guchar destColorMask;
  SplashScreenCursor screenCursor;
  int cSrcStride, x, lastX;

  if (cSrcPtr) {
    cSrcStride = 1;
  } else {
    cSrcPtr = pipe->cSrcVal;
    cSrcStride = 0;
  }
  for (; x0 <= x1; ++x0) {
    if (*shapePtr) {
      break;
    }
    cSrcPtr += cSrcStride;
    ++shapePtr;
  }
  if (x0 > x1) {
    return;
  }
  updateModX(x0);
  updateModY(y);
  lastX = x0;

  destColorPtr = &bitmap->data[y * bitmap->rowSize + (x0 >> 3)];
  destColorMask = (Guchar)(0x80 >> (x0 & 7));

  screenCursor = state->screen->getTestCursor(y);

  for (x = x0; x <= x1; ++x) {

    //----- shape
    shape = *shapePtr;
    if (!shape) {
      destColorPtr += destColorMask & 1;
      destColorMask = (Guchar)((destColorMask << 7) | (destColorMask >> 1));
      cSrcPtr += cSrcStride;
      ++shapePtr;
      continue;
    }
    lastX = x;

    //----- read destination pixel
    cDest0 = (*destColorPtr & destColorMask) ? 0xff : 0x00;

    //----- source color
    cSrc0 = state->grayTransfer[cSrcPtr[0]];

    //----- source alpha
    aSrc = div255(pipe->aInput * shape);

    //----- result color
    // note: aDest = alphaI = aResult = 0xff
    cResult0 = (Guchar)div255((0xff - aSrc) * cDest0 + aSrc * cSrc0);

    //----- write destination pixel
    if (state->screen->testWithCursor(screenCursor, x, cResult0)) {
      *destColorPtr |= destColorMask;
    } else {
      *destColorPtr &= (Guchar)~destColorMask;
    }
    destColorPtr += destColorMask & 1;
    destColorMask = (Guchar)((destColorMask << 7) | (destColorMask >> 1));

    cSrcPtr += cSrcStride;
    ++shapePtr;
  }

  updateModX(lastX);
}

// special case:
// !pipe->pattern && !pipe->noTransparency && !state->softMask &&
// pipe->usesShape && !pipe->alpha0Ptr && !state->blendFunc &&
// !pipe->nonIsolatedGroup &&
// bitmap->mode == splashModeMono8 && bitmap->alpha
void Splash::pipeRunAAMono8(SplashPipe *pipe, int x0, int x1, int y,
			    Guchar *shapePtr, SplashColorPtr cSrcPtr) {
  Guchar shape, aSrc, aDest, alphaI, aResult, cSrc0, cDest0, cResult0;
  SplashColorPtr destColorPtr;
  Guchar *destAlphaPtr;
  int cSrcStride, x, lastX;

  if (cSrcPtr) {
    cSrcStride = 1;
  } else {
    cSrcPtr = pipe->cSrcVal;
    cSrcStride = 0;
  }
  for (; x0 <= x1; ++x0) {
    if (*shapePtr) {
      break;
    }
    cSrcPtr += cSrcStride;
    ++shapePtr;
  }
  if (x0 > x1) {
    return;
  }
  updateModX(x0);
  updateModY(y);
  lastX = x0;

  destColorPtr = &bitmap->data[y * bitmap->rowSize + x0];
  destAlphaPtr = &bitmap->alpha[y * bitmap->alphaRowSize + x0];

  for (x = x0; x <= x1; ++x) {

    //----- shape
    shape = *shapePtr;
    if (!shape) {
      ++destColorPtr;
      ++destAlphaPtr;
      cSrcPtr += cSrcStride;
      ++shapePtr;
      continue;
    }
    lastX = x;

    //----- read destination pixel
    cDest0 = *destColorPtr;
    aDest = *destAlphaPtr;

    //----- source color
    cSrc0 = state->grayTransfer[cSrcPtr[0]];

    //----- source alpha
    aSrc = div255(pipe->aInput * shape);

    //----- result alpha and non-isolated group element correction
    aResult = (Guchar)(aSrc + aDest - div255(aSrc * aDest));
    alphaI = aResult;

    //----- result color
    if (alphaI == 0) {
      cResult0 = 0;
    } else {
      cResult0 = (Guchar)(((alphaI - aSrc) * cDest0 + aSrc * cSrc0) / alphaI);
    }

    //----- write destination pixel
    *destColorPtr++ = cResult0;
    *destAlphaPtr++ = aResult;

    cSrcPtr += cSrcStride;
    ++shapePtr;
  }

  updateModX(lastX);
}

// special case:
// !pipe->pattern && !pipe->noTransparency && !state->softMask &&
// pipe->usesShape && !pipe->alpha0Ptr && !state->blendFunc &&
// !pipe->nonIsolatedGroup &&
// bitmap->mode == splashModeRGB8 && bitmap->alpha
void Splash::pipeRunAARGB8(SplashPipe *pipe, int x0, int x1, int y,
			   Guchar *shapePtr, SplashColorPtr cSrcPtr) {
  Guchar shape, aSrc, aDest, alphaI, aResult;
  Guchar cSrc0, cSrc1, cSrc2;
  Guchar cDest0, cDest1, cDest2;
  Guchar cResult0, cResult1, cResult2;
  SplashColorPtr destColorPtr;
  Guchar *destAlphaPtr;
  int cSrcStride, x, lastX;

  if (cSrcPtr) {
    cSrcStride = 3;
  } else {
    cSrcPtr = pipe->cSrcVal;
    cSrcStride = 0;
  }
  for (; x0 <= x1; ++x0) {
    if (*shapePtr) {
      break;
    }
    cSrcPtr += cSrcStride;
    ++shapePtr;
  }
  if (x0 > x1) {
    return;
  }
  updateModX(x0);
  updateModY(y);
  lastX = x0;

  destColorPtr = &bitmap->data[y * bitmap->rowSize + 3 * x0];
  destAlphaPtr = &bitmap->alpha[y * bitmap->alphaRowSize + x0];

  for (x = x0; x <= x1; ++x) {

    //----- shape
    shape = *shapePtr;
    if (!shape) {
      destColorPtr += 3;
      ++destAlphaPtr;
      cSrcPtr += cSrcStride;
      ++shapePtr;
      continue;
    }
    lastX = x;

    //----- read destination pixel
    cDest0 = destColorPtr[0];
    cDest1 = destColorPtr[1];
    cDest2 = destColorPtr[2];
    aDest = *destAlphaPtr;

    //----- source color
    cSrc0 = state->rgbTransferR[cSrcPtr[0]];
    cSrc1 = state->rgbTransferG[cSrcPtr[1]];
    cSrc2 = state->rgbTransferB[cSrcPtr[2]];

    //----- source alpha
    aSrc = div255(pipe->aInput * shape);

    //----- result alpha and non-isolated group element correction
    aResult = (Guchar)(aSrc + aDest - div255(aSrc * aDest));
    alphaI = aResult;

    //----- result color
    if (alphaI == 0) {
      cResult0 = 0;
      cResult1 = 0;
      cResult2 = 0;
    } else {
      cResult0 = (Guchar)(((alphaI - aSrc) * cDest0 + aSrc * cSrc0) / alphaI);
      cResult1 = (Guchar)(((alphaI - aSrc) * cDest1 + aSrc * cSrc1) / alphaI);
      cResult2 = (Guchar)(((alphaI - aSrc) * cDest2 + aSrc * cSrc2) / alphaI);
    }

    //----- write destination pixel
    destColorPtr[0] = cResult0;
    destColorPtr[1] = cResult1;
    destColorPtr[2] = cResult2;
    destColorPtr += 3;
    *destAlphaPtr++ = aResult;

    cSrcPtr += cSrcStride;
    ++shapePtr;
  }

  updateModX(lastX);
}

// special case:
// !pipe->pattern && !pipe->noTransparency && !state->softMask &&
// pipe->usesShape && !pipe->alpha0Ptr && !state->blendFunc &&
// !pipe->nonIsolatedGroup &&
// bitmap->mode == splashModeBGR8 && bitmap->alpha
void Splash::pipeRunAABGR8(SplashPipe *pipe, int x0, int x1, int y,
			   Guchar *shapePtr, SplashColorPtr cSrcPtr) {
  Guchar shape, aSrc, aDest, alphaI, aResult;
  Guchar cSrc0, cSrc1, cSrc2;
  Guchar cDest0, cDest1, cDest2;
  Guchar cResult0, cResult1, cResult2;
  SplashColorPtr destColorPtr;
  Guchar *destAlphaPtr;
  int cSrcStride, x, lastX;

  if (cSrcPtr) {
    cSrcStride = 3;
  } else {
    cSrcPtr = pipe->cSrcVal;
    cSrcStride = 0;
  }
  for (; x0 <= x1; ++x0) {
    if (*shapePtr) {
      break;
    }
    cSrcPtr += cSrcStride;
    ++shapePtr;
  }
  if (x0 > x1) {
    return;
  }
  updateModX(x0);
  updateModY(y);
  lastX = x0;

  destColorPtr = &bitmap->data[y * bitmap->rowSize + 3 * x0];
  destAlphaPtr = &bitmap->alpha[y * bitmap->alphaRowSize + x0];

  for (x = x0; x <= x1; ++x) {

    //----- shape
    shape = *shapePtr;
    if (!shape) {
      destColorPtr += 3;
      ++destAlphaPtr;
      cSrcPtr += cSrcStride;
      ++shapePtr;
      continue;
    }
    lastX = x;

    //----- read destination pixel
    cDest0 = destColorPtr[2];
    cDest1 = destColorPtr[1];
    cDest2 = destColorPtr[0];
    aDest = *destAlphaPtr;

    //----- source color
    cSrc0 = state->rgbTransferR[cSrcPtr[0]];
    cSrc1 = state->rgbTransferG[cSrcPtr[1]];
    cSrc2 = state->rgbTransferB[cSrcPtr[2]];

    //----- source alpha
    aSrc = div255(pipe->aInput * shape);

    //----- result alpha and non-isolated group element correction
    aResult = (Guchar)(aSrc + aDest - div255(aSrc * aDest));
    alphaI = aResult;

    //----- result color
    if (alphaI == 0) {
      cResult0 = 0;
      cResult1 = 0;
      cResult2 = 0;
    } else {
      cResult0 = (Guchar)(((alphaI - aSrc) * cDest0 + aSrc * cSrc0) / alphaI);
      cResult1 = (Guchar)(((alphaI - aSrc) * cDest1 + aSrc * cSrc1) / alphaI);
      cResult2 = (Guchar)(((alphaI - aSrc) * cDest2 + aSrc * cSrc2) / alphaI);
    }

    //----- write destination pixel
    destColorPtr[0] = cResult2;
    destColorPtr[1] = cResult1;
    destColorPtr[2] = cResult0;
    destColorPtr += 3;
    *destAlphaPtr++ = aResult;

    cSrcPtr += cSrcStride;
    ++shapePtr;
  }

  updateModX(lastX);
}

#if SPLASH_CMYK
// special case:
// !pipe->pattern && !pipe->noTransparency && !state->softMask &&
// pipe->usesShape && !pipe->alpha0Ptr && !state->blendFunc &&
// !pipe->nonIsolatedGroup &&
// bitmap->mode == splashModeCMYK8 && bitmap->alpha
void Splash::pipeRunAACMYK8(SplashPipe *pipe, int x0, int x1, int y,
			    Guchar *shapePtr, SplashColorPtr cSrcPtr) {
  Guchar shape, aSrc, aDest, alphaI, aResult;
  Guchar cSrc0, cSrc1, cSrc2, cSrc3;
  Guchar cDest0, cDest1, cDest2, cDest3;
  Guchar cResult0, cResult1, cResult2, cResult3;
  SplashColorPtr destColorPtr;
  Guchar *destAlphaPtr;
  int cSrcStride, x, lastX;

  if (cSrcPtr) {
    cSrcStride = 4;
  } else {
    cSrcPtr = pipe->cSrcVal;
    cSrcStride = 0;
  }
  for (; x0 <= x1; ++x0) {
    if (*shapePtr) {
      break;
    }
    cSrcPtr += cSrcStride;
    ++shapePtr;
  }
  if (x0 > x1) {
    return;
  }
  updateModX(x0);
  updateModY(y);
  lastX = x0;

  destColorPtr = &bitmap->data[y * bitmap->rowSize + 4 * x0];
  destAlphaPtr = &bitmap->alpha[y * bitmap->alphaRowSize + x0];

  for (x = x0; x <= x1; ++x) {

    //----- shape
    shape = *shapePtr;
    if (!shape) {
      destColorPtr += 4;
      ++destAlphaPtr;
      cSrcPtr += cSrcStride;
      ++shapePtr;
      continue;
    }
    lastX = x;

    //----- read destination pixel
    cDest0 = destColorPtr[0];
    cDest1 = destColorPtr[1];
    cDest2 = destColorPtr[2];
    cDest3 = destColorPtr[3];
    aDest = *destAlphaPtr;

    //----- overprint
    if (state->overprintMask & 1) {
      cSrc0 = state->cmykTransferC[cSrcPtr[0]];
    } else {
      cSrc0 = div255(aDest * cDest0);
    }
    if (state->overprintMask & 2) {
      cSrc1 = state->cmykTransferM[cSrcPtr[1]];
    } else {
      cSrc1 = div255(aDest * cDest1);
    }
    if (state->overprintMask & 4) {
      cSrc2 = state->cmykTransferY[cSrcPtr[2]];
    } else {
      cSrc2 = div255(aDest * cDest2);
    }
    if (state->overprintMask & 8) {
      cSrc3 = state->cmykTransferK[cSrcPtr[3]];
    } else {
      cSrc3 = div255(aDest * cDest3);
    }

    //----- source alpha
    aSrc = div255(pipe->aInput * shape);

    //----- result alpha and non-isolated group element correction
    aResult = (Guchar)(aSrc + aDest - div255(aSrc * aDest));
    alphaI = aResult;

    //----- result color
    if (alphaI == 0) {
      cResult0 = 0;
      cResult1 = 0;
      cResult2 = 0;
      cResult3 = 0;
    } else {
      cResult0 = (Guchar)(((alphaI - aSrc) * cDest0 + aSrc * cSrc0) / alphaI);
      cResult1 = (Guchar)(((alphaI - aSrc) * cDest1 + aSrc * cSrc1) / alphaI);
      cResult2 = (Guchar)(((alphaI - aSrc) * cDest2 + aSrc * cSrc2) / alphaI);
      cResult3 = (Guchar)(((alphaI - aSrc) * cDest3 + aSrc * cSrc3) / alphaI);
    }

    //----- write destination pixel
    destColorPtr[0] = cResult0;
    destColorPtr[1] = cResult1;
    destColorPtr[2] = cResult2;
    destColorPtr[3] = cResult3;
    destColorPtr += 4;
    *destAlphaPtr++ = aResult;

    cSrcPtr += cSrcStride;
    ++shapePtr;
  }

  updateModX(lastX);
}
#endif


//------------------------------------------------------------------------

// Transform a point from user space to device space.
inline void Splash::transform(SplashCoord *matrix,
			      SplashCoord xi, SplashCoord yi,
			      SplashCoord *xo, SplashCoord *yo) {
  //                          [ m[0] m[1] 0 ]
  // [xo yo 1] = [xi yi 1] *  [ m[2] m[3] 0 ]
  //                          [ m[4] m[5] 1 ]
  *xo = xi * matrix[0] + yi * matrix[2] + matrix[4];
  *yo = xi * matrix[1] + yi * matrix[3] + matrix[5];
}

//------------------------------------------------------------------------
// Splash
//------------------------------------------------------------------------

Splash::Splash(SplashBitmap *bitmapA, GBool vectorAntialiasA,
	       SplashScreenParams *screenParams) {
  bitmap = bitmapA;
  bitmapComps = splashColorModeNComps[bitmap->mode];
  vectorAntialias = vectorAntialiasA;
  inShading = gFalse;
  state = new SplashState(bitmap->width, bitmap->height, vectorAntialias,
			  screenParams);
  scanBuf = (Guchar *)gmalloc(bitmap->width);
  if (bitmap->mode == splashModeMono1) {
    scanBuf2 = (Guchar *)gmalloc(bitmap->width);
  } else {
    scanBuf2 = NULL;
  }
  groupBackBitmap = NULL;
  minLineWidth = 0;
  clearModRegion();
  debugMode = gFalse;
}

Splash::Splash(SplashBitmap *bitmapA, GBool vectorAntialiasA,
	       SplashScreen *screenA) {
  bitmap = bitmapA;
  bitmapComps = splashColorModeNComps[bitmap->mode];
  vectorAntialias = vectorAntialiasA;
  inShading = gFalse;
  state = new SplashState(bitmap->width, bitmap->height, vectorAntialias,
			  screenA);
  scanBuf = (Guchar *)gmalloc(bitmap->width);
  if (bitmap->mode == splashModeMono1) {
    scanBuf2 = (Guchar *)gmalloc(bitmap->width);
  } else {
    scanBuf2 = NULL;
  }
  groupBackBitmap = NULL;
  minLineWidth = 0;
  clearModRegion();
  debugMode = gFalse;
}

Splash::~Splash() {
  while (state->next) {
    restoreState();
  }
  delete state;
  gfree(scanBuf);
  gfree(scanBuf2);
}

//------------------------------------------------------------------------
// state read
//------------------------------------------------------------------------

SplashCoord *Splash::getMatrix() {
  return state->matrix;
}

SplashPattern *Splash::getStrokePattern() {
  return state->strokePattern;
}

SplashPattern *Splash::getFillPattern() {
  return state->fillPattern;
}

SplashScreen *Splash::getScreen() {
  return state->screen;
}

SplashBlendFunc Splash::getBlendFunc() {
  return state->blendFunc;
}

SplashCoord Splash::getStrokeAlpha() {
  return state->strokeAlpha;
}

SplashCoord Splash::getFillAlpha() {
  return state->fillAlpha;
}

SplashCoord Splash::getLineWidth() {
  return state->lineWidth;
}

int Splash::getLineCap() {
  return state->lineCap;
}

int Splash::getLineJoin() {
  return state->lineJoin;
}

SplashCoord Splash::getMiterLimit() {
  return state->miterLimit;
}

SplashCoord Splash::getFlatness() {
  return state->flatness;
}

SplashCoord *Splash::getLineDash() {
  return state->lineDash;
}

int Splash::getLineDashLength() {
  return state->lineDashLength;
}

SplashCoord Splash::getLineDashPhase() {
  return state->lineDashPhase;
}

SplashStrokeAdjustMode Splash::getStrokeAdjust() {
  return state->strokeAdjust;
}

SplashClip *Splash::getClip() {
  return state->clip;
}

SplashBitmap *Splash::getSoftMask() {
  return state->softMask;
}

GBool Splash::getInNonIsolatedGroup() {
  return state->inNonIsolatedGroup;
}

GBool Splash::getInKnockoutGroup() {
  return state->inKnockoutGroup;
}

//------------------------------------------------------------------------
// state write
//------------------------------------------------------------------------

void Splash::setMatrix(SplashCoord *matrix) {
  memcpy(state->matrix, matrix, 6 * sizeof(SplashCoord));
}

void Splash::setStrokePattern(SplashPattern *strokePattern) {
  state->setStrokePattern(strokePattern);
}

void Splash::setFillPattern(SplashPattern *fillPattern) {
  state->setFillPattern(fillPattern);
}

void Splash::setScreen(SplashScreen *screen) {
  state->setScreen(screen);
}

void Splash::setBlendFunc(SplashBlendFunc func) {
  state->blendFunc = func;
}

void Splash::setStrokeAlpha(SplashCoord alpha) {
  state->strokeAlpha = alpha;
}

void Splash::setFillAlpha(SplashCoord alpha) {
  state->fillAlpha = alpha;
}

void Splash::setLineWidth(SplashCoord lineWidth) {
  state->lineWidth = lineWidth;
}

void Splash::setLineCap(int lineCap) {
  if (lineCap >= 0 && lineCap <= 2) {
    state->lineCap = lineCap;
  } else {
    state->lineCap = 0;
  }
}

void Splash::setLineJoin(int lineJoin) {
  if (lineJoin >= 0 && lineJoin <= 2) {
    state->lineJoin = lineJoin;
  } else {
    state->lineJoin = 0;
  }
}

void Splash::setMiterLimit(SplashCoord miterLimit) {
  state->miterLimit = miterLimit;
}

void Splash::setFlatness(SplashCoord flatness) {
  if (flatness < 1) {
    state->flatness = 1;
  } else {
    state->flatness = flatness;
  }
}

void Splash::setLineDash(SplashCoord *lineDash, int lineDashLength,
			 SplashCoord lineDashPhase) {
  state->setLineDash(lineDash, lineDashLength, lineDashPhase);
}

void Splash::setStrokeAdjust(SplashStrokeAdjustMode strokeAdjust) {
  state->strokeAdjust = strokeAdjust;
}

void Splash::clipResetToRect(SplashCoord x0, SplashCoord y0,
			     SplashCoord x1, SplashCoord y1) {
  state->clipResetToRect(x0, y0, x1, y1);
}

SplashError Splash::clipToRect(SplashCoord x0, SplashCoord y0,
			       SplashCoord x1, SplashCoord y1) {
  return state->clipToRect(x0, y0, x1, y1);
}

SplashError Splash::clipToPath(SplashPath *path, GBool eo) {
  return state->clipToPath(path, eo);
}

void Splash::setSoftMask(SplashBitmap *softMask) {
  state->setSoftMask(softMask);
}

void Splash::setInTransparencyGroup(SplashBitmap *groupBackBitmapA,
				    int groupBackXA, int groupBackYA,
				    GBool nonIsolated, GBool knockout) {
  groupBackBitmap = groupBackBitmapA;
  groupBackX = groupBackXA;
  groupBackY = groupBackYA;
  state->inNonIsolatedGroup = nonIsolated;
  state->inKnockoutGroup = knockout;
}

void Splash::setTransfer(Guchar *red, Guchar *green, Guchar *blue,
			 Guchar *gray) {
  state->setTransfer(red, green, blue, gray);
}

void Splash::setOverprintMask(Guint overprintMask) {
  state->overprintMask = overprintMask;
}


void Splash::setEnablePathSimplification(GBool en) {
  state->enablePathSimplification = en;
}

//------------------------------------------------------------------------
// state save/restore
//------------------------------------------------------------------------

void Splash::saveState() {
  SplashState *newState;

  newState = state->copy();
  newState->next = state;
  state = newState;
}

SplashError Splash::restoreState() {
  SplashState *oldState;

  if (!state->next) {
    return splashErrNoSave;
  }
  oldState = state;
  state = state->next;
  delete oldState;
  return splashOk;
}

//------------------------------------------------------------------------
// drawing operations
//------------------------------------------------------------------------

void Splash::clear(SplashColorPtr color, Guchar alpha) {
  SplashColorPtr row, p;
  Guchar mono;
  int x, y;

  switch (bitmap->mode) {
  case splashModeMono1:
    mono = (color[0] & 0x80) ? 0xff : 0x00;
    if (bitmap->rowSize < 0) {
      memset(bitmap->data + bitmap->rowSize * (bitmap->height - 1),
	     mono, -bitmap->rowSize * bitmap->height);
    } else {
      memset(bitmap->data, mono, bitmap->rowSize * bitmap->height);
    }
    break;
  case splashModeMono8:
    if (bitmap->rowSize < 0) {
      memset(bitmap->data + bitmap->rowSize * (bitmap->height - 1),
	     color[0], -bitmap->rowSize * bitmap->height);
    } else {
      memset(bitmap->data, color[0], bitmap->rowSize * bitmap->height);
    }
    break;
  case splashModeRGB8:
    if (color[0] == color[1] && color[1] == color[2]) {
      if (bitmap->rowSize < 0) {
	memset(bitmap->data + bitmap->rowSize * (bitmap->height - 1),
	       color[0], -bitmap->rowSize * bitmap->height);
      } else {
	memset(bitmap->data, color[0], bitmap->rowSize * bitmap->height);
      }
    } else {
      row = bitmap->data;
      for (y = 0; y < bitmap->height; ++y) {
	p = row;
	for (x = 0; x < bitmap->width; ++x) {
	  *p++ = color[0];
	  *p++ = color[1];
	  *p++ = color[2];
	}
	row += bitmap->rowSize;
      }
    }
    break;
  case splashModeBGR8:
    if (color[0] == color[1] && color[1] == color[2]) {
      if (bitmap->rowSize < 0) {
	memset(bitmap->data + bitmap->rowSize * (bitmap->height - 1),
	       color[0], -bitmap->rowSize * bitmap->height);
      } else {
	memset(bitmap->data, color[0], bitmap->rowSize * bitmap->height);
      }
    } else {
      row = bitmap->data;
      for (y = 0; y < bitmap->height; ++y) {
	p = row;
	for (x = 0; x < bitmap->width; ++x) {
	  *p++ = color[2];
	  *p++ = color[1];
	  *p++ = color[0];
	}
	row += bitmap->rowSize;
      }
    }
    break;
#if SPLASH_CMYK
  case splashModeCMYK8:
    if (color[0] == color[1] && color[1] == color[2] && color[2] == color[3]) {
      if (bitmap->rowSize < 0) {
	memset(bitmap->data + bitmap->rowSize * (bitmap->height - 1),
	       color[0], -bitmap->rowSize * bitmap->height);
      } else {
	memset(bitmap->data, color[0], bitmap->rowSize * bitmap->height);
      }
    } else {
      row = bitmap->data;
      for (y = 0; y < bitmap->height; ++y) {
	p = row;
	for (x = 0; x < bitmap->width; ++x) {
	  *p++ = color[0];
	  *p++ = color[1];
	  *p++ = color[2];
	  *p++ = color[3];
	}
	row += bitmap->rowSize;
      }
    }
    break;
#endif
  }

  if (bitmap->alpha) {
    memset(bitmap->alpha, alpha, bitmap->alphaRowSize * bitmap->height);
  }

  updateModX(0);
  updateModY(0);
  updateModX(bitmap->width - 1);
  updateModY(bitmap->height - 1);
}

SplashError Splash::stroke(SplashPath *path) {
  SplashPath *path2, *dPath;
  SplashCoord t0, t1, t2, t3, w, w2, lineDashMax, lineDashTotal;
  int lineCap, lineJoin, i;

  if (debugMode) {
    printf("stroke [dash:%d] [width:%.2f]:\n",
	   state->lineDashLength, (double)state->lineWidth);
    dumpPath(path);
  }
  opClipRes = splashClipAllOutside;
  if (path->length == 0) {
    return splashErrEmptyPath;
  }
  path2 = flattenPath(path, state->matrix, state->flatness);

  // Compute an approximation of the transformed line width.
  // Given a CTM of [m0 m1],
  //                [m2 m3]
  // if |m0|*|m3| >= |m1|*|m2| then use min{|m0|,|m3|}, else
  // use min{|m1|,|m2|}.
  // This handles the common cases -- [s 0   ] and [0    s] --
  //                                  [0 +/-s]     [+/-s 0]
  // well, and still does something reasonable for the uncommon
  // case transforms.
  t0 = splashAbs(state->matrix[0]);
  t1 = splashAbs(state->matrix[1]);
  t2 = splashAbs(state->matrix[2]);
  t3 = splashAbs(state->matrix[3]);
  if (t0 * t3 >= t1 * t2) {
    w = (t0 < t3) ? t0 : t3;
  } else {
    w = (t1 < t2) ? t1 : t2;
  }
  w2 = w * state->lineWidth;

  // construct the dashed path
  if (state->lineDashLength > 0) {

    // check the maximum transformed dash element length (using the
    // same approximation as for line width) -- if it's less than 0.1
    // pixel, don't apply the dash pattern; this avoids a huge
    // performance/memory hit with PDF files that use absurd dash
    // patterns like [0.0007 0.0003]
    lineDashTotal = 0;
    lineDashMax = 0;
    for (i = 0; i < state->lineDashLength; ++i) {
      lineDashTotal += state->lineDash[i];
      if (state->lineDash[i] > lineDashMax) {
	lineDashMax = state->lineDash[i];
      }
    }
    // Acrobat simply draws nothing if the dash array is [0]
    if (lineDashTotal == 0) {
      delete path2;
      return splashOk;
    }
    if (w * lineDashMax > 0.1) {

      dPath = makeDashedPath(path2);
      delete path2;
      path2 = dPath;
      if (path2->length == 0) {
	delete path2;
	return splashErrEmptyPath;
      }
    }
  }

  // round caps on narrow lines look bad, and can't be
  // stroke-adjusted, so use projecting caps instead (but we can't do
  // this if there are zero-length dashes or segments, because those
  // turn into round dots)
  lineCap = state->lineCap;
  lineJoin = state->lineJoin;
  if (state->strokeAdjust == splashStrokeAdjustCAD &&
      w2 < 3.5) {
    if (lineCap == splashLineCapRound &&
	!state->lineDashContainsZeroLengthDashes() &&
	!path->containsZeroLengthSubpaths()) {
      lineCap = splashLineCapProjecting;
    }
    if (lineJoin == splashLineJoinRound) {
      lineJoin = splashLineJoinBevel;
    }
  }

  // if there is a min line width set, and the transformed line width
  // is smaller, use the min line width
  if (w > 0 && w2 < minLineWidth) {
    strokeWide(path2, minLineWidth / w, splashLineCapButt, splashLineJoinBevel);
  } else if (bitmap->mode == splashModeMono1 || !vectorAntialias) {
    // in monochrome mode or if antialiasing is disabled, use 0-width
    // lines for any transformed line width <= 1 -- lines less than 1
    // pixel wide look too fat without antialiasing
    if (w2 < 1.001) {
      strokeNarrow(path2);
    } else {
      strokeWide(path2, state->lineWidth, lineCap, lineJoin);
    }
  } else {
    // in gray and color modes, only use 0-width lines if the line
    // width is explicitly set to 0
    if (state->lineWidth == 0) {
      strokeNarrow(path2);
    } else {
      strokeWide(path2, state->lineWidth, lineCap, lineJoin);
    }
  }

  delete path2;
  return splashOk;
}

void Splash::strokeNarrow(SplashPath *path) {
  SplashPipe pipe;
  SplashXPath *xPath;
  SplashXPathSeg *seg;
  int x0, x1, y0, y1, xa, xb, y;
  SplashCoord dxdy;
  SplashClipResult clipRes;
  int nClipRes[3];
  int i;

  nClipRes[0] = nClipRes[1] = nClipRes[2] = 0;

  xPath = new SplashXPath(path, state->matrix, state->flatness, gFalse,
			  state->enablePathSimplification,
			  state->strokeAdjust);

  pipeInit(&pipe, state->strokePattern,
	   (Guchar)splashRound(state->strokeAlpha * 255),
	   gTrue, gFalse);

  for (i = 0, seg = xPath->segs; i < xPath->length; ++i, ++seg) {
    if (seg->y0 <= seg->y1) {
      y0 = splashFloor(seg->y0);
      y1 = splashFloor(seg->y1);
      x0 = splashFloor(seg->x0);
      x1 = splashFloor(seg->x1);
    } else {
      y0 = splashFloor(seg->y1);
      y1 = splashFloor(seg->y0);
      x0 = splashFloor(seg->x1);
      x1 = splashFloor(seg->x0);
    }
    if ((clipRes = state->clip->testRect(x0 <= x1 ? x0 : x1, y0,
					 x0 <= x1 ? x1 : x0, y1,
					 state->strokeAdjust))
	!= splashClipAllOutside) {
      if (y0 == y1) {
	if (x0 <= x1) {
	  drawStrokeSpan(&pipe, x0, x1, y0, clipRes == splashClipAllInside);
	} else {
	  drawStrokeSpan(&pipe, x1, x0, y0, clipRes == splashClipAllInside);
	}
      } else {
	dxdy = seg->dxdy;
	y = state->clip->getYMinI(state->strokeAdjust);
	if (y0 < y) {
	  y0 = y;
	  x0 = splashFloor(seg->x0 + ((SplashCoord)y0 - seg->y0) * dxdy);
	}
	y = state->clip->getYMaxI(state->strokeAdjust);
	if (y1 > y) {
	  y1 = y;
	  x1 = splashFloor(seg->x0 + ((SplashCoord)y1 - seg->y0) * dxdy);
	}
	if (x0 <= x1) {
	  xa = x0;
	  for (y = y0; y <= y1; ++y) {
	    if (y < y1) {
	      xb = splashFloor(seg->x0 +
			       ((SplashCoord)y + 1 - seg->y0) * dxdy);
	    } else {
	      xb = x1 + 1;
	    }
	    if (xa == xb) {
	      drawStrokeSpan(&pipe, xa, xa, y, clipRes == splashClipAllInside);
	    } else {
	      drawStrokeSpan(&pipe, xa, xb - 1, y,
			     clipRes == splashClipAllInside);
	    }
	    xa = xb;
	  }
	} else {
	  xa = x0;
	  for (y = y0; y <= y1; ++y) {
	    if (y < y1) {
	      xb = splashFloor(seg->x0 +
			       ((SplashCoord)y + 1 - seg->y0) * dxdy);
	    } else {
	      xb = x1 - 1;
	    }
	    if (xa == xb) {
	      drawStrokeSpan(&pipe, xa, xa, y, clipRes == splashClipAllInside);
	    } else {
	      drawStrokeSpan(&pipe, xb + 1, xa, y,
			     clipRes == splashClipAllInside);
	    }
	    xa = xb;
	  }
	}
      }
    }
    ++nClipRes[clipRes];
  }
  if (nClipRes[splashClipPartial] ||
      (nClipRes[splashClipAllInside] && nClipRes[splashClipAllOutside])) {
    opClipRes = splashClipPartial;
  } else if (nClipRes[splashClipAllInside]) {
    opClipRes = splashClipAllInside;
  } else {
    opClipRes = splashClipAllOutside;
  }

  delete xPath;
}

void Splash::drawStrokeSpan(SplashPipe *pipe, int x0, int x1, int y,
			    GBool noClip) {
  int x;

  x = state->clip->getXMinI(state->strokeAdjust);
  if (x > x0) {
    x0 = x;
  }
  x = state->clip->getXMaxI(state->strokeAdjust);
  if (x < x1) {
    x1 = x;
  }
  if (x0 > x1) {
    return;
  }
  for (x = x0; x <= x1; ++x) {
    scanBuf[x] = 0xff;
  }
  if (!noClip) {
    if (!state->clip->clipSpanBinary(scanBuf, y, x0, x1, state->strokeAdjust)) {
      return;
    }
  }
  (this->*pipe->run)(pipe, x0, x1, y, scanBuf + x0, NULL);
}

void Splash::strokeWide(SplashPath *path, SplashCoord w,
			int lineCap, int lineJoin) {
  SplashPath *path2;

  path2 = makeStrokePath(path, w, lineCap, lineJoin, gFalse);
  fillWithPattern(path2, gFalse, state->strokePattern, state->strokeAlpha);
  delete path2;
}

SplashPath *Splash::flattenPath(SplashPath *path, SplashCoord *matrix,
				SplashCoord flatness) {
  SplashPath *fPath;
  SplashCoord flatness2;
  Guchar flag;
  int i;

  fPath = new SplashPath();
#if USE_FIXEDPOINT
  flatness2 = flatness;
#else
  flatness2 = flatness * flatness;
#endif
  i = 0;
  while (i < path->length) {
    flag = path->flags[i];
    if (flag & splashPathFirst) {
      fPath->moveTo(path->pts[i].x, path->pts[i].y);
      ++i;
    } else {
      if (flag & splashPathCurve) {
	flattenCurve(path->pts[i-1].x, path->pts[i-1].y,
		     path->pts[i  ].x, path->pts[i  ].y,
		     path->pts[i+1].x, path->pts[i+1].y,
		     path->pts[i+2].x, path->pts[i+2].y,
		     matrix, flatness2, fPath);
	i += 3;
      } else {
	fPath->lineTo(path->pts[i].x, path->pts[i].y);
	++i;
      }
      if (path->flags[i-1] & splashPathClosed) {
	fPath->close();
      }
    }
  }
  return fPath;
}

void Splash::flattenCurve(SplashCoord x0, SplashCoord y0,
			  SplashCoord x1, SplashCoord y1,
			  SplashCoord x2, SplashCoord y2,
			  SplashCoord x3, SplashCoord y3,
			  SplashCoord *matrix, SplashCoord flatness2,
			  SplashPath *fPath) {
  SplashCoord cx[splashMaxCurveSplits + 1][3];
  SplashCoord cy[splashMaxCurveSplits + 1][3];
  int cNext[splashMaxCurveSplits + 1];
  SplashCoord xl0, xl1, xl2, xr0, xr1, xr2, xr3, xx1, xx2, xh;
  SplashCoord yl0, yl1, yl2, yr0, yr1, yr2, yr3, yy1, yy2, yh;
  SplashCoord dx, dy, mx, my, tx, ty, d1, d2;
  int p1, p2, p3;

  // initial segment
  p1 = 0;
  p2 = splashMaxCurveSplits;
  cx[p1][0] = x0;  cy[p1][0] = y0;
  cx[p1][1] = x1;  cy[p1][1] = y1;
  cx[p1][2] = x2;  cy[p1][2] = y2;
  cx[p2][0] = x3;  cy[p2][0] = y3;
  cNext[p1] = p2;

  while (p1 < splashMaxCurveSplits) {

    // get the next segment
    xl0 = cx[p1][0];  yl0 = cy[p1][0];
    xx1 = cx[p1][1];  yy1 = cy[p1][1];
    xx2 = cx[p1][2];  yy2 = cy[p1][2];
    p2 = cNext[p1];
    xr3 = cx[p2][0];  yr3 = cy[p2][0];

    // compute the distances (in device space) from the control points
    // to the midpoint of the straight line (this is a bit of a hack,
    // but it's much faster than computing the actual distances to the
    // line)
    transform(matrix, (xl0 + xr3) * 0.5, (yl0 + yr3) * 0.5, &mx, &my);
    transform(matrix, xx1, yy1, &tx, &ty);
#if USE_FIXEDPOINT
    d1 = splashDist(tx, ty, mx, my);
#else
    dx = tx - mx;
    dy = ty - my;
    d1 = dx*dx + dy*dy;
#endif
    transform(matrix, xx2, yy2, &tx, &ty);
#if USE_FIXEDPOINT
    d2 = splashDist(tx, ty, mx, my);
#else
    dx = tx - mx;
    dy = ty - my;
    d2 = dx*dx + dy*dy;
#endif

    // if the curve is flat enough, or no more subdivisions are
    // allowed, add the straight line segment
    if (p2 - p1 == 1 || (d1 <= flatness2 && d2 <= flatness2)) {
      fPath->lineTo(xr3, yr3);
      p1 = p2;

    // otherwise, subdivide the curve
    } else {
      xl1 = splashAvg(xl0, xx1);
      yl1 = splashAvg(yl0, yy1);
      xh = splashAvg(xx1, xx2);
      yh = splashAvg(yy1, yy2);
      xl2 = splashAvg(xl1, xh);
      yl2 = splashAvg(yl1, yh);
      xr2 = splashAvg(xx2, xr3);
      yr2 = splashAvg(yy2, yr3);
      xr1 = splashAvg(xh, xr2);
      yr1 = splashAvg(yh, yr2);
      xr0 = splashAvg(xl2, xr1);
      yr0 = splashAvg(yl2, yr1);
      // add the new subdivision points
      p3 = (p1 + p2) / 2;
      cx[p1][1] = xl1;  cy[p1][1] = yl1;
      cx[p1][2] = xl2;  cy[p1][2] = yl2;
      cNext[p1] = p3;
      cx[p3][0] = xr0;  cy[p3][0] = yr0;
      cx[p3][1] = xr1;  cy[p3][1] = yr1;
      cx[p3][2] = xr2;  cy[p3][2] = yr2;
      cNext[p3] = p2;
    }
  }
}

SplashPath *Splash::makeDashedPath(SplashPath *path) {
  SplashPath *dPath;
  SplashCoord lineDashTotal;
  SplashCoord lineDashStartPhase, lineDashDist, segLen;
  SplashCoord x0, y0, x1, y1, xa, ya;
  GBool lineDashStartOn, lineDashEndOn, lineDashOn, newPath;
  int lineDashStartIdx, lineDashIdx, subpathStart, nDashes;
  int i, j, k;

  lineDashTotal = 0;
  for (i = 0; i < state->lineDashLength; ++i) {
    lineDashTotal += state->lineDash[i];
  }
  // Acrobat simply draws nothing if the dash array is [0]
  if (lineDashTotal == 0) {
    return new SplashPath();
  }
  lineDashStartPhase = state->lineDashPhase;
  if (lineDashStartPhase > lineDashTotal * 2) {
    i = splashFloor(lineDashStartPhase / (lineDashTotal * 2));
    lineDashStartPhase -= lineDashTotal * i * 2;
  } else if (lineDashStartPhase < 0) {
    i = splashCeil(-lineDashStartPhase / (lineDashTotal * 2));
    lineDashStartPhase += lineDashTotal * i * 2;
  }
  i = splashFloor(lineDashStartPhase / lineDashTotal);
  lineDashStartPhase -= (SplashCoord)i * lineDashTotal;
  lineDashStartOn = gTrue;
  lineDashStartIdx = 0;
  if (lineDashStartPhase > 0) {
    while (lineDashStartPhase >= state->lineDash[lineDashStartIdx]) {
      lineDashStartOn = !lineDashStartOn;
      lineDashStartPhase -= state->lineDash[lineDashStartIdx];
      if (++lineDashStartIdx == state->lineDashLength) {
	lineDashStartIdx = 0;
      }
    }
  }

  dPath = new SplashPath();

  // process each subpath
  i = 0;
  while (i < path->length) {

    // find the end of the subpath
    for (j = i;
	 j < path->length - 1 && !(path->flags[j] & splashPathLast);
	 ++j) ;

    // initialize the dash parameters
    lineDashOn = lineDashStartOn;
    lineDashEndOn = lineDashStartOn;
    lineDashIdx = lineDashStartIdx;
    lineDashDist = state->lineDash[lineDashIdx] - lineDashStartPhase;
    subpathStart = dPath->length;
    nDashes = 0;

    // process each segment of the subpath
    newPath = gTrue;
    for (k = i; k < j; ++k) {

      // grab the segment
      x0 = path->pts[k].x;
      y0 = path->pts[k].y;
      x1 = path->pts[k+1].x;
      y1 = path->pts[k+1].y;
      segLen = splashDist(x0, y0, x1, y1);

      // process the segment
      while (segLen > 0) {

	// Special case for zero-length dash segments: draw a very
	// short -- but not zero-length -- segment.  This ensures that
	// we get the correct behavior with butt and projecting line
	// caps.  The PS/PDF specs imply that zero-length segments are
	// not drawn unless the line cap is round, but Acrobat and
	// Ghostscript both draw very short segments (for butt caps)
	// and squares (for projecting caps).
	if (lineDashDist == 0) {
	  if (lineDashOn) {
	    if (newPath) {
	      dPath->moveTo(x0, y0);
	      newPath = gFalse;
	      ++nDashes;
	    }
	    xa = x0 + ((SplashCoord)0.001 / segLen) * (x1 - x0);
	    ya = y0 + ((SplashCoord)0.001 / segLen) * (y1 - y0);
	    dPath->lineTo(xa, ya);
	  }

	} else if (lineDashDist >= segLen) {
	  if (lineDashOn) {
	    if (newPath) {
	      dPath->moveTo(x0, y0);
	      newPath = gFalse;
	      ++nDashes;
	    }
	    dPath->lineTo(x1, y1);
	  }
	  lineDashDist -= segLen;
	  segLen = 0;

	} else {
	  xa = x0 + (lineDashDist / segLen) * (x1 - x0);
	  ya = y0 + (lineDashDist / segLen) * (y1 - y0);
	  if (lineDashOn) {
	    if (newPath) {
	      dPath->moveTo(x0, y0);
	      newPath = gFalse;
	      ++nDashes;
	    }
	    dPath->lineTo(xa, ya);
	  }
	  x0 = xa;
	  y0 = ya;
	  segLen -= lineDashDist;
	  lineDashDist = 0;
	}

	lineDashEndOn = lineDashOn;

	// get the next entry in the dash array
	if (lineDashDist <= 0) {
	  lineDashOn = !lineDashOn;
	  if (++lineDashIdx == state->lineDashLength) {
	    lineDashIdx = 0;
	  }
	  lineDashDist = state->lineDash[lineDashIdx];
	  newPath = gTrue;
	}
      }
    }

    // in a closed subpath, where the dash pattern is "on" at both the
    // start and end of the subpath, we need to merge the start and
    // end to get a proper line join
    if ((path->flags[j] & splashPathClosed) &&
	lineDashStartOn &&
	lineDashEndOn) {
      if (nDashes == 1) {
	dPath->close();
      } else if (nDashes > 1) {
	k = subpathStart;
	do {
	  ++k;
	  dPath->lineTo(dPath->pts[k].x, dPath->pts[k].y);
	} while (!(dPath->flags[k] & splashPathLast));
	++k;
	memmove(&dPath->pts[subpathStart], &dPath->pts[k],
		(dPath->length - k) * sizeof(SplashPathPoint));
	memmove(&dPath->flags[subpathStart], &dPath->flags[k],
		(dPath->length - k) * sizeof(Guchar));
	dPath->length -= k - subpathStart;
	dPath->curSubpath -= k - subpathStart;
      }
    }

    i = j + 1;
  }

  return dPath;
}

SplashError Splash::fill(SplashPath *path, GBool eo) {
  if (debugMode) {
    printf("fill [eo:%d]:\n", eo);
    dumpPath(path);
  }
  return fillWithPattern(path, eo, state->fillPattern, state->fillAlpha);
}

SplashError Splash::fillWithPattern(SplashPath *path, GBool eo,
				    SplashPattern *pattern,
				    SplashCoord alpha) {
  SplashPipe pipe;
  SplashPath *path2;
  SplashXPath *xPath;
  SplashXPathScanner *scanner;
  int xMin, yMin, xMax, xMin2, xMax2, yMax, y, t;
  SplashClipResult clipRes;

  if (path->length == 0) {
    return splashErrEmptyPath;
  }
  if (pathAllOutside(path)) {
    opClipRes = splashClipAllOutside;
    return splashOk;
  }

  path2 = tweakFillPath(path);

  xPath = new SplashXPath(path2, state->matrix, state->flatness, gTrue,
			  state->enablePathSimplification,
			  state->strokeAdjust);
  if (path2 != path) {
    delete path2;
  }
  xMin = xPath->getXMin();
  yMin = xPath->getYMin();
  xMax = xPath->getXMax();
  yMax = xPath->getYMax();
  if (xMin > xMax || yMin > yMax) {
    delete xPath;
    return splashOk;
  }
  scanner = new SplashXPathScanner(xPath, eo, yMin, yMax);

  // check clipping
  if ((clipRes = state->clip->testRect(xMin, yMin, xMax, yMax,
				       state->strokeAdjust))
      != splashClipAllOutside) {

    if ((t = state->clip->getXMinI(state->strokeAdjust)) > xMin) {
      xMin = t;
    }
    if ((t = state->clip->getXMaxI(state->strokeAdjust)) < xMax) {
      xMax = t;
    }
    if ((t = state->clip->getYMinI(state->strokeAdjust)) > yMin) {
      yMin = t;
    }
    if ((t = state->clip->getYMaxI(state->strokeAdjust)) < yMax) {
      yMax = t;
    }
    if (xMin > xMax || yMin > yMax) {
      delete scanner;
      delete xPath;
      return splashOk;
    }

    pipeInit(&pipe, pattern, (Guchar)splashRound(alpha * 255),
	     gTrue, gFalse);

    // draw the spans
    if (vectorAntialias && !inShading) {
      for (y = yMin; y <= yMax; ++y) {
	scanner->getSpan(scanBuf, y, xMin, xMax, &xMin2, &xMax2);
	if (xMin2 <= xMax2) {
	  if (clipRes != splashClipAllInside) {
	    state->clip->clipSpan(scanBuf, y, xMin2, xMax2,
				  state->strokeAdjust);
	  }
	  (this->*pipe.run)(&pipe, xMin2, xMax2, y, scanBuf + xMin2, NULL);
	}
      }
    } else {
      for (y = yMin; y <= yMax; ++y) {
	scanner->getSpanBinary(scanBuf, y, xMin, xMax, &xMin2, &xMax2);
	if (xMin2 <= xMax2) {
	  if (clipRes != splashClipAllInside) {
	    state->clip->clipSpanBinary(scanBuf, y, xMin2, xMax2,
					state->strokeAdjust);
	  }
	  (this->*pipe.run)(&pipe, xMin2, xMax2, y, scanBuf + xMin2, NULL);
	}
      }
    }
  }
  opClipRes = clipRes;

  delete scanner;
  delete xPath;
  return splashOk;
}

// Applies various tweaks to a fill path:
// (1) add stroke adjust hints to a filled rectangle
// (2) applies a minimum width to a zero-width filled rectangle (so
//     stroke adjustment works correctly
// (3) convert a degenerate fill ('moveto lineto fill' and 'moveto
//     lineto closepath fill') to a minimum-width filled rectangle
//
// These tweaks only apply to paths with a single subpath.
//
// Returns either the unchanged input path or a new path (in which
// case the returned path must be deleted by the caller).
SplashPath *Splash::tweakFillPath(SplashPath *path) {
  SplashPath *path2;
  SplashCoord xx0, yy0, xx1, yy1, dx, dy, d, wx, wy, w;
  int n;

  if (state->strokeAdjust == splashStrokeAdjustOff || path->hints) {
    return path;
  }

  n = path->getLength();
  if (!((n == 2) ||
	(n == 3 &&
	 path->flags[1] == 0) ||
	(n == 4 &&
	 path->flags[1] == 0 &&
	 path->flags[2] == 0) ||
	(n == 5 &&
	 path->flags[1] == 0 &&
	 path->flags[2] == 0 &&
	 path->flags[3] == 0))) {
    return path;
  }

  path2 = path;

  // degenerate fill (2 or 3 points) or rectangle of (nearly) zero
  // width --> replace with a min-width rectangle and hint
  if (n == 2 ||
      (n == 3 && (path->flags[0] & splashPathClosed)) ||
      (n == 3 && (splashAbs(path->pts[0].x - path->pts[2].x) < 0.001 &&
		  splashAbs(path->pts[0].y - path->pts[2].y) < 0.001)) ||
      ((n == 4 ||
	(n == 5 && (path->flags[0] & splashPathClosed))) &&
       ((splashAbs(path->pts[0].x - path->pts[1].x) < 0.001 &&
	 splashAbs(path->pts[0].y - path->pts[1].y) < 0.001 &&
	 splashAbs(path->pts[2].x - path->pts[3].x) < 0.001 &&
	 splashAbs(path->pts[2].y - path->pts[3].y) < 0.001) ||
	(splashAbs(path->pts[0].x - path->pts[3].x) < 0.001 &&
	 splashAbs(path->pts[0].y - path->pts[3].y) < 0.001 &&
	 splashAbs(path->pts[1].x - path->pts[2].x) < 0.001 &&
	 splashAbs(path->pts[1].y - path->pts[2].y) < 0.001)))) {
    wx = state->matrix[0] + state->matrix[2];
    wy = state->matrix[1] + state->matrix[3];
    w = splashSqrt(wx*wx + wy*wy);
    if (w < 0.001) {
      w = 0;
    } else {
      // min width is 0.1 -- this constant is minWidth * sqrt(2)
      w = (SplashCoord)0.1414 / w;
    }
    xx0 = path->pts[0].x;
    yy0 = path->pts[0].y;
    if (n <= 3) {
      xx1 = path->pts[1].x;
      yy1 = path->pts[1].y;
    } else {
      xx1 = path->pts[2].x;
      yy1 = path->pts[2].y;
    }
    dx = xx1 - xx0;
    dy = yy1 - yy0;
    d = splashSqrt(dx * dx + dy * dy);
    if (d < 0.001) {
      d = 0;
    } else {
      d = w / d;
    }
    dx *= d;
    dy *= d;
    path2 = new SplashPath();
    path2->moveTo(xx0 + dy, yy0 - dx);
    path2->lineTo(xx1 + dy, yy1 - dx);
    path2->lineTo(xx1 - dy, yy1 + dx);
    path2->lineTo(xx0 - dy, yy0 + dx);
    path2->close(gTrue);
    path2->addStrokeAdjustHint(0, 2, 0, 4);
    path2->addStrokeAdjustHint(1, 3, 0, 4);

  // unclosed rectangle --> close and hint
  } else if (n == 4 && !(path->flags[0] & splashPathClosed)) {
    path2->close(gTrue);
    path2->addStrokeAdjustHint(0, 2, 0, 4);
    path2->addStrokeAdjustHint(1, 3, 0, 4);

  // closed rectangle --> hint
  } else if (n == 5 && (path->flags[0] & splashPathClosed)) {
    path2->addStrokeAdjustHint(0, 2, 0, 4);
    path2->addStrokeAdjustHint(1, 3, 0, 4);
  }

  return path2;
}

GBool Splash::pathAllOutside(SplashPath *path) {
  SplashCoord xMin1, yMin1, xMax1, yMax1;
  SplashCoord xMin2, yMin2, xMax2, yMax2;
  SplashCoord x, y;
  int xMinI, yMinI, xMaxI, yMaxI;
  int i;

  xMin1 = xMax1 = path->pts[0].x;
  yMin1 = yMax1 = path->pts[0].y;
  for (i = 1; i < path->length; ++i) {
    if (path->pts[i].x < xMin1) {
      xMin1 = path->pts[i].x;
    } else if (path->pts[i].x > xMax1) {
      xMax1 = path->pts[i].x;
    }
    if (path->pts[i].y < yMin1) {
      yMin1 = path->pts[i].y;
    } else if (path->pts[i].y > yMax1) {
      yMax1 = path->pts[i].y;
    }
  }

  transform(state->matrix, xMin1, yMin1, &x, &y);
  xMin2 = xMax2 = x;
  yMin2 = yMax2 = y;
  transform(state->matrix, xMin1, yMax1, &x, &y);
  if (x < xMin2) {
    xMin2 = x;
  } else if (x > xMax2) {
    xMax2 = x;
  }
  if (y < yMin2) {
    yMin2 = y;
  } else if (y > yMax2) {
    yMax2 = y;
  }
  transform(state->matrix, xMax1, yMin1, &x, &y);
  if (x < xMin2) {
    xMin2 = x;
  } else if (x > xMax2) {
    xMax2 = x;
  }
  if (y < yMin2) {
    yMin2 = y;
  } else if (y > yMax2) {
    yMax2 = y;
  }
  transform(state->matrix, xMax1, yMax1, &x, &y);
  if (x < xMin2) {
    xMin2 = x;
  } else if (x > xMax2) {
    xMax2 = x;
  }
  if (y < yMin2) {
    yMin2 = y;
  } else if (y > yMax2) {
    yMax2 = y;
  }
  // sanity-check the coordinates - xMinI/yMinI/xMaxI/yMaxI are
  // 32-bit integers, so coords need to be < 2^31
  SplashXPath::clampCoords(&xMin2, &yMin2);
  SplashXPath::clampCoords(&xMax2, &yMax2);
  xMinI = splashFloor(xMin2);
  yMinI = splashFloor(yMin2);
  xMaxI = splashFloor(xMax2);
  yMaxI = splashFloor(yMax2);

  return state->clip->testRect(xMinI, yMinI, xMaxI, yMaxI,
			       state->strokeAdjust) ==
         splashClipAllOutside;
}

SplashError Splash::fillChar(SplashCoord x, SplashCoord y,
			     int c, SplashFont *font) {
  SplashGlyphBitmap glyph;
  SplashCoord xt, yt;
  int x0, y0, xFrac, yFrac;
  SplashError err;

  if (debugMode) {
    printf("fillChar: x=%.2f y=%.2f c=%3d=0x%02x='%c'\n",
	   (double)x, (double)y, c, c, c);
  }
  transform(state->matrix, x, y, &xt, &yt);
  x0 = splashFloor(xt);
  xFrac = splashFloor((xt - x0) * splashFontFraction);
  y0 = splashFloor(yt);
  yFrac = splashFloor((yt - y0) * splashFontFraction);
  if (!font->getGlyph(c, xFrac, yFrac, &glyph)) {
    return splashErrNoGlyph;
  }
  err = fillGlyph2(x0, y0, &glyph);
  if (glyph.freeData) {
    gfree(glyph.data);
  }
  return err;
}

SplashError Splash::fillGlyph(SplashCoord x, SplashCoord y,
			      SplashGlyphBitmap *glyph) {
  SplashCoord xt, yt;
  int x0, y0;

  transform(state->matrix, x, y, &xt, &yt);
  x0 = splashFloor(xt);
  y0 = splashFloor(yt);
  return fillGlyph2(x0, y0, glyph);
}

SplashError Splash::fillGlyph2(int x0, int y0, SplashGlyphBitmap *glyph) {
  SplashPipe pipe;
  SplashClipResult clipRes;
  Guchar alpha;
  Guchar *p;
  int xMin, yMin, xMax, yMax;
  int x, y, xg, yg, xx, t;

  xg = x0 - glyph->x;
  yg = y0 - glyph->y;
  xMin = xg;
  xMax = xg + glyph->w - 1;
  yMin = yg;
  yMax = yg + glyph->h - 1;
  if ((clipRes = state->clip->testRect(xMin, yMin, xMax, yMax,
				       state->strokeAdjust))
      != splashClipAllOutside) {
    pipeInit(&pipe, state->fillPattern,
	     (Guchar)splashRound(state->fillAlpha * 255),
	     gTrue, gFalse);
    if (clipRes == splashClipAllInside) {
      if (glyph->aa) {
	p = glyph->data;
	for (y = yMin; y <= yMax; ++y) {
	  (this->*pipe.run)(&pipe, xMin, xMax, y,
			    glyph->data + (y - yMin) * glyph->w, NULL);
	}
      } else {
	p = glyph->data;
	for (y = yMin; y <= yMax; ++y) {
	  for (x = xMin; x <= xMax; x += 8) {
	    alpha = *p++;
	    for (xx = 0; xx < 8 && x + xx <= xMax; ++xx) {
	      scanBuf[x + xx] = (alpha & 0x80) ? 0xff : 0x00;
	      alpha = (Guchar)(alpha << 1);
	    }
	  }
	  (this->*pipe.run)(&pipe, xMin, xMax, y, scanBuf + xMin, NULL);
	}
      }
    } else {
      if ((t = state->clip->getXMinI(state->strokeAdjust)) > xMin) {
	xMin = t;
      }
      if ((t = state->clip->getXMaxI(state->strokeAdjust)) < xMax) {
	xMax = t;
      }
      if ((t = state->clip->getYMinI(state->strokeAdjust)) > yMin) {
	yMin = t;
      }
      if ((t = state->clip->getYMaxI(state->strokeAdjust)) < yMax) {
	yMax = t;
      }
      if (xMin <= xMax && yMin <= yMax) {
	if (glyph->aa) {
	  for (y = yMin; y <= yMax; ++y) {
	    p = glyph->data + (y - yg) * glyph->w + (xMin - xg);
	    memcpy(scanBuf + xMin, p, xMax - xMin + 1);
	    state->clip->clipSpan(scanBuf, y, xMin, xMax,
				  state->strokeAdjust);
	    (this->*pipe.run)(&pipe, xMin, xMax, y, scanBuf + xMin, NULL);
	  }
	} else {
	  for (y = yMin; y <= yMax; ++y) {
	    p = glyph->data + (y - yg) * ((glyph->w + 7) >> 3)
	      + ((xMin - xg) >> 3);
	    alpha = *p++;
	    xx = (xMin - xg) & 7;
	    alpha = (Guchar)(alpha << xx);
	    for (x = xMin; xx < 8 && x <= xMax; ++x, ++xx) {
	      scanBuf[x] = (alpha & 0x80) ? 255 : 0;
	      alpha = (Guchar)(alpha << 1);
	    }
	    for (; x <= xMax; x += 8) {
	      alpha = *p++;
	      for (xx = 0; xx < 8 && x + xx <= xMax; ++xx) {
		scanBuf[x + xx] = (alpha & 0x80) ? 255 : 0;
		alpha = (Guchar)(alpha << 1);
	      }
	    }
	    state->clip->clipSpanBinary(scanBuf, y, xMin, xMax,
					state->strokeAdjust);
	    (this->*pipe.run)(&pipe, xMin, xMax, y, scanBuf + xMin, NULL);
	  }
	}
      }
    }
  }
  opClipRes = clipRes;

  return splashOk;
}

void Splash::getImageBounds(SplashCoord xyMin, SplashCoord xyMax,
			    int *xyMinI, int *xyMaxI) {
  if (state->strokeAdjust == splashStrokeAdjustOff) {
    *xyMinI = splashFloor(xyMin);
    *xyMaxI = splashFloor(xyMax);
    if (*xyMaxI <= *xyMinI) {
      *xyMaxI = *xyMinI + 1;
    }
  } else {
    splashStrokeAdjust(xyMin, xyMax, xyMinI, xyMaxI, state->strokeAdjust);
  }
}

// The glyphMode flag is not currently used, but may be useful if the
// stroke adjustment behavior is changed.
SplashError Splash::fillImageMask(SplashImageMaskSource src, void *srcData,
				  int w, int h, SplashCoord *mat,
				  GBool glyphMode, GBool interpolate) {
  SplashBitmap *scaledMask;
  SplashClipResult clipRes;
  GBool minorAxisZero;
  SplashCoord wSize, hSize, t0, t1;
  int x0, y0, x1, y1, scaledWidth, scaledHeight;

  if (debugMode) {
    printf("fillImageMask: w=%d h=%d mat=[%.2f %.2f %.2f %.2f %.2f %.2f]\n",
	   w, h, (double)mat[0], (double)mat[1], (double)mat[2],
	   (double)mat[3], (double)mat[4], (double)mat[5]);
  }

  // check for singular matrix
  if (!splashCheckDet(mat[0], mat[1], mat[2], mat[3], 0.000001)) {
    return splashErrSingularMatrix;
  }

  minorAxisZero = splashAbs(mat[1]) <= 0.0001 && splashAbs(mat[2]) <= 0.0001;

  // rough estimate of size of scaled mask
  t0 = splashAbs(mat[0]);
  t1 = splashAbs(mat[1]);
  wSize = t0 > t1 ? t0 : t1;
  t0 = splashAbs(mat[2]);
  t1 = splashAbs(mat[3]);
  hSize = t0 > t1 ? t0 : t1;

  // stream-mode upscaling -- this is slower, so we only use it if the
  // upscaled mask is large (in which case clipping should remove many
  // pixels)
#if USE_FIXEDPOINT
  if ((wSize > 2 * w && hSize > 2 * h && (int)wSize > 1000000 / (int)hSize) ||
      (wSize >     w && hSize >     h && (int)wSize > 10000000 / (int)hSize) ||
      ((wSize > w || hSize > h) && (int)wSize > 25000000 / (int)hSize)) {
#else
  if ((wSize > 2 * w && hSize > 2 * h && wSize * hSize > 1000000) ||
      (wSize >     w && hSize >     h && wSize * hSize > 10000000) ||
      ((wSize > w || hSize > h) && wSize * hSize > 25000000)) {
    upscaleMask(src, srcData, w, h, mat, glyphMode, interpolate);
#endif

  // scaling only
  } else if (mat[0] > 0 && minorAxisZero && mat[3] > 0) {
    getImageBounds(mat[4], mat[0] + mat[4], &x0, &x1);
    getImageBounds(mat[5], mat[3] + mat[5], &y0, &y1);
    clipRes = state->clip->testRect(x0, y0, x1 - 1, y1 - 1,
				    state->strokeAdjust);
    opClipRes = clipRes;
    if (clipRes != splashClipAllOutside) {
      scaledWidth = x1 - x0;
      scaledHeight = y1 - y0;
      scaledMask = scaleMask(src, srcData, w, h, scaledWidth, scaledHeight,
			     interpolate);
      blitMask(scaledMask, x0, y0, clipRes);
      delete scaledMask;
    }
    
  // scaling plus vertical flip
  } else if (mat[0] > 0 && minorAxisZero && mat[3] < 0) {
    getImageBounds(mat[4], mat[0] + mat[4], &x0, &x1);
    getImageBounds(mat[3] + mat[5], mat[5], &y0, &y1);
    clipRes = state->clip->testRect(x0, y0, x1 - 1, y1 - 1,
				    state->strokeAdjust);
    opClipRes = clipRes;
    if (clipRes != splashClipAllOutside) {
      scaledWidth = x1 - x0;
      scaledHeight = y1 - y0;
      scaledMask = scaleMask(src, srcData, w, h, scaledWidth, scaledHeight,
			     interpolate);
      vertFlipImage(scaledMask, scaledWidth, scaledHeight, 1);
      blitMask(scaledMask, x0, y0, clipRes);
      delete scaledMask;
    }

  // scaling plus horizontal flip
  } else if (mat[0] < 0 && minorAxisZero && mat[3] > 0) {
    getImageBounds(mat[0] + mat[4], mat[4], &x0, &x1);
    getImageBounds(mat[5], mat[3] + mat[5], &y0, &y1);
    clipRes = state->clip->testRect(x0, y0, x1 - 1, y1 - 1,
				    state->strokeAdjust);
    opClipRes = clipRes;
    if (clipRes != splashClipAllOutside) {
      scaledWidth = x1 - x0;
      scaledHeight = y1 - y0;
      scaledMask = scaleMask(src, srcData, w, h, scaledWidth, scaledHeight,
			     interpolate);
      horizFlipImage(scaledMask, scaledWidth, scaledHeight, 1);
      blitMask(scaledMask, x0, y0, clipRes);
      delete scaledMask;
    }

  // scaling plus horizontal and vertical flips
  } else if (mat[0] < 0 && minorAxisZero && mat[3] < 0) {
    getImageBounds(mat[0] + mat[4], mat[4], &x0, &x1);
    getImageBounds(mat[3] + mat[5], mat[5], &y0, &y1);
    clipRes = state->clip->testRect(x0, y0, x1 - 1, y1 - 1,
				    state->strokeAdjust);
    opClipRes = clipRes;
    if (clipRes != splashClipAllOutside) {
      scaledWidth = x1 - x0;
      scaledHeight = y1 - y0;
      scaledMask = scaleMask(src, srcData, w, h, scaledWidth, scaledHeight,
			     interpolate);
      vertFlipImage(scaledMask, scaledWidth, scaledHeight, 1);
      horizFlipImage(scaledMask, scaledWidth, scaledHeight, 1);
      blitMask(scaledMask, x0, y0, clipRes);
      delete scaledMask;
    }

  // all other cases
  } else {
    arbitraryTransformMask(src, srcData, w, h, mat, glyphMode, interpolate);
  }

  return splashOk;
}

// The glyphMode flag is not currently used, but may be useful if the
// stroke adjustment behavior is changed.
void Splash::upscaleMask(SplashImageMaskSource src, void *srcData,
			 int srcWidth, int srcHeight,
			 SplashCoord *mat, GBool glyphMode,
			 GBool interpolate) {
  SplashClipResult clipRes;
  SplashPipe pipe;
  Guchar *unscaledImage, *p;
  SplashCoord xMin, yMin, xMax, yMax, t;
  SplashCoord mi0, mi1, mi2, mi3, mi4, mi5, det;
  SplashCoord ix, iy, sx, sy, pix0, pix1;
  int xMinI, yMinI, xMaxI, yMaxI, x, y, x0, y0, x1, y1, tt;

  // compute the bbox of the target quadrilateral
  xMin = xMax = mat[4];
  t = mat[2] + mat[4];
  if (t < xMin) {
    xMin = t;
  } else if (t > xMax) {
    xMax = t;
  }
  t = mat[0] + mat[2] + mat[4];
  if (t < xMin) {
    xMin = t;
  } else if (t > xMax) {
    xMax = t;
  }
  t = mat[0] + mat[4];
  if (t < xMin) {
    xMin = t;
  } else if (t > xMax) {
    xMax = t;
  }
  getImageBounds(xMin, xMax, &xMinI, &xMaxI);
  yMin = yMax = mat[5];
  t = mat[3] + mat[5];
  if (t < yMin) {
    yMin = t;
  } else if (t > yMax) {
    yMax = t;
  }
  t = mat[1] + mat[3] + mat[5];
  if (t < yMin) {
    yMin = t;
  } else if (t > yMax) {
    yMax = t;
  }
  t = mat[1] + mat[5];
  if (t < yMin) {
    yMin = t;
  } else if (t > yMax) {
    yMax = t;
  }
  getImageBounds(yMin, yMax, &yMinI, &yMaxI);

  // clipping
  clipRes = state->clip->testRect(xMinI, yMinI, xMaxI - 1, yMaxI - 1,
				  state->strokeAdjust);
  opClipRes = clipRes;
  if (clipRes == splashClipAllOutside) {
    return;
  }
  if (clipRes != splashClipAllInside) {
    if ((tt = state->clip->getXMinI(state->strokeAdjust)) > xMinI) {
      xMinI = tt;
    }
    if ((tt = state->clip->getXMaxI(state->strokeAdjust) + 1) < xMaxI) {
      xMaxI = tt;
    }
    if ((tt = state->clip->getYMinI(state->strokeAdjust)) > yMinI) {
      yMinI = tt;
    }
    if ((tt = state->clip->getYMaxI(state->strokeAdjust) + 1) < yMaxI) {
      yMaxI = tt;
    }
  }

  // invert the matrix
  det = mat[0] * mat[3] - mat[1] * mat[2];
  if (splashAbs(det) < 1e-6) {
    // this should be caught by the singular matrix check in fillImageMask
    return;
  }
  det = (SplashCoord)1 / det;
  mi0 = det * mat[3] * srcWidth;
  mi1 = -det * mat[1] * srcHeight;
  mi2 = -det * mat[2] * srcWidth;
  mi3 = det * mat[0] * srcHeight;
  mi4 = det * (mat[2] * mat[5] - mat[3] * mat[4]) * srcWidth;
  mi5 = -det * (mat[0] * mat[5] - mat[1] * mat[4]) * srcHeight;

  // grab the image
  unscaledImage = (Guchar *)gmallocn(srcWidth, srcHeight);
  for (y = 0, p = unscaledImage; y < srcHeight; ++y, p += srcWidth) {
    (*src)(srcData, p);
    for (x = 0; x < srcWidth; ++x) {
      p[x] = (Guchar)(p[x] * 255);
    }
  }

  // draw it
  pipeInit(&pipe, state->fillPattern,
	   (Guchar)splashRound(state->fillAlpha * 255),
	   gTrue, gFalse);
  for (y = yMinI; y < yMaxI; ++y) {
    for (x = xMinI; x < xMaxI; ++x) {
      ix = ((SplashCoord)x + 0.5) * mi0 + ((SplashCoord)y + 0.5) * mi2 + mi4;
      iy = ((SplashCoord)x + 0.5) * mi1 + ((SplashCoord)y + 0.5) * mi3 + mi5;
      if (interpolate) {
	if (ix >= 0 && ix < srcWidth && iy >= 0 && iy < srcHeight) {
	  x0 = splashFloor(ix - 0.5);
	  x1 = x0 + 1;
	  sx = (ix - 0.5) - x0;
	  y0 = splashFloor(iy - 0.5);
	  y1 = y0 + 1;
	  sy = (iy - 0.5) - y0;
	  if (x0 < 0) {
	    x0 = 0;
	  }
	  if (x1 >= srcWidth) {
	    x1 = srcWidth - 1;
	  }
	  if (y0 < 0) {
	    y0 = 0;
	  }
	  if (y1 >= srcHeight) {
	    y1 = srcHeight - 1;
	  }
	  pix0 = ((SplashCoord)1 - sx)
	           * (SplashCoord)unscaledImage[y0 * srcWidth + x0]
	         + sx * (SplashCoord)unscaledImage[y0 * srcWidth + x1];
	  pix1 = ((SplashCoord)1 - sx)
	           * (SplashCoord)unscaledImage[y1 * srcWidth + x0]
	         + sx * (SplashCoord)unscaledImage[y1 * srcWidth + x1];
	  scanBuf[x] = (Guchar)splashRound(((SplashCoord)1 - sy) * pix0
					   + sy * pix1);
	} else {
	  scanBuf[x] = 0;
	}
      } else {
	x0 = splashFloor(ix);
	y0 = splashFloor(iy);
	if (x0 >= 0 && x0 < srcWidth && y0 >= 0 && y0 < srcHeight) {
	  scanBuf[x] = unscaledImage[y0 * srcWidth + x0];
	} else {
	  scanBuf[x] = 0;
	}
      }
    }
    if (clipRes != splashClipAllInside) {
      if (vectorAntialias) {
	state->clip->clipSpan(scanBuf, y, xMinI, xMaxI - 1,
			      state->strokeAdjust);
      } else {
	state->clip->clipSpanBinary(scanBuf, y, xMinI, xMaxI - 1,
				    state->strokeAdjust);
      }
    }
    (this->*pipe.run)(&pipe, xMinI, xMaxI - 1, y, scanBuf + xMinI, NULL);
  }

  gfree(unscaledImage);
}

// The glyphMode flag is not currently used, but may be useful if the
// stroke adjustment behavior is changed.
void Splash::arbitraryTransformMask(SplashImageMaskSource src, void *srcData,
				    int srcWidth, int srcHeight,
				    SplashCoord *mat, GBool glyphMode,
				    GBool interpolate) {
  SplashBitmap *scaledMask;
  SplashClipResult clipRes;
  SplashPipe pipe;
  int scaledWidth, scaledHeight, t0, t1;
  SplashCoord r00, r01, r10, r11, det, ir00, ir01, ir10, ir11;
  SplashCoord vx[4], vy[4];
  int xMin, yMin, xMax, yMax;
  ImageSection section[3];
  int nSections;
  int bw, y, xa, xb, x, i, xx, yy;

  // compute the four vertices of the target quadrilateral
  vx[0] = mat[4];                    vy[0] = mat[5];
  vx[1] = mat[2] + mat[4];           vy[1] = mat[3] + mat[5];
  vx[2] = mat[0] + mat[2] + mat[4];  vy[2] = mat[1] + mat[3] + mat[5];
  vx[3] = mat[0] + mat[4];           vy[3] = mat[1] + mat[5];

  // clipping
  xMin = splashRound(vx[0]);
  xMax = splashRound(vx[0]);
  yMin = splashRound(vy[0]);
  yMax = splashRound(vy[0]);
  for (i = 1; i < 4; ++i) {
    t0 = splashRound(vx[i]);
    if (t0 < xMin) {
      xMin = t0;
    } else if (t0 > xMax) {
      xMax = t0;
    }
    t1 = splashRound(vy[i]);
    if (t1 < yMin) {
      yMin = t1;
    } else if (t1 > yMax) {
      yMax = t1;
    }
  }
  clipRes = state->clip->testRect(xMin, yMin, xMax - 1, yMax - 1,
				  state->strokeAdjust);
  opClipRes = clipRes;
  if (clipRes == splashClipAllOutside) {
    return;
  }

  // compute the scale factors
  if (mat[0] >= 0) {
    t0 = splashRound(mat[0] + mat[4]) - splashRound(mat[4]);
  } else {
    t0 = splashRound(mat[4]) - splashRound(mat[0] + mat[4]);
  }
  if (mat[1] >= 0) {
    t1 = splashRound(mat[1] + mat[5]) - splashRound(mat[5]);
  } else {
    t1 = splashRound(mat[5]) - splashRound(mat[1] + mat[5]);
  }
  scaledWidth = t0 > t1 ? t0 : t1;
  if (mat[2] >= 0) {
    t0 = splashRound(mat[2] + mat[4]) - splashRound(mat[4]);
  } else {
    t0 = splashRound(mat[4]) - splashRound(mat[2] + mat[4]);
  }
  if (mat[3] >= 0) {
    t1 = splashRound(mat[3] + mat[5]) - splashRound(mat[5]);
  } else {
    t1 = splashRound(mat[5]) - splashRound(mat[3] + mat[5]);
  }
  scaledHeight = t0 > t1 ? t0 : t1;
  if (scaledWidth == 0) {
    scaledWidth = 1;
  }
  if (scaledHeight == 0) {
    scaledHeight = 1;
  }

  // compute the inverse transform (after scaling) matrix
  r00 = mat[0] / scaledWidth;
  r01 = mat[1] / scaledWidth;
  r10 = mat[2] / scaledHeight;
  r11 = mat[3] / scaledHeight;
  det = r00 * r11 - r01 * r10;
  if (splashAbs(det) < 1e-6) {
    // this should be caught by the singular matrix check in fillImageMask
    return;
  }
  ir00 = r11 / det;
  ir01 = -r01 / det;
  ir10 = -r10 / det;
  ir11 = r00 / det;

  // scale the input image
  scaledMask = scaleMask(src, srcData, srcWidth, srcHeight,
			 scaledWidth, scaledHeight, interpolate);

  // construct the three sections
  i = 0;
  if (vy[1] < vy[i]) {
    i = 1;
  }
  if (vy[2] < vy[i]) {
    i = 2;
  }
  if (vy[3] < vy[i]) {
    i = 3;
  }
  // NB: if using fixed point, 0.000001 will be truncated to zero,
  // so these two comparisons must be <=, not <
  if (splashAbs(vy[i] - vy[(i-1) & 3]) <= 0.000001 &&
      vy[(i-1) & 3] < vy[(i+1) & 3]) {
    i = (i-1) & 3;
  }
  if (splashAbs(vy[i] - vy[(i+1) & 3]) <= 0.000001) {
    section[0].y0 = splashRound(vy[i]);
    section[0].y1 = splashRound(vy[(i+2) & 3]) - 1;
    if (vx[i] < vx[(i+1) & 3]) {
      section[0].ia0 = i;
      section[0].ia1 = (i+3) & 3;
      section[0].ib0 = (i+1) & 3;
      section[0].ib1 = (i+2) & 3;
    } else {
      section[0].ia0 = (i+1) & 3;
      section[0].ia1 = (i+2) & 3;
      section[0].ib0 = i;
      section[0].ib1 = (i+3) & 3;
    }
    nSections = 1;
  } else {
    section[0].y0 = splashRound(vy[i]);
    section[2].y1 = splashRound(vy[(i+2) & 3]) - 1;
    section[0].ia0 = section[0].ib0 = i;
    section[2].ia1 = section[2].ib1 = (i+2) & 3;
    if (vx[(i+1) & 3] < vx[(i+3) & 3]) {
      section[0].ia1 = section[2].ia0 = (i+1) & 3;
      section[0].ib1 = section[2].ib0 = (i+3) & 3;
    } else {
      section[0].ia1 = section[2].ia0 = (i+3) & 3;
      section[0].ib1 = section[2].ib0 = (i+1) & 3;
    }
    if (vy[(i+1) & 3] < vy[(i+3) & 3]) {
      section[1].y0 = splashRound(vy[(i+1) & 3]);
      section[2].y0 = splashRound(vy[(i+3) & 3]);
      if (vx[(i+1) & 3] < vx[(i+3) & 3]) {
	section[1].ia0 = (i+1) & 3;
	section[1].ia1 = (i+2) & 3;
	section[1].ib0 = i;
	section[1].ib1 = (i+3) & 3;
      } else {
	section[1].ia0 = i;
	section[1].ia1 = (i+3) & 3;
	section[1].ib0 = (i+1) & 3;
	section[1].ib1 = (i+2) & 3;
      }
    } else {
      section[1].y0 = splashRound(vy[(i+3) & 3]);
      section[2].y0 = splashRound(vy[(i+1) & 3]);
      if (vx[(i+1) & 3] < vx[(i+3) & 3]) {
	section[1].ia0 = i;
	section[1].ia1 = (i+1) & 3;
	section[1].ib0 = (i+3) & 3;
	section[1].ib1 = (i+2) & 3;
      } else {
	section[1].ia0 = (i+3) & 3;
	section[1].ia1 = (i+2) & 3;
	section[1].ib0 = i;
	section[1].ib1 = (i+1) & 3;
      }
    }
    section[0].y1 = section[1].y0 - 1;
    section[1].y1 = section[2].y0 - 1;
    nSections = 3;
  }
  for (i = 0; i < nSections; ++i) {
    section[i].xa0 = vx[section[i].ia0];
    section[i].ya0 = vy[section[i].ia0];
    section[i].xa1 = vx[section[i].ia1];
    section[i].ya1 = vy[section[i].ia1];
    section[i].xb0 = vx[section[i].ib0];
    section[i].yb0 = vy[section[i].ib0];
    section[i].xb1 = vx[section[i].ib1];
    section[i].yb1 = vy[section[i].ib1];
    section[i].dxdya = (section[i].xa1 - section[i].xa0) /
                       (section[i].ya1 - section[i].ya0);
    section[i].dxdyb = (section[i].xb1 - section[i].xb0) /
                       (section[i].yb1 - section[i].yb0);
  }

  // initialize the pixel pipe
  pipeInit(&pipe, state->fillPattern,
	   (Guchar)splashRound(state->fillAlpha * 255),
	   gTrue, gFalse);

  // make sure narrow images cover at least one pixel
  if (nSections == 1) {
    if (section[0].y0 == section[0].y1) {
      ++section[0].y1;
      clipRes = opClipRes = splashClipPartial;
    }
  } else {
    if (section[0].y0 == section[2].y1) {
      ++section[1].y1;
      clipRes = opClipRes = splashClipPartial;
    }
  }

  // scan all pixels inside the target region
  bw = bitmap->width;
  for (i = 0; i < nSections; ++i) {
    for (y = section[i].y0; y <= section[i].y1; ++y) {
      xa = splashRound(section[i].xa0 +
		         ((SplashCoord)y + 0.5 - section[i].ya0) *
		           section[i].dxdya);
      xb = splashRound(section[i].xb0 +
		         ((SplashCoord)y + 0.5 - section[i].yb0) *
		           section[i].dxdyb);
      if (xa > xb) {
	continue;
      }
      // make sure narrow images cover at least one pixel
      if (xa == xb) {
	++xb;
      }
      // check the scanBuf bounds
      if (xa >= bw || xb < 0) {
	continue;
      }
      if (xa < 0) {
	xa = 0;
      }
      if (xb > bw) {
	xb = bw;
      }
      // get the scan line
      for (x = xa; x < xb; ++x) {
	// map (x+0.5, y+0.5) back to the scaled image
	xx = splashFloor(((SplashCoord)x + 0.5 - mat[4]) * ir00 +
			 ((SplashCoord)y + 0.5 - mat[5]) * ir10);
	yy = splashFloor(((SplashCoord)x + 0.5 - mat[4]) * ir01 +
			 ((SplashCoord)y + 0.5 - mat[5]) * ir11);
	// xx should always be within bounds, but floating point
	// inaccuracy can cause problems
	if (xx < 0) {
	  xx = 0;
	} else if (xx >= scaledWidth) {
	  xx = scaledWidth - 1;
	}
	if (yy < 0) {
	  yy = 0;
	} else if (yy >= scaledHeight) {
	  yy = scaledHeight - 1;
	}
	scanBuf[x] = scaledMask->data[yy * scaledWidth + xx];
      }
      // clip the scan line
      if (clipRes != splashClipAllInside) {
	if (vectorAntialias) {
	  state->clip->clipSpan(scanBuf, y, xa, xb - 1, state->strokeAdjust);
	} else {
	  state->clip->clipSpanBinary(scanBuf, y, xa, xb - 1,
				      state->strokeAdjust);
	}
      }
      // draw the scan line
      (this->*pipe.run)(&pipe, xa, xb - 1, y, scanBuf + xa, NULL);
    }
  }

  delete scaledMask;
}

// Scale an image mask into a SplashBitmap.
SplashBitmap *Splash::scaleMask(SplashImageMaskSource src, void *srcData,
				int srcWidth, int srcHeight,
				int scaledWidth, int scaledHeight,
				GBool interpolate) {
  SplashBitmap *dest;

  dest = new SplashBitmap(scaledWidth, scaledHeight, 1, splashModeMono8,
			  gFalse);
  if (scaledHeight < srcHeight) {
    if (scaledWidth < srcWidth) {
      scaleMaskYdXd(src, srcData, srcWidth, srcHeight,
		    scaledWidth, scaledHeight, dest);
    } else {
      scaleMaskYdXu(src, srcData, srcWidth, srcHeight,
		    scaledWidth, scaledHeight, dest);
    }
  } else {
    if (scaledWidth < srcWidth) {
      scaleMaskYuXd(src, srcData, srcWidth, srcHeight,
		    scaledWidth, scaledHeight, dest);
    } else {
      if (interpolate) {
	scaleMaskYuXuI(src, srcData, srcWidth, srcHeight,
		       scaledWidth, scaledHeight, dest);
      } else {
	scaleMaskYuXu(src, srcData, srcWidth, srcHeight,
		      scaledWidth, scaledHeight, dest);
      }
    }
  }
  return dest;
}

void Splash::scaleMaskYdXd(SplashImageMaskSource src, void *srcData,
			   int srcWidth, int srcHeight,
			   int scaledWidth, int scaledHeight,
			   SplashBitmap *dest) {
  Guchar *lineBuf;
  Guint *pixBuf;
  Guint pix;
  Guchar *destPtr;
  int yp, yq, xp, xq, yt, y, yStep, xt, x, xStep, xx, d, d0, d1;
  int i, j;

  // Bresenham parameters for y scale
  yp = srcHeight / scaledHeight;
  yq = srcHeight % scaledHeight;

  // Bresenham parameters for x scale
  xp = srcWidth / scaledWidth;
  xq = srcWidth % scaledWidth;

  // allocate buffers
  lineBuf = (Guchar *)gmalloc(srcWidth);
  pixBuf = (Guint *)gmallocn(srcWidth, sizeof(int));

  // init y scale Bresenham
  yt = 0;

  destPtr = dest->data;
  for (y = 0; y < scaledHeight; ++y) {

    // y scale Bresenham
    if ((yt += yq) >= scaledHeight) {
      yt -= scaledHeight;
      yStep = yp + 1;
    } else {
      yStep = yp;
    }

    // read rows from image
    memset(pixBuf, 0, srcWidth * sizeof(int));
    for (i = 0; i < yStep; ++i) {
      (*src)(srcData, lineBuf);
      for (j = 0; j < srcWidth; ++j) {
	pixBuf[j] += lineBuf[j];
      }
    }

    // init x scale Bresenham
    xt = 0;
    d0 = (255 << 23) / (yStep * xp);
    d1 = (255 << 23) / (yStep * (xp + 1));

    xx = 0;
    for (x = 0; x < scaledWidth; ++x) {

      // x scale Bresenham
      if ((xt += xq) >= scaledWidth) {
	xt -= scaledWidth;
	xStep = xp + 1;
	d = d1;
      } else {
	xStep = xp;
	d = d0;
      }

      // compute the final pixel
      pix = 0;
      for (i = 0; i < xStep; ++i) {
	pix += pixBuf[xx++];
      }
      // (255 * pix) / xStep * yStep
      pix = (pix * d) >> 23;

      // store the pixel
      *destPtr++ = (Guchar)pix;
    }
  }

  gfree(pixBuf);
  gfree(lineBuf);
}

void Splash::scaleMaskYdXu(SplashImageMaskSource src, void *srcData,
			   int srcWidth, int srcHeight,
			   int scaledWidth, int scaledHeight,
			   SplashBitmap *dest) {
  Guchar *lineBuf;
  Guint *pixBuf;
  Guint pix;
  Guchar *destPtr;
  int yp, yq, xp, xq, yt, y, yStep, xt, x, xStep, d;
  int i, j;

  // Bresenham parameters for y scale
  yp = srcHeight / scaledHeight;
  yq = srcHeight % scaledHeight;

  // Bresenham parameters for x scale
  xp = scaledWidth / srcWidth;
  xq = scaledWidth % srcWidth;

  // allocate buffers
  lineBuf = (Guchar *)gmalloc(srcWidth);
  pixBuf = (Guint *)gmallocn(srcWidth, sizeof(int));

  // init y scale Bresenham
  yt = 0;

  destPtr = dest->data;
  for (y = 0; y < scaledHeight; ++y) {

    // y scale Bresenham
    if ((yt += yq) >= scaledHeight) {
      yt -= scaledHeight;
      yStep = yp + 1;
    } else {
      yStep = yp;
    }

    // read rows from image
    memset(pixBuf, 0, srcWidth * sizeof(int));
    for (i = 0; i < yStep; ++i) {
      (*src)(srcData, lineBuf);
      for (j = 0; j < srcWidth; ++j) {
	pixBuf[j] += lineBuf[j];
      }
    }

    // init x scale Bresenham
    xt = 0;
    d = (255 << 23) / yStep;

    for (x = 0; x < srcWidth; ++x) {

      // x scale Bresenham
      if ((xt += xq) >= srcWidth) {
	xt -= srcWidth;
	xStep = xp + 1;
      } else {
	xStep = xp;
      }

      // compute the final pixel
      pix = pixBuf[x];
      // (255 * pix) / yStep
      pix = (pix * d) >> 23;

      // store the pixel
      for (i = 0; i < xStep; ++i) {
	*destPtr++ = (Guchar)pix;
      }
    }
  }

  gfree(pixBuf);
  gfree(lineBuf);
}

void Splash::scaleMaskYuXd(SplashImageMaskSource src, void *srcData,
			   int srcWidth, int srcHeight,
			   int scaledWidth, int scaledHeight,
			   SplashBitmap *dest) {
  Guchar *lineBuf;
  Guint pix;
  Guchar *destPtr0, *destPtr;
  int yp, yq, xp, xq, yt, y, yStep, xt, x, xStep, xx, d, d0, d1;
  int i;

  // Bresenham parameters for y scale
  yp = scaledHeight / srcHeight;
  yq = scaledHeight % srcHeight;

  // Bresenham parameters for x scale
  xp = srcWidth / scaledWidth;
  xq = srcWidth % scaledWidth;

  // allocate buffers
  lineBuf = (Guchar *)gmalloc(srcWidth);

  // init y scale Bresenham
  yt = 0;

  destPtr0 = dest->data;
  for (y = 0; y < srcHeight; ++y) {

    // y scale Bresenham
    if ((yt += yq) >= srcHeight) {
      yt -= srcHeight;
      yStep = yp + 1;
    } else {
      yStep = yp;
    }

    // read row from image
    (*src)(srcData, lineBuf);

    // init x scale Bresenham
    xt = 0;
    d0 = (255 << 23) / xp;
    d1 = (255 << 23) / (xp + 1);

    xx = 0;
    for (x = 0; x < scaledWidth; ++x) {

      // x scale Bresenham
      if ((xt += xq) >= scaledWidth) {
	xt -= scaledWidth;
	xStep = xp + 1;
	d = d1;
      } else {
	xStep = xp;
	d = d0;
      }

      // compute the final pixel
      pix = 0;
      for (i = 0; i < xStep; ++i) {
	pix += lineBuf[xx++];
      }
      // (255 * pix) / xStep
      pix = (pix * d) >> 23;

      // store the pixel
      for (i = 0; i < yStep; ++i) {
	destPtr = destPtr0 + i * scaledWidth + x;
	*destPtr = (Guchar)pix;
      }
    }

    destPtr0 += yStep * scaledWidth;
  }

  gfree(lineBuf);
}

void Splash::scaleMaskYuXu(SplashImageMaskSource src, void *srcData,
			   int srcWidth, int srcHeight,
			   int scaledWidth, int scaledHeight,
			   SplashBitmap *dest) {
  Guchar *lineBuf;
  Guchar pix;
  Guchar *srcPtr, *destPtr;
  int yp, yq, xp, xq, yt, y, yStep, xt, x, xStep;
  int i;

  // Bresenham parameters for y scale
  yp = scaledHeight / srcHeight;
  yq = scaledHeight % srcHeight;

  // Bresenham parameters for x scale
  xp = scaledWidth / srcWidth;
  xq = scaledWidth % srcWidth;

  // allocate buffers
  lineBuf = (Guchar *)gmalloc(srcWidth);

  // init y scale Bresenham
  yt = 0;

  destPtr = dest->data;
  for (y = 0; y < srcHeight; ++y) {

    // y scale Bresenham
    if ((yt += yq) >= srcHeight) {
      yt -= srcHeight;
      yStep = yp + 1;
    } else {
      yStep = yp;
    }

    // read row from image
    (*src)(srcData, lineBuf);

    // init x scale Bresenham
    xt = 0;

    // generate one row
    srcPtr = lineBuf;
    for (x = 0; x < srcWidth; ++x) {

      // x scale Bresenham
      if ((xt += xq) >= srcWidth) {
	xt -= srcWidth;
	xStep = xp + 1;
      } else {
	xStep = xp;
      }

      // compute the final pixel
      pix = *srcPtr ? 255 : 0;
      ++srcPtr;

      // duplicate the pixel horizontally
      for (i = 0; i < xStep; ++i) {
	*destPtr++ = pix;
      }
    }

    // duplicate the row vertically
    for (i = 1 ; i < yStep; ++i) {
      memcpy(destPtr, destPtr - scaledWidth, scaledWidth);
      destPtr += scaledWidth;
    }
  }

  gfree(lineBuf);
}

void Splash::scaleMaskYuXuI(SplashImageMaskSource src, void *srcData,
			    int srcWidth, int srcHeight,
			    int scaledWidth, int scaledHeight,
			    SplashBitmap *dest) {
  Guchar *lineBuf0, *lineBuf1, *tBuf;
  Guchar pix;
  SplashCoord yr, xr, ys, xs, ySrc, xSrc;
  int ySrc0, ySrc1, yBuf, xSrc0, xSrc1, y, x;
  Guchar *destPtr;

  // ratios
  yr = (SplashCoord)srcHeight / (SplashCoord)scaledHeight;
  xr = (SplashCoord)srcWidth / (SplashCoord)scaledWidth;

  // allocate buffers
  lineBuf0 = (Guchar *)gmalloc(scaledWidth);
  lineBuf1 = (Guchar *)gmalloc(scaledWidth);

  // read first two rows
  (*src)(srcData, lineBuf0);
  if (srcHeight > 1) {
    (*src)(srcData, lineBuf1);
    yBuf = 1;
  } else {
    memcpy(lineBuf1, lineBuf0, srcWidth);
    yBuf = 0;
  }

  // interpolate first two rows
  for (x = scaledWidth - 1; x >= 0; --x) {
    xSrc = xr * x;
    xSrc0 = splashFloor(xSrc + xr * 0.5 - 0.5);
    xSrc1 = xSrc0 + 1;
    xs = ((SplashCoord)xSrc1 + 0.5) - (xSrc + xr * 0.5);
    if (xSrc0 < 0) {
      xSrc0 = 0;
    }
    if (xSrc1 >= srcWidth) {
      xSrc1 = srcWidth - 1;
    }
    lineBuf0[x] = (Guchar)(int)
          ((xs * (int)lineBuf0[xSrc0] +
	    ((SplashCoord)1 - xs) * (int)lineBuf0[xSrc1]) * 255);
    lineBuf1[x] = (Guchar)(int)
          ((xs * (int)lineBuf1[xSrc0] +
	    ((SplashCoord)1 - xs) * (int)lineBuf1[xSrc1]) * 255);
  }

  destPtr = dest->data;
  for (y = 0; y < scaledHeight; ++y) {

    // compute vertical interpolation parameters
    ySrc = yr * y;
    ySrc0 = splashFloor(ySrc + yr * 0.5 - 0.5);
    ySrc1 = ySrc0 + 1;
    ys = ((SplashCoord)ySrc1 + 0.5) - (ySrc + yr * 0.5);
    if (ySrc0 < 0) {
      ySrc0 = 0;
      ys = 1;
    }
    if (ySrc1 >= srcHeight) {
      ySrc1 = srcHeight - 1;
      ys = 0;
    }

    // read another row (if necessary)
    if (ySrc1 > yBuf) {
      tBuf = lineBuf0;
      lineBuf0 = lineBuf1;
      lineBuf1 = tBuf;
      (*src)(srcData, lineBuf1);

      // interpolate the row
      for (x = scaledWidth - 1; x >= 0; --x) {
	xSrc = xr * x;
	xSrc0 = splashFloor(xSrc + xr * 0.5 - 0.5);
	xSrc1 = xSrc0 + 1;
	xs = ((SplashCoord)xSrc1 + 0.5) - (xSrc + xr * 0.5);
	if (xSrc0 < 0) {
	  xSrc0 = 0;
	}
	if (xSrc1 >= srcWidth) {
	  xSrc1 = srcWidth - 1;
	}
	lineBuf1[x] = (Guchar)(int)
	              ((xs * (int)lineBuf1[xSrc0] +
			((SplashCoord)1 - xs) * (int)lineBuf1[xSrc1]) * 255);
      }

      ++yBuf;
    }

    // do the vertical interpolation
    for (x = 0; x < scaledWidth; ++x) {

      pix = (Guchar)(int)(ys * (int)lineBuf0[x] +
			  ((SplashCoord)1 - ys) * (int)lineBuf1[x]);

      // store the pixel
      *destPtr++ = pix;
    }
  }

  gfree(lineBuf1);
  gfree(lineBuf0);
}

void Splash::blitMask(SplashBitmap *src, int xDest, int yDest,
		      SplashClipResult clipRes) {
  SplashPipe pipe;
  int w, h, x0, x1, y0, y1, y, t;

  w = src->width;
  h = src->height;
  pipeInit(&pipe, state->fillPattern,
	   (Guchar)splashRound(state->fillAlpha * 255),
	   gTrue, gFalse);
  if (clipRes == splashClipAllInside) {
    for (y = 0; y < h; ++y) {
      (this->*pipe.run)(&pipe, xDest, xDest + w - 1, yDest + y,
			src->data + y * (size_t)w, NULL);
    }
  } else {
    x0 = xDest;
    if ((t = state->clip->getXMinI(state->strokeAdjust)) > x0) {
      x0 = t;
    }
    x1 = xDest + w;
    if ((t = state->clip->getXMaxI(state->strokeAdjust) + 1) < x1) {
      x1 = t;
    }
    y0 = yDest;
    if ((t = state->clip->getYMinI(state->strokeAdjust)) > y0) {
      y0 = t;
    }
    y1 = yDest + h;
    if ((t = state->clip->getYMaxI(state->strokeAdjust) + 1) < y1) {
      y1 = t;
    }
    if (x0 < x1 && y0 < y1) {
      for (y = y0; y < y1; ++y) {
	memcpy(scanBuf + x0,
	       src->data + (y - yDest) * (size_t)w + (x0 - xDest),
	       x1 - x0);
	if (vectorAntialias) {
	  state->clip->clipSpan(scanBuf, y, x0, x1 - 1,
				state->strokeAdjust);
	} else {
	  state->clip->clipSpanBinary(scanBuf, y, x0, x1 - 1,
				      state->strokeAdjust);
	}
	(this->*pipe.run)(&pipe, x0, x1 - 1, y, scanBuf + x0, NULL);
      }
    }
  }
}

SplashError Splash::drawImage(SplashImageSource src, void *srcData,
			      SplashColorMode srcMode, GBool srcAlpha,
			      int w, int h, SplashCoord *mat,
			      GBool interpolate) {
  GBool ok;
  SplashBitmap *scaledImg;
  SplashClipResult clipRes;
  GBool minorAxisZero;
  SplashCoord wSize, hSize, t0, t1;
  int x0, y0, x1, y1, scaledWidth, scaledHeight;
  int nComps;

  if (debugMode) {
    printf("drawImage: srcMode=%d srcAlpha=%d w=%d h=%d mat=[%.2f %.2f %.2f %.2f %.2f %.2f]\n",
	   srcMode, srcAlpha, w, h, (double)mat[0], (double)mat[1], (double)mat[2],
	   (double)mat[3], (double)mat[4], (double)mat[5]);
  }

  // check color modes
  ok = gFalse; // make gcc happy
  nComps = 0; // make gcc happy
  switch (bitmap->mode) {
  case splashModeMono1:
  case splashModeMono8:
    ok = srcMode == splashModeMono8;
    nComps = 1;
    break;
  case splashModeRGB8:
  case splashModeBGR8:
    ok = srcMode == splashModeRGB8;
    nComps = 3;
    break;
#if SPLASH_CMYK
  case splashModeCMYK8:
    ok = srcMode == splashModeCMYK8;
    nComps = 4;
    break;
#endif
  default:
    ok = gFalse;
    break;
  }
  if (!ok) {
    return splashErrModeMismatch;
  }

  // check for singular matrix
  if (!splashCheckDet(mat[0], mat[1], mat[2], mat[3], 0.000001)) {
    return splashErrSingularMatrix;
  }

  minorAxisZero = splashAbs(mat[1]) <= 0.0001 && splashAbs(mat[2]) <= 0.0001;

  // rough estimate of size of scaled image
  t0 = splashAbs(mat[0]);
  t1 = splashAbs(mat[1]);
  wSize = t0 > t1 ? t0 : t1;
  t0 = splashAbs(mat[2]);
  t1 = splashAbs(mat[3]);
  hSize = t0 > t1 ? t0 : t1;

  // stream-mode upscaling -- this is slower, so we only use it if the
  // upscaled image is large (in which case clipping should remove
  // many pixels)
#if USE_FIXEDPOINT
  if ((wSize > 2 * w && hSize > 2 * h && (int)wSize > 1000000 / (int)hSize) ||
      (wSize >     w && hSize >     h && (int)wSize > 10000000 / (int)hSize) ||
      ((wSize > w || hSize > h) && (int)wSize > 25000000 / (int)hSize)) {
#else
  if ((wSize > 2 * w && hSize > 2 * h && wSize * hSize > 1000000) ||
      (wSize >     w && hSize >     h && wSize * hSize > 10000000) ||
      ((wSize > w || hSize > h) && wSize * hSize > 25000000)) {
#endif
    upscaleImage(src, srcData, srcMode, nComps, srcAlpha,
		 w, h, mat, interpolate);

  // scaling only
  } else if (mat[0] > 0 && minorAxisZero && mat[3] > 0) {
    getImageBounds(mat[4], mat[0] + mat[4], &x0, &x1);
    getImageBounds(mat[5], mat[3] + mat[5], &y0, &y1);
    clipRes = state->clip->testRect(x0, y0, x1 - 1, y1 - 1,
				    state->strokeAdjust);
    opClipRes = clipRes;
    if (clipRes != splashClipAllOutside) {
      scaledWidth = x1 - x0;
      scaledHeight = y1 - y0;
      scaledImg = scaleImage(src, srcData, srcMode, nComps, srcAlpha, w, h,
			     scaledWidth, scaledHeight, interpolate);
      blitImage(scaledImg, srcAlpha, x0, y0, clipRes);
      delete scaledImg;
    }
    
  // scaling plus vertical flip
  } else if (mat[0] > 0 && minorAxisZero && mat[3] < 0) {
    getImageBounds(mat[4], mat[0] + mat[4], &x0, &x1);
    getImageBounds(mat[3] + mat[5], mat[5], &y0, &y1);
    clipRes = state->clip->testRect(x0, y0, x1 - 1, y1 - 1,
				    state->strokeAdjust);
    opClipRes = clipRes;
    if (clipRes != splashClipAllOutside) {
      scaledWidth = x1 - x0;
      scaledHeight = y1 - y0;
      scaledImg = scaleImage(src, srcData, srcMode, nComps, srcAlpha, w, h,
			     scaledWidth, scaledHeight, interpolate);
      vertFlipImage(scaledImg, scaledWidth, scaledHeight, nComps);
      blitImage(scaledImg, srcAlpha, x0, y0, clipRes);
      delete scaledImg;
    }

  // scaling plus horizontal flip
  } else if (mat[0] < 0 && minorAxisZero && mat[3] > 0) {
    getImageBounds(mat[0] + mat[4], mat[4], &x0, &x1);
    getImageBounds(mat[5], mat[3] + mat[5], &y0, &y1);
    clipRes = state->clip->testRect(x0, y0, x1 - 1, y1 - 1,
				    state->strokeAdjust);
    opClipRes = clipRes;
    if (clipRes != splashClipAllOutside) {
      scaledWidth = x1 - x0;
      scaledHeight = y1 - y0;
      scaledImg = scaleImage(src, srcData, srcMode, nComps, srcAlpha, w, h,
			     scaledWidth, scaledHeight, interpolate);
      horizFlipImage(scaledImg, scaledWidth, scaledHeight, nComps);
      blitImage(scaledImg, srcAlpha, x0, y0, clipRes);
      delete scaledImg;
    }
    
  // scaling plus horizontal and vertical flips
  } else if (mat[0] < 0 && minorAxisZero && mat[3] < 0) {
    getImageBounds(mat[0] + mat[4], mat[4], &x0, &x1);
    getImageBounds(mat[3] + mat[5], mat[5], &y0, &y1);
    clipRes = state->clip->testRect(x0, y0, x1 - 1, y1 - 1,
				    state->strokeAdjust);
    opClipRes = clipRes;
    if (clipRes != splashClipAllOutside) {
      scaledWidth = x1 - x0;
      scaledHeight = y1 - y0;
      scaledImg = scaleImage(src, srcData, srcMode, nComps, srcAlpha, w, h,
			     scaledWidth, scaledHeight, interpolate);
      vertFlipImage(scaledImg, scaledWidth, scaledHeight, nComps);
      horizFlipImage(scaledImg, scaledWidth, scaledHeight, nComps);
      blitImage(scaledImg, srcAlpha, x0, y0, clipRes);
      delete scaledImg;
    }

  // all other cases
  } else {
    arbitraryTransformImage(src, srcData, srcMode, nComps, srcAlpha,
			    w, h, mat, interpolate);
  }

  return splashOk;
}

void Splash::upscaleImage(SplashImageSource src, void *srcData,
			  SplashColorMode srcMode, int nComps,
			  GBool srcAlpha, int srcWidth, int srcHeight,
			  SplashCoord *mat, GBool interpolate) {
  SplashClipResult clipRes;
  SplashPipe pipe;
  SplashColorPtr unscaledImage, pixelBuf, p, q, q00, q01, q10, q11;
  Guchar *unscaledAlpha, *alphaPtr;
  SplashCoord xMin, yMin, xMax, yMax, t;
  SplashCoord mi0, mi1, mi2, mi3, mi4, mi5, det;
  SplashCoord ix, iy, sx, sy, pix0, pix1;
  SplashBitmapRowSize rowSize;
  int xMinI, yMinI, xMaxI, yMaxI, x, y, x0, y0, x1, y1, tt, i;

  // compute the bbox of the target quadrilateral
  xMin = xMax = mat[4];
  t = mat[2] + mat[4];
  if (t < xMin) {
    xMin = t;
  } else if (t > xMax) {
    xMax = t;
  }
  t = mat[0] + mat[2] + mat[4];
  if (t < xMin) {
    xMin = t;
  } else if (t > xMax) {
    xMax = t;
  }
  t = mat[0] + mat[4];
  if (t < xMin) {
    xMin = t;
  } else if (t > xMax) {
    xMax = t;
  }
  getImageBounds(xMin, xMax, &xMinI, &xMaxI);
  yMin = yMax = mat[5];
  t = mat[3] + mat[5];
  if (t < yMin) {
    yMin = t;
  } else if (t > yMax) {
    yMax = t;
  }
  t = mat[1] + mat[3] + mat[5];
  if (t < yMin) {
    yMin = t;
  } else if (t > yMax) {
    yMax = t;
  }
  t = mat[1] + mat[5];
  if (t < yMin) {
    yMin = t;
  } else if (t > yMax) {
    yMax = t;
  }
  getImageBounds(yMin, yMax, &yMinI, &yMaxI);

  // clipping
  clipRes = state->clip->testRect(xMinI, yMinI, xMaxI - 1, yMaxI - 1,
				  state->strokeAdjust);
  opClipRes = clipRes;
  if (clipRes == splashClipAllOutside) {
    return;
  }
  if (clipRes != splashClipAllInside) {
    if ((tt = state->clip->getXMinI(state->strokeAdjust)) > xMinI) {
      xMinI = tt;
    }
    if ((tt = state->clip->getXMaxI(state->strokeAdjust) + 1) < xMaxI) {
      xMaxI = tt;
    }
    if ((tt = state->clip->getYMinI(state->strokeAdjust)) > yMinI) {
      yMinI = tt;
    }
    if ((tt = state->clip->getYMaxI(state->strokeAdjust) + 1) < yMaxI) {
      yMaxI = tt;
    }
  }

  // invert the matrix
  det = mat[0] * mat[3] - mat[1] * mat[2];
  if (splashAbs(det) < 1e-6) {
    // this should be caught by the singular matrix check in fillImageMask
    return;
  }
  det = (SplashCoord)1 / det;
  mi0 = det * mat[3] * srcWidth;
  mi1 = -det * mat[1] * srcHeight;
  mi2 = -det * mat[2] * srcWidth;
  mi3 = det * mat[0] * srcHeight;
  mi4 = det * (mat[2] * mat[5] - mat[3] * mat[4]) * srcWidth;
  mi5 = -det * (mat[0] * mat[5] - mat[1] * mat[4]) * srcHeight;

  // grab the image
  if (srcWidth > INT_MAX / nComps) {
    rowSize = -1;
  } else {
    rowSize = srcWidth * nComps;
  }
  unscaledImage = (SplashColorPtr)gmallocn64(srcHeight, rowSize);
  if (srcAlpha) {
    unscaledAlpha = (Guchar *)gmallocn(srcHeight, srcWidth);
    for (y = 0, p = unscaledImage, alphaPtr = unscaledAlpha;
	 y < srcHeight;
	 ++y, p += rowSize, alphaPtr += srcWidth) {
      (*src)(srcData, p, alphaPtr);
    }
  } else {
    unscaledAlpha = NULL;
    for (y = 0, p = unscaledImage; y < srcHeight; ++y, p += rowSize) {
      (*src)(srcData, p, NULL);
    }
  }

  // draw it
  pixelBuf = (SplashColorPtr)gmallocn(xMaxI - xMinI, nComps);
  pipeInit(&pipe, NULL,
	   (Guchar)splashRound(state->fillAlpha * 255),
	   gTrue, gFalse);
  for (y = yMinI; y < yMaxI; ++y) {
    p = pixelBuf;
    for (x = xMinI; x < xMaxI; ++x) {
      ix = ((SplashCoord)x + 0.5) * mi0 + ((SplashCoord)y + 0.5) * mi2 + mi4;
      iy = ((SplashCoord)x + 0.5) * mi1 + ((SplashCoord)y + 0.5) * mi3 + mi5;
      if (interpolate) {
	if (ix >= 0 && ix < srcWidth && iy >= 0 && iy < srcHeight) {
	  x0 = splashFloor(ix - 0.5);
	  x1 = x0 + 1;
	  sx = (ix - 0.5) - x0;
	  y0 = splashFloor(iy - 0.5);
	  y1 = y0 + 1;
	  sy = (iy - 0.5) - y0;
	  if (x0 < 0) {
	    x0 = 0;
	  }
	  if (x1 >= srcWidth) {
	    x1 = srcWidth - 1;
	  }
	  if (y0 < 0) {
	    y0 = 0;
	  }
	  if (y1 >= srcHeight) {
	    y1 = srcHeight - 1;
	  }
	  q00 = &unscaledImage[y0 * rowSize + (SplashBitmapRowSize)x0 * nComps];
	  q01 = &unscaledImage[y0 * rowSize + (SplashBitmapRowSize)x1 * nComps];
	  q10 = &unscaledImage[y1 * rowSize + (SplashBitmapRowSize)x0 * nComps];
	  q11 = &unscaledImage[y1 * rowSize + (SplashBitmapRowSize)x1 * nComps];
	  for (i = 0; i < nComps; ++i) {
	    pix0 = ((SplashCoord)1 - sx) * (int)*q00++ + sx * (int)*q01++;
	    pix1 = ((SplashCoord)1 - sx) * (int)*q10++ + sx * (int)*q11++;
	    *p++ = (Guchar)splashRound(((SplashCoord)1 - sy) * pix0
				       + sy * pix1);
	  }
	  if (srcAlpha) {
	    pix0 = ((SplashCoord)1 - sx)
	             * (SplashCoord)unscaledAlpha[y0 * srcWidth + x0]
	           + sx * (SplashCoord)unscaledAlpha[y0 * srcWidth + x1];
	    pix1 = ((SplashCoord)1 - sx)
	             * (SplashCoord)unscaledAlpha[y1 * srcWidth + x0]
	           + sx * (SplashCoord)unscaledAlpha[y1 * srcWidth + x1];
	    scanBuf[x] = (Guchar)splashRound(((SplashCoord)1 - sy) * pix0
					     + sy * pix1);
	  } else {
	    scanBuf[x] = 0xff;
	  }
	} else {
	  for (i = 0; i < nComps; ++i) {
	    *p++ = 0;
	  }
	  scanBuf[x] = 0;
	}
      } else {
	x0 = splashFloor(ix);
	y0 = splashFloor(iy);
	if (x0 >= 0 && x0 < srcWidth && y0 >= 0 && y0 < srcHeight) {
	  q = &unscaledImage[y0 * rowSize + (SplashBitmapRowSize)x0 * nComps];
	  for (i = 0; i < nComps; ++i) {
	    *p++ = *q++;
	  }
	  if (srcAlpha) {
	    scanBuf[x] = unscaledAlpha[y0 * srcWidth + x0];
	  } else {
	    scanBuf[x] = 0xff;
	  }
	} else {
	  for (i = 0; i < nComps; ++i) {
	    *p++ = 0;
	  }
	  scanBuf[x] = 0;
	}
      }
    }
    if (clipRes != splashClipAllInside) {
      if (vectorAntialias) {
	state->clip->clipSpan(scanBuf, y, xMinI, xMaxI - 1,
			      state->strokeAdjust);
      } else {
	state->clip->clipSpanBinary(scanBuf, y, xMinI, xMaxI - 1,
				    state->strokeAdjust);
      }
    }
    (this->*pipe.run)(&pipe, xMinI, xMaxI - 1, y, scanBuf + xMinI, pixelBuf);
  }

  gfree(pixelBuf);
  gfree(unscaledImage);
  gfree(unscaledAlpha);
}

void Splash::arbitraryTransformImage(SplashImageSource src, void *srcData,
				     SplashColorMode srcMode, int nComps,
				     GBool srcAlpha,
				     int srcWidth, int srcHeight,
				     SplashCoord *mat, GBool interpolate) {
  SplashBitmap *scaledImg;
  SplashClipResult clipRes;
  SplashPipe pipe;
  SplashColorPtr pixelBuf;
  int scaledWidth, scaledHeight, t0, t1;
  SplashCoord r00, r01, r10, r11, det, ir00, ir01, ir10, ir11;
  SplashCoord vx[4], vy[4];
  int xMin, yMin, xMax, yMax;
  ImageSection section[3];
  int nSections;
  int y, xa, xb, x, i, xx, yy;

  // compute the four vertices of the target quadrilateral
  vx[0] = mat[4];                    vy[0] = mat[5];
  vx[1] = mat[2] + mat[4];           vy[1] = mat[3] + mat[5];
  vx[2] = mat[0] + mat[2] + mat[4];  vy[2] = mat[1] + mat[3] + mat[5];
  vx[3] = mat[0] + mat[4];           vy[3] = mat[1] + mat[5];

  // clipping
  xMin = splashRound(vx[0]);
  xMax = splashRound(vx[0]);
  yMin = splashRound(vy[0]);
  yMax = splashRound(vy[0]);
  for (i = 1; i < 4; ++i) {
    t0 = splashRound(vx[i]);
    if (t0 < xMin) {
      xMin = t0;
    } else if (t0 > xMax) {
      xMax = t0;
    }
    t1 = splashRound(vy[i]);
    if (t1 < yMin) {
      yMin = t1;
    } else if (t1 > yMax) {
      yMax = t1;
    }
  }
  clipRes = state->clip->testRect(xMin, yMin, xMax - 1, yMax - 1,
				  state->strokeAdjust);
  opClipRes = clipRes;
  if (clipRes == splashClipAllOutside) {
    return;
  }

  // compute the scale factors
  if (mat[0] >= 0) {
    t0 = splashRound(mat[0] + mat[4]) - splashRound(mat[4]);
  } else {
    t0 = splashRound(mat[4]) - splashRound(mat[0] + mat[4]);
  }
  if (mat[1] >= 0) {
    t1 = splashRound(mat[1] + mat[5]) - splashRound(mat[5]);
  } else {
    t1 = splashRound(mat[5]) - splashRound(mat[1] + mat[5]);
  }
  scaledWidth = t0 > t1 ? t0 : t1;
  if (mat[2] >= 0) {
    t0 = splashRound(mat[2] + mat[4]) - splashRound(mat[4]);
  } else {
    t0 = splashRound(mat[4]) - splashRound(mat[2] + mat[4]);
  }
  if (mat[3] >= 0) {
    t1 = splashRound(mat[3] + mat[5]) - splashRound(mat[5]);
  } else {
    t1 = splashRound(mat[5]) - splashRound(mat[3] + mat[5]);
  }
  scaledHeight = t0 > t1 ? t0 : t1;
  if (scaledWidth == 0) {
    scaledWidth = 1;
  }
  if (scaledHeight == 0) {
    scaledHeight = 1;
  }

  // compute the inverse transform (after scaling) matrix
  r00 = mat[0] / scaledWidth;
  r01 = mat[1] / scaledWidth;
  r10 = mat[2] / scaledHeight;
  r11 = mat[3] / scaledHeight;
  det = r00 * r11 - r01 * r10;
  if (splashAbs(det) < 1e-6) {
    // this should be caught by the singular matrix check in drawImage
    return;
  }
  ir00 = r11 / det;
  ir01 = -r01 / det;
  ir10 = -r10 / det;
  ir11 = r00 / det;

  // scale the input image
  scaledImg = scaleImage(src, srcData, srcMode, nComps, srcAlpha,
			 srcWidth, srcHeight, scaledWidth, scaledHeight,
			 interpolate);

  // construct the three sections
  i = 0;
  if (vy[1] < vy[i]) {
    i = 1;
  }
  if (vy[2] < vy[i]) {
    i = 2;
  }
  if (vy[3] < vy[i]) {
    i = 3;
  }
  // NB: if using fixed point, 0.000001 will be truncated to zero,
  // so these two comparisons must be <=, not <
  if (splashAbs(vy[i] - vy[(i-1) & 3]) <= 0.000001 &&
      vy[(i-1) & 3] < vy[(i+1) & 3]) {
    i = (i-1) & 3;
  }
  if (splashAbs(vy[i] - vy[(i+1) & 3]) <= 0.000001) {
    section[0].y0 = splashRound(vy[i]);
    section[0].y1 = splashRound(vy[(i+2) & 3]) - 1;
    if (vx[i] < vx[(i+1) & 3]) {
      section[0].ia0 = i;
      section[0].ia1 = (i+3) & 3;
      section[0].ib0 = (i+1) & 3;
      section[0].ib1 = (i+2) & 3;
    } else {
      section[0].ia0 = (i+1) & 3;
      section[0].ia1 = (i+2) & 3;
      section[0].ib0 = i;
      section[0].ib1 = (i+3) & 3;
    }
    nSections = 1;
  } else {
    section[0].y0 = splashRound(vy[i]);
    section[2].y1 = splashRound(vy[(i+2) & 3]) - 1;
    section[0].ia0 = section[0].ib0 = i;
    section[2].ia1 = section[2].ib1 = (i+2) & 3;
    if (vx[(i+1) & 3] < vx[(i+3) & 3]) {
      section[0].ia1 = section[2].ia0 = (i+1) & 3;
      section[0].ib1 = section[2].ib0 = (i+3) & 3;
    } else {
      section[0].ia1 = section[2].ia0 = (i+3) & 3;
      section[0].ib1 = section[2].ib0 = (i+1) & 3;
    }
    if (vy[(i+1) & 3] < vy[(i+3) & 3]) {
      section[1].y0 = splashRound(vy[(i+1) & 3]);
      section[2].y0 = splashRound(vy[(i+3) & 3]);
      if (vx[(i+1) & 3] < vx[(i+3) & 3]) {
	section[1].ia0 = (i+1) & 3;
	section[1].ia1 = (i+2) & 3;
	section[1].ib0 = i;
	section[1].ib1 = (i+3) & 3;
      } else {
	section[1].ia0 = i;
	section[1].ia1 = (i+3) & 3;
	section[1].ib0 = (i+1) & 3;
	section[1].ib1 = (i+2) & 3;
      }
    } else {
      section[1].y0 = splashRound(vy[(i+3) & 3]);
      section[2].y0 = splashRound(vy[(i+1) & 3]);
      if (vx[(i+1) & 3] < vx[(i+3) & 3]) {
	section[1].ia0 = i;
	section[1].ia1 = (i+1) & 3;
	section[1].ib0 = (i+3) & 3;
	section[1].ib1 = (i+2) & 3;
      } else {
	section[1].ia0 = (i+3) & 3;
	section[1].ia1 = (i+2) & 3;
	section[1].ib0 = i;
	section[1].ib1 = (i+1) & 3;
      }
    }
    section[0].y1 = section[1].y0 - 1;
    section[1].y1 = section[2].y0 - 1;
    nSections = 3;
  }
  for (i = 0; i < nSections; ++i) {
    section[i].xa0 = vx[section[i].ia0];
    section[i].ya0 = vy[section[i].ia0];
    section[i].xa1 = vx[section[i].ia1];
    section[i].ya1 = vy[section[i].ia1];
    section[i].xb0 = vx[section[i].ib0];
    section[i].yb0 = vy[section[i].ib0];
    section[i].xb1 = vx[section[i].ib1];
    section[i].yb1 = vy[section[i].ib1];
    section[i].dxdya = (section[i].xa1 - section[i].xa0) /
                       (section[i].ya1 - section[i].ya0);
    section[i].dxdyb = (section[i].xb1 - section[i].xb0) /
                       (section[i].yb1 - section[i].yb0);
  }

  // initialize the pixel pipe
  pipeInit(&pipe, NULL,
	   (Guchar)splashRound(state->fillAlpha * 255),
	   gTrue, gFalse);

  // make sure narrow images cover at least one pixel
  if (nSections == 1) {
    if (section[0].y0 == section[0].y1) {
      ++section[0].y1;
      clipRes = opClipRes = splashClipPartial;
    }
  } else {
    if (section[0].y0 == section[2].y1) {
      ++section[1].y1;
      clipRes = opClipRes = splashClipPartial;
    }
  }

  pixelBuf = (SplashColorPtr)gmallocn(xMax - xMin + 1, bitmapComps);

  // scan all pixels inside the target region
  for (i = 0; i < nSections; ++i) {
    for (y = section[i].y0; y <= section[i].y1; ++y) {
      xa = splashRound(section[i].xa0 +
		         ((SplashCoord)y + 0.5 - section[i].ya0) *
		           section[i].dxdya);
      xb = splashRound(section[i].xb0 +
		         ((SplashCoord)y + 0.5 - section[i].yb0) *
		           section[i].dxdyb);
      if (xa > xb) {
	continue;
      }
      // make sure narrow images cover at least one pixel
      if (xa == xb) {
	++xb;
      }
      // check the scanBuf bounds
      if (xa >= bitmap->width || xb < 0) {
	continue;
      }
      if (xa < 0) {
	xa = 0;
      }
      if (xb > bitmap->width) {
	xb = bitmap->width;
      }
      // clip the scan line
      memset(scanBuf + xa, 0xff, xb - xa);
      if (clipRes != splashClipAllInside) {
	if (vectorAntialias) {
	  state->clip->clipSpan(scanBuf, y, xa, xb - 1,
				state->strokeAdjust);
	} else {
	  state->clip->clipSpanBinary(scanBuf, y, xa, xb - 1,
				      state->strokeAdjust);
	}
      }
      // draw the scan line
      for (x = xa; x < xb; ++x) {
	// map (x+0.5, y+0.5) back to the scaled image
	xx = splashFloor(((SplashCoord)x + 0.5 - mat[4]) * ir00 +
			 ((SplashCoord)y + 0.5 - mat[5]) * ir10);
	yy = splashFloor(((SplashCoord)x + 0.5 - mat[4]) * ir01 +
			 ((SplashCoord)y + 0.5 - mat[5]) * ir11);
	// xx should always be within bounds, but floating point
	// inaccuracy can cause problems
	if (xx < 0) {
	  xx = 0;
	} else if (xx >= scaledWidth) {
	  xx = scaledWidth - 1;
	}
	if (yy < 0) {
	  yy = 0;
	} else if (yy >= scaledHeight) {
	  yy = scaledHeight - 1;
	}
	// get the color
	scaledImg->getPixel(xx, yy, pixelBuf + (x - xa) * bitmapComps);
	// apply alpha
	if (srcAlpha) {
	  scanBuf[x] = div255(scanBuf[x] *
			      scaledImg->alpha[yy * scaledWidth + xx]);
	}
      }
      (this->*pipe.run)(&pipe, xa, xb - 1, y, scanBuf + xa, pixelBuf);
    }
  }

  gfree(pixelBuf);
  delete scaledImg;
}

// Scale an image into a SplashBitmap.
SplashBitmap *Splash::scaleImage(SplashImageSource src, void *srcData,
				 SplashColorMode srcMode, int nComps,
				 GBool srcAlpha, int srcWidth, int srcHeight,
				 int scaledWidth, int scaledHeight,
				 GBool interpolate) {
  SplashBitmap *dest;

  dest = new SplashBitmap(scaledWidth, scaledHeight, 1, srcMode, srcAlpha);
  if (scaledHeight < srcHeight) {
    if (scaledWidth < srcWidth) {
      scaleImageYdXd(src, srcData, srcMode, nComps, srcAlpha,
		     srcWidth, srcHeight, scaledWidth, scaledHeight, dest);
    } else {
      scaleImageYdXu(src, srcData, srcMode, nComps, srcAlpha,
		     srcWidth, srcHeight, scaledWidth, scaledHeight, dest);
    }
  } else {
    if (scaledWidth < srcWidth) {
      scaleImageYuXd(src, srcData, srcMode, nComps, srcAlpha,
		     srcWidth, srcHeight, scaledWidth, scaledHeight, dest);
    } else {
      if (interpolate) {
	scaleImageYuXuI(src, srcData, srcMode, nComps, srcAlpha,
			srcWidth, srcHeight, scaledWidth, scaledHeight, dest);
      } else {
	scaleImageYuXu(src, srcData, srcMode, nComps, srcAlpha,
		       srcWidth, srcHeight, scaledWidth, scaledHeight, dest);
      }
    }
  }
  return dest;
}

void Splash::scaleImageYdXd(SplashImageSource src, void *srcData,
			    SplashColorMode srcMode, int nComps,
			    GBool srcAlpha, int srcWidth, int srcHeight,
			    int scaledWidth, int scaledHeight,
			    SplashBitmap *dest) {
  Guchar *lineBuf, *alphaLineBuf;
  Guint *pixBuf, *alphaPixBuf;
  Guint pix0, pix1, pix2;
#if SPLASH_CMYK
  Guint pix3;
#endif
  Guint alpha;
  Guchar *destPtr, *destAlphaPtr;
  int yp, yq, xp, xq, yt, y, yStep, xt, x, xStep, xx, xxa, d, d0, d1;
  int i, j;

  // Bresenham parameters for y scale
  yp = srcHeight / scaledHeight;
  yq = srcHeight % scaledHeight;

  // Bresenham parameters for x scale
  xp = srcWidth / scaledWidth;
  xq = srcWidth % scaledWidth;

  // allocate buffers
  lineBuf = (Guchar *)gmallocn(srcWidth, nComps);
  pixBuf = (Guint *)gmallocn(srcWidth, (int)(nComps * sizeof(int)));
  if (srcAlpha) {
    alphaLineBuf = (Guchar *)gmalloc(srcWidth);
    alphaPixBuf = (Guint *)gmallocn(srcWidth, sizeof(int));
  } else {
    alphaLineBuf = NULL;
    alphaPixBuf = NULL;
  }

  // init y scale Bresenham
  yt = 0;

  destPtr = dest->data;
  destAlphaPtr = dest->alpha;
  for (y = 0; y < scaledHeight; ++y) {

    // y scale Bresenham
    if ((yt += yq) >= scaledHeight) {
      yt -= scaledHeight;
      yStep = yp + 1;
    } else {
      yStep = yp;
    }

    // read rows from image
    memset(pixBuf, 0, srcWidth * nComps * sizeof(int));
    if (srcAlpha) {
      memset(alphaPixBuf, 0, srcWidth * sizeof(int));
    }
    for (i = 0; i < yStep; ++i) {
      (*src)(srcData, lineBuf, alphaLineBuf);
      for (j = 0; j < srcWidth * nComps; ++j) {
	pixBuf[j] += lineBuf[j];
      }
      if (srcAlpha) {
	for (j = 0; j < srcWidth; ++j) {
	  alphaPixBuf[j] += alphaLineBuf[j];
	}
      }
    }

    // init x scale Bresenham
    xt = 0;
    d0 = (1 << 23) / (yStep * xp);
    d1 = (1 << 23) / (yStep * (xp + 1));

    xx = xxa = 0;
    for (x = 0; x < scaledWidth; ++x) {

      // x scale Bresenham
      if ((xt += xq) >= scaledWidth) {
	xt -= scaledWidth;
	xStep = xp + 1;
	d = d1;
      } else {
	xStep = xp;
	d = d0;
      }

      switch (srcMode) {

      case splashModeMono8:

	// compute the final pixel
	pix0 = 0;
	for (i = 0; i < xStep; ++i) {
	  pix0 += pixBuf[xx++];
	}
	// pix / xStep * yStep
	pix0 = (pix0 * d) >> 23;

	// store the pixel
	*destPtr++ = (Guchar)pix0;
	break;

      case splashModeRGB8:

	// compute the final pixel
	pix0 = pix1 = pix2 = 0;
	for (i = 0; i < xStep; ++i) {
	  pix0 += pixBuf[xx];
	  pix1 += pixBuf[xx+1];
	  pix2 += pixBuf[xx+2];
	  xx += 3;
	}
	// pix / xStep * yStep
	pix0 = (pix0 * d) >> 23;
	pix1 = (pix1 * d) >> 23;
	pix2 = (pix2 * d) >> 23;

	// store the pixel
	*destPtr++ = (Guchar)pix0;
	*destPtr++ = (Guchar)pix1;
	*destPtr++ = (Guchar)pix2;
	break;

#if SPLASH_CMYK
      case splashModeCMYK8:

	// compute the final pixel
	pix0 = pix1 = pix2 = pix3 = 0;
	for (i = 0; i < xStep; ++i) {
	  pix0 += pixBuf[xx];
	  pix1 += pixBuf[xx+1];
	  pix2 += pixBuf[xx+2];
	  pix3 += pixBuf[xx+3];
	  xx += 4;
	}
	// pix / xStep * yStep
	pix0 = (pix0 * d) >> 23;
	pix1 = (pix1 * d) >> 23;
	pix2 = (pix2 * d) >> 23;
	pix3 = (pix3 * d) >> 23;

	// store the pixel
	*destPtr++ = (Guchar)pix0;
	*destPtr++ = (Guchar)pix1;
	*destPtr++ = (Guchar)pix2;
	*destPtr++ = (Guchar)pix3;
	break;
#endif


      case splashModeMono1: // mono1 is not allowed
      case splashModeBGR8: // bgr8 is not allowed
      default:
	break;
      }

      // process alpha
      if (srcAlpha) {
	alpha = 0;
	for (i = 0; i < xStep; ++i, ++xxa) {
	  alpha += alphaPixBuf[xxa];
	}
	// alpha / xStep * yStep
	alpha = (alpha * d) >> 23;
	*destAlphaPtr++ = (Guchar)alpha;
      }
    }
  }

  gfree(alphaPixBuf);
  gfree(alphaLineBuf);
  gfree(pixBuf);
  gfree(lineBuf);
}

void Splash::scaleImageYdXu(SplashImageSource src, void *srcData,
			    SplashColorMode srcMode, int nComps,
			    GBool srcAlpha, int srcWidth, int srcHeight,
			    int scaledWidth, int scaledHeight,
			    SplashBitmap *dest) {
  Guchar *lineBuf, *alphaLineBuf;
  Guint *pixBuf, *alphaPixBuf;
  Guint pix[splashMaxColorComps];
  Guint alpha;
  Guchar *destPtr, *destAlphaPtr;
  int yp, yq, xp, xq, yt, y, yStep, xt, x, xStep, d;
  int i, j;

  // Bresenham parameters for y scale
  yp = srcHeight / scaledHeight;
  yq = srcHeight % scaledHeight;

  // Bresenham parameters for x scale
  xp = scaledWidth / srcWidth;
  xq = scaledWidth % srcWidth;

  // allocate buffers
  lineBuf = (Guchar *)gmallocn(srcWidth, nComps);
  pixBuf = (Guint *)gmallocn(srcWidth, (int)(nComps * sizeof(int)));
  if (srcAlpha) {
    alphaLineBuf = (Guchar *)gmalloc(srcWidth);
    alphaPixBuf = (Guint *)gmallocn(srcWidth, sizeof(int));
  } else {
    alphaLineBuf = NULL;
    alphaPixBuf = NULL;
  }

  // make gcc happy
  pix[0] = pix[1] = pix[2] = 0;
#if SPLASH_CMYK
  pix[3] = 0;
#endif

  // init y scale Bresenham
  yt = 0;

  destPtr = dest->data;
  destAlphaPtr = dest->alpha;
  for (y = 0; y < scaledHeight; ++y) {

    // y scale Bresenham
    if ((yt += yq) >= scaledHeight) {
      yt -= scaledHeight;
      yStep = yp + 1;
    } else {
      yStep = yp;
    }

    // read rows from image
    memset(pixBuf, 0, srcWidth * nComps * sizeof(int));
    if (srcAlpha) {
      memset(alphaPixBuf, 0, srcWidth * sizeof(int));
    }
    for (i = 0; i < yStep; ++i) {
      (*src)(srcData, lineBuf, alphaLineBuf);
      for (j = 0; j < srcWidth * nComps; ++j) {
	pixBuf[j] += lineBuf[j];
      }
      if (srcAlpha) {
	for (j = 0; j < srcWidth; ++j) {
	  alphaPixBuf[j] += alphaLineBuf[j];
	}
      }
    }

    // init x scale Bresenham
    xt = 0;
    d = (1 << 23) / yStep;

    for (x = 0; x < srcWidth; ++x) {

      // x scale Bresenham
      if ((xt += xq) >= srcWidth) {
	xt -= srcWidth;
	xStep = xp + 1;
      } else {
	xStep = xp;
      }

      // compute the final pixel
      for (i = 0; i < nComps; ++i) {
	// pixBuf[] / yStep
	pix[i] = (pixBuf[x * nComps + i] * d) >> 23;
      }

      // store the pixel
      switch (srcMode) {
      case splashModeMono8:
	for (i = 0; i < xStep; ++i) {
	  *destPtr++ = (Guchar)pix[0];
	}
	break;
      case splashModeRGB8:
	for (i = 0; i < xStep; ++i) {
	  *destPtr++ = (Guchar)pix[0];
	  *destPtr++ = (Guchar)pix[1];
	  *destPtr++ = (Guchar)pix[2];
	}
	break;
#if SPLASH_CMYK
      case splashModeCMYK8:
	for (i = 0; i < xStep; ++i) {
	  *destPtr++ = (Guchar)pix[0];
	  *destPtr++ = (Guchar)pix[1];
	  *destPtr++ = (Guchar)pix[2];
	  *destPtr++ = (Guchar)pix[3];
	}
	break;
#endif
      case splashModeMono1: // mono1 is not allowed
      case splashModeBGR8: // BGR8 is not allowed
      default:
	break;
      }

      // process alpha
      if (srcAlpha) {
	// alphaPixBuf[] / yStep
	alpha = (alphaPixBuf[x] * d) >> 23;
	for (i = 0; i < xStep; ++i) {
	  *destAlphaPtr++ = (Guchar)alpha;
	}
      }
    }
  }

  gfree(alphaPixBuf);
  gfree(alphaLineBuf);
  gfree(pixBuf);
  gfree(lineBuf);
}

void Splash::scaleImageYuXd(SplashImageSource src, void *srcData,
			    SplashColorMode srcMode, int nComps,
			    GBool srcAlpha, int srcWidth, int srcHeight,
			    int scaledWidth, int scaledHeight,
			    SplashBitmap *dest) {
  Guchar *lineBuf, *alphaLineBuf;
  Guint pix[splashMaxColorComps];
  Guint alpha;
  Guchar *destPtr0, *destPtr, *destAlphaPtr0, *destAlphaPtr;
  int yp, yq, xp, xq, yt, y, yStep, xt, x, xStep, xx, xxa, d, d0, d1;
  int i, j;

  // Bresenham parameters for y scale
  yp = scaledHeight / srcHeight;
  yq = scaledHeight % srcHeight;

  // Bresenham parameters for x scale
  xp = srcWidth / scaledWidth;
  xq = srcWidth % scaledWidth;

  // allocate buffers
  lineBuf = (Guchar *)gmallocn(srcWidth, nComps);
  if (srcAlpha) {
    alphaLineBuf = (Guchar *)gmalloc(srcWidth);
  } else {
    alphaLineBuf = NULL;
  }

  // make gcc happy
  pix[0] = pix[1] = pix[2] = 0;
#if SPLASH_CMYK
  pix[3] = 0;
#endif

  // init y scale Bresenham
  yt = 0;

  destPtr0 = dest->data;
  destAlphaPtr0 = dest->alpha;
  for (y = 0; y < srcHeight; ++y) {

    // y scale Bresenham
    if ((yt += yq) >= srcHeight) {
      yt -= srcHeight;
      yStep = yp + 1;
    } else {
      yStep = yp;
    }

    // read row from image
    (*src)(srcData, lineBuf, alphaLineBuf);

    // init x scale Bresenham
    xt = 0;
    d0 = (1 << 23) / xp;
    d1 = (1 << 23) / (xp + 1);

    xx = xxa = 0;
    for (x = 0; x < scaledWidth; ++x) {

      // x scale Bresenham
      if ((xt += xq) >= scaledWidth) {
	xt -= scaledWidth;
	xStep = xp + 1;
	d = d1;
      } else {
	xStep = xp;
	d = d0;
      }

      // compute the final pixel
      for (i = 0; i < nComps; ++i) {
	pix[i] = 0;
      }
      for (i = 0; i < xStep; ++i) {
	for (j = 0; j < nComps; ++j, ++xx) {
	  pix[j] += lineBuf[xx];
	}
      }
      for (i = 0; i < nComps; ++i) {
	// pix[] / xStep
	pix[i] = (pix[i] * d) >> 23;
      }

      // store the pixel
      switch (srcMode) {
      case splashModeMono8:
	for (i = 0; i < yStep; ++i) {
	  destPtr = destPtr0 + (i * scaledWidth + x) * nComps;
	  *destPtr++ = (Guchar)pix[0];
	}
	break;
      case splashModeRGB8:
	for (i = 0; i < yStep; ++i) {
	  destPtr = destPtr0 + (i * scaledWidth + x) * nComps;
	  *destPtr++ = (Guchar)pix[0];
	  *destPtr++ = (Guchar)pix[1];
	  *destPtr++ = (Guchar)pix[2];
	}
	break;
#if SPLASH_CMYK
      case splashModeCMYK8:
	for (i = 0; i < yStep; ++i) {
	  destPtr = destPtr0 + (i * scaledWidth + x) * nComps;
	  *destPtr++ = (Guchar)pix[0];
	  *destPtr++ = (Guchar)pix[1];
	  *destPtr++ = (Guchar)pix[2];
	  *destPtr++ = (Guchar)pix[3];
	}
	break;
#endif
      case splashModeMono1: // mono1 is not allowed
      case splashModeBGR8: // BGR8 is not allowed
      default:
	break;
      }

      // process alpha
      if (srcAlpha) {
	alpha = 0;
	for (i = 0; i < xStep; ++i, ++xxa) {
	  alpha += alphaLineBuf[xxa];
	}
	// alpha / xStep
	alpha = (alpha * d) >> 23;
	for (i = 0; i < yStep; ++i) {
	  destAlphaPtr = destAlphaPtr0 + i * scaledWidth + x;
	  *destAlphaPtr = (Guchar)alpha;
	}
      }
    }

    destPtr0 += yStep * scaledWidth * nComps;
    if (srcAlpha) {
      destAlphaPtr0 += yStep * scaledWidth;
    }
  }

  gfree(alphaLineBuf);
  gfree(lineBuf);
}

void Splash::scaleImageYuXu(SplashImageSource src, void *srcData,
			    SplashColorMode srcMode, int nComps,
			    GBool srcAlpha, int srcWidth, int srcHeight,
			    int scaledWidth, int scaledHeight,
			    SplashBitmap *dest) {
  Guchar *lineBuf, *alphaLineBuf;
  Guchar pix0, pix1, pix2;
#if SPLASH_CMYK
  Guchar pix3;
#endif
  Guchar alpha;
  Guchar *srcPtr, *srcAlphaPtr;
  Guchar *destPtr, *destAlphaPtr;
  int yp, yq, xp, xq, yt, y, yStep, xt, x, xStep;
  int i;

  // Bresenham parameters for y scale
  yp = scaledHeight / srcHeight;
  yq = scaledHeight % srcHeight;

  // Bresenham parameters for x scale
  xp = scaledWidth / srcWidth;
  xq = scaledWidth % srcWidth;

  // allocate buffers
  lineBuf = (Guchar *)gmallocn(srcWidth, nComps);
  if (srcAlpha) {
    alphaLineBuf = (Guchar *)gmalloc(srcWidth);
  } else {
    alphaLineBuf = NULL;
  }

  // init y scale Bresenham
  yt = 0;

  destPtr = dest->data;
  destAlphaPtr = dest->alpha;
  for (y = 0; y < srcHeight; ++y) {

    // y scale Bresenham
    if ((yt += yq) >= srcHeight) {
      yt -= srcHeight;
      yStep = yp + 1;
    } else {
      yStep = yp;
    }

    // read row from image
    (*src)(srcData, lineBuf, alphaLineBuf);

    // init x scale Bresenham
    xt = 0;

    // generate one row
    srcPtr = lineBuf;
    srcAlphaPtr = alphaLineBuf;
    for (x = 0; x < srcWidth; ++x) {

      // x scale Bresenham
      if ((xt += xq) >= srcWidth) {
	xt -= srcWidth;
	xStep = xp + 1;
      } else {
	xStep = xp;
      }

      // duplicate the pixel horizontally
      switch (srcMode) {
      case splashModeMono8:
	pix0 = *srcPtr++;
	for (i = 0; i < xStep; ++i) {
	  *destPtr++ = pix0;
	}
	break;
      case splashModeRGB8:
	pix0 = *srcPtr++;
	pix1 = *srcPtr++;
	pix2 = *srcPtr++;
	for (i = 0; i < xStep; ++i) {
	  *destPtr++ = pix0;
	  *destPtr++ = pix1;
	  *destPtr++ = pix2;
	}
	break;
#if SPLASH_CMYK
      case splashModeCMYK8:
	pix0 = *srcPtr++;
	pix1 = *srcPtr++;
	pix2 = *srcPtr++;
	pix3 = *srcPtr++;
	for (i = 0; i < xStep; ++i) {
	  *destPtr++ = pix0;
	  *destPtr++ = pix1;
	  *destPtr++ = pix2;
	  *destPtr++ = pix3;
	}
	break;
#endif
      case splashModeMono1: // mono1 is not allowed
      case splashModeBGR8: // BGR8 is not allowed
      default:
	break;
      }

      // duplicate the alpha value horizontally
      if (srcAlpha) {
	alpha = *srcAlphaPtr++;
	for (i = 0; i < xStep; ++i) {
	  *destAlphaPtr++ = alpha;
	}
      }
    }

    // duplicate the row vertically
    for (i = 1; i < yStep; ++i) {
      memcpy(destPtr, destPtr - scaledWidth * nComps,
	     scaledWidth * nComps);
      destPtr += scaledWidth * nComps;
    }
    if (srcAlpha) {
      for (i = 1; i < yStep; ++i) {
	memcpy(destAlphaPtr, destAlphaPtr - scaledWidth, scaledWidth);
	destAlphaPtr += scaledWidth;
      }
    }
  }

  gfree(alphaLineBuf);
  gfree(lineBuf);
}

void Splash::scaleImageYuXuI(SplashImageSource src, void *srcData,
			     SplashColorMode srcMode, int nComps,
			     GBool srcAlpha, int srcWidth, int srcHeight,
			     int scaledWidth, int scaledHeight,
			     SplashBitmap *dest) {
  Guchar *lineBuf0, *lineBuf1, *alphaLineBuf0, *alphaLineBuf1, *tBuf;
  Guchar pix[splashMaxColorComps];
  SplashCoord yr, xr, ys, xs, ySrc, xSrc;
  int ySrc0, ySrc1, yBuf, xSrc0, xSrc1, y, x, i;
  Guchar *destPtr, *destAlphaPtr;

  // ratios
  yr = (SplashCoord)srcHeight / (SplashCoord)scaledHeight;
  xr = (SplashCoord)srcWidth / (SplashCoord)scaledWidth;

  // allocate buffers
  lineBuf0 = (Guchar *)gmallocn(scaledWidth, nComps);
  lineBuf1 = (Guchar *)gmallocn(scaledWidth, nComps);
  if (srcAlpha) {
    alphaLineBuf0 = (Guchar *)gmalloc(scaledWidth);
    alphaLineBuf1 = (Guchar *)gmalloc(scaledWidth);
  } else {
    alphaLineBuf0 = NULL;
    alphaLineBuf1 = NULL;
  }

  // read first two rows
  (*src)(srcData, lineBuf0, alphaLineBuf0);
  if (srcHeight > 1) {
    (*src)(srcData, lineBuf1, alphaLineBuf1);
    yBuf = 1;
  } else {
    memcpy(lineBuf1, lineBuf0, srcWidth * nComps);
    if (srcAlpha) {
      memcpy(alphaLineBuf1, alphaLineBuf0, srcWidth);
    }
    yBuf = 0;
  }

  // interpolate first two rows
  for (x = scaledWidth - 1; x >= 0; --x) {
    xSrc = xr * x;
    xSrc0 = splashFloor(xSrc + xr * 0.5 - 0.5);
    xSrc1 = xSrc0 + 1;
    xs = ((SplashCoord)xSrc1 + 0.5) - (xSrc + xr * 0.5);
    if (xSrc0 < 0) {
      xSrc0 = 0;
    }
    if (xSrc1 >= srcWidth) {
      xSrc1 = srcWidth - 1;
    }
    for (i = 0; i < nComps; ++i) {
      lineBuf0[x*nComps+i] = (Guchar)(int)
	    (xs * (int)lineBuf0[xSrc0*nComps+i] +
	     ((SplashCoord)1 - xs) * (int)lineBuf0[xSrc1*nComps+i]);
      lineBuf1[x*nComps+i] = (Guchar)(int)
	    (xs * (int)lineBuf1[xSrc0*nComps+i] +
	     ((SplashCoord)1 - xs) * (int)lineBuf1[xSrc1*nComps+i]);
    }
    if (srcAlpha) {
      alphaLineBuf0[x] = (Guchar)(int)
	    (xs * (int)alphaLineBuf0[xSrc0] +
	     ((SplashCoord)1 - xs) * (int)alphaLineBuf0[xSrc1]);
      alphaLineBuf1[x] = (Guchar)(int)
	    (xs * (int)alphaLineBuf1[xSrc0] +
	     ((SplashCoord)1 - xs) * (int)alphaLineBuf1[xSrc1]);
    }
  }

  // make gcc happy
  pix[0] = pix[1] = pix[2] = 0;
#if SPLASH_CMYK
  pix[3] = 0;
#endif

  destPtr = dest->data;
  destAlphaPtr = dest->alpha;
  for (y = 0; y < scaledHeight; ++y) {

    // compute vertical interpolation parameters
    ySrc = yr * y;
    ySrc0 = splashFloor(ySrc + yr * 0.5 - 0.5);
    ySrc1 = ySrc0 + 1;
    ys = ((SplashCoord)ySrc1 + 0.5) - (ySrc + yr * 0.5);
    if (ySrc0 < 0) {
      ySrc0 = 0;
      ys = 1;
    }
    if (ySrc1 >= srcHeight) {
      ySrc1 = srcHeight - 1;
      ys = 0;
    }

    // read another row (if necessary)
    if (ySrc1 > yBuf) {
      tBuf = lineBuf0;
      lineBuf0 = lineBuf1;
      lineBuf1 = tBuf;
      tBuf = alphaLineBuf0;
      alphaLineBuf0 = alphaLineBuf1;
      alphaLineBuf1 = tBuf;
      (*src)(srcData, lineBuf1, alphaLineBuf1);

      // interpolate the row
      for (x = scaledWidth - 1; x >= 0; --x) {
	xSrc = xr * x;
	xSrc0 = splashFloor(xSrc + xr * 0.5 - 0.5);
	xSrc1 = xSrc0 + 1;
	xs = ((SplashCoord)xSrc1 + 0.5) - (xSrc + xr * 0.5);
	if (xSrc0 < 0) {
	  xSrc0 = 0;
	}
	if (xSrc1 >= srcWidth) {
	  xSrc1 = srcWidth - 1;
	}
	for (i = 0; i < nComps; ++i) {
	  lineBuf1[x*nComps+i] = (Guchar)(int)
	        (xs * (int)lineBuf1[xSrc0*nComps+i] +
		 ((SplashCoord)1 - xs) * (int)lineBuf1[xSrc1*nComps+i]);
	}
	if (srcAlpha) {
	  alphaLineBuf1[x] = (Guchar)(int)
	        (xs * (int)alphaLineBuf1[xSrc0] +
		 ((SplashCoord)1 - xs) * (int)alphaLineBuf1[xSrc1]);
	}
      }

      ++yBuf;
    }

    // do the vertical interpolation
    for (x = 0; x < scaledWidth; ++x) {

      for (i = 0; i < nComps; ++i) {
	pix[i] = (Guchar)(int)
	         (ys * (int)lineBuf0[x*nComps+i] +
		  ((SplashCoord)1 - ys) * (int)lineBuf1[x*nComps+i]);
      }

      // store the pixel
      switch (srcMode) {
      case splashModeMono8:
	*destPtr++ = pix[0];
	break;
      case splashModeRGB8:
	*destPtr++ = pix[0];
	*destPtr++ = pix[1];
	*destPtr++ = pix[2];
	break;
#if SPLASH_CMYK
      case splashModeCMYK8:
	*destPtr++ = pix[0];
	*destPtr++ = pix[1];
	*destPtr++ = pix[2];
	*destPtr++ = pix[3];
	break;
#endif
      case splashModeMono1: // mono1 is not allowed
      case splashModeBGR8: // BGR8 is not allowed
      default:
	break;
      }

      // process alpha
      if (srcAlpha) {
	*destAlphaPtr++ = (Guchar)(int)
	                  (ys * (int)alphaLineBuf0[x] +
			   ((SplashCoord)1 - ys) * (int)alphaLineBuf1[x]);
      }
    }
  }

  gfree(alphaLineBuf1);
  gfree(alphaLineBuf0);
  gfree(lineBuf1);
  gfree(lineBuf0);
}

void Splash::vertFlipImage(SplashBitmap *img, int width, int height,
			   int nComps) {
  Guchar *lineBuf;
  Guchar *p0, *p1;
  int w;

  w = width * nComps;
  lineBuf = (Guchar *)gmalloc(w);
  for (p0 = img->data, p1 = img->data + (height - 1) * (size_t)w;
       p0 < p1;
       p0 += w, p1 -= w) {
    memcpy(lineBuf, p0, w);
    memcpy(p0, p1, w);
    memcpy(p1, lineBuf, w);
  }
  if (img->alpha) {
    for (p0 = img->alpha, p1 = img->alpha + (height - 1) * (size_t)width;
	 p0 < p1;
	 p0 += width, p1 -= width) {
      memcpy(lineBuf, p0, width);
      memcpy(p0, p1, width);
      memcpy(p1, lineBuf, width);
    }
  }
  gfree(lineBuf);
}

void Splash::horizFlipImage(SplashBitmap *img, int width, int height,
			    int nComps) {
  Guchar *lineBuf;
  SplashColorPtr p0, p1, p2;
  int w, x, y, i;

  w = width * nComps;
  lineBuf = (Guchar *)gmalloc(w);
  for (y = 0, p0 = img->data; y < height; ++y, p0 += img->rowSize) {
    memcpy(lineBuf, p0, w);
    p1 = p0;
    p2 = lineBuf + (w - nComps);
    for (x = 0; x < width; ++x) {
      for (i = 0; i < nComps; ++i) {
	p1[i] = p2[i];
      }
      p1 += nComps;
      p2 -= nComps;
    }
  }
  if (img->alpha) {
    for (y = 0, p0 = img->alpha; y < height; ++y, p0 += width) {
      memcpy(lineBuf, p0, width);
      p1 = p0;
      p2 = lineBuf + (width - 1);
      for (x = 0; x < width; ++x) {
	*p1++ = *p2--;
      }
    }
  }
  gfree(lineBuf);
}

void Splash::blitImage(SplashBitmap *src, GBool srcAlpha, int xDest, int yDest,
		       SplashClipResult clipRes) {
  SplashPipe pipe;
  int w, h, x0, y0, x1, y1, y;

  // split the image into clipped and unclipped regions
  w = src->width;
  h = src->height;
  if (clipRes == splashClipAllInside) {
    x0 = 0;
    y0 = 0;
    x1 = w;
    y1 = h;
  } else {
    if (state->clip->getNumPaths()) {
      x0 = x1 = w;
      y0 = y1 = h;
    } else {
      if ((x0 = splashCeil(state->clip->getXMin()) - xDest) < 0) {
	x0 = 0;
      }
      if ((y0 = splashCeil(state->clip->getYMin()) - yDest) < 0) {
	y0 = 0;
      }
      if ((x1 = splashFloor(state->clip->getXMax()) - xDest) > w) {
	x1 = w;
      }
      if (x1 < x0) {
	x1 = x0;
      }
      if ((y1 = splashFloor(state->clip->getYMax()) - yDest) > h) {
	y1 = h; 
     }
      if (y1 < y0) {
	y1 = y0;
      }
    }
  }

  // draw the unclipped region
  if (x0 < w && y0 < h && x0 < x1 && y0 < y1) {
    pipeInit(&pipe, NULL,
	     (Guchar)splashRound(state->fillAlpha * 255),
	     srcAlpha, gFalse);
    if (srcAlpha) {
      for (y = y0; y < y1; ++y) {
	(this->*pipe.run)(&pipe, xDest + x0, xDest + x1 - 1, yDest + y,
			  src->alpha + y * src->alphaRowSize + x0,
			  src->data + y * src->rowSize + x0 * bitmapComps);
      }
    } else {
      for (y = y0; y < y1; ++y) {
	(this->*pipe.run)(&pipe, xDest + x0, xDest + x1 - 1, yDest + y,
			  NULL,
			  src->data + y * src->getRowSize() +
			    x0 * bitmapComps);
      }
    }
  }

  // draw the clipped regions
  if (y0 > 0) {
    blitImageClipped(src, srcAlpha, 0, 0, xDest, yDest, w, y0);
  }
  if (y1 < h) {
    blitImageClipped(src, srcAlpha, 0, y1, xDest, yDest + y1, w, h - y1);
  }
  if (x0 > 0 && y0 < y1) {
    blitImageClipped(src, srcAlpha, 0, y0, xDest, yDest + y0, x0, y1 - y0);
  }
  if (x1 < w && y0 < y1) {
    blitImageClipped(src, srcAlpha, x1, y0, xDest + x1, yDest + y0,
		     w - x1, y1 - y0);
  }
}

void Splash::blitImageClipped(SplashBitmap *src, GBool srcAlpha,
			      int xSrc, int ySrc, int xDest, int yDest,
			      int w, int h) {
  SplashPipe pipe;
  int y;

  if (xDest < 0) {
    xSrc -= xDest;
    w += xDest;
    xDest = 0;
  }
  if (xDest + w > bitmap->width) {
    w = bitmap->width - xDest;
  }
  if (yDest < 0) {
    ySrc -= yDest;
    h += yDest;
    yDest = 0;
  }
  if (yDest + h > bitmap->height) {
    h = bitmap->height - yDest;
  }
  if (w <= 0 || h <= 0) {
    return;
  }

  pipeInit(&pipe, NULL,
	   (Guchar)splashRound(state->fillAlpha * 255),
	   gTrue, gFalse);
  if (srcAlpha) {
    for (y = 0; y < h; ++y) {
      memcpy(scanBuf + xDest,
	     src->alpha + (ySrc + y) * src->alphaRowSize + xSrc,
	     w);
      if (vectorAntialias) {
	state->clip->clipSpan(scanBuf, yDest + y, xDest, xDest + w - 1,
			      state->strokeAdjust);
      } else {
	state->clip->clipSpanBinary(scanBuf, yDest + y, xDest, xDest + w - 1,
				    state->strokeAdjust);
      }
      (this->*pipe.run)(&pipe, xDest, xDest + w - 1, yDest + y,
			scanBuf + xDest,
			src->data + (ySrc + y) * src->rowSize +
			  xSrc * bitmapComps);
    }
  } else {
    for (y = 0; y < h; ++y) {
      memset(scanBuf + xDest, 0xff, w);
      if (vectorAntialias) {
	state->clip->clipSpan(scanBuf, yDest + y, xDest, xDest + w - 1,
			      state->strokeAdjust);
      } else {
	state->clip->clipSpanBinary(scanBuf, yDest + y, xDest, xDest + w - 1,
				    state->strokeAdjust);
      }
      (this->*pipe.run)(&pipe, xDest, xDest + w - 1, yDest + y,
			scanBuf + xDest,
			src->data + (ySrc + y) * src->rowSize +
			  xSrc * bitmapComps);
    }
  }
}

SplashError Splash::composite(SplashBitmap *src, int xSrc, int ySrc,
			      int xDest, int yDest, int w, int h,
			      GBool noClip, GBool nonIsolated) {
  SplashPipe pipe;
  Guchar *mono1Ptr, *lineBuf, *linePtr;
  Guchar mono1Mask, b;
  int x0, x1, x, y0, y1, y, t;

  if (!(src->mode == bitmap->mode ||
	(src->mode == splashModeMono8 && bitmap->mode == splashModeMono1) ||
	(src->mode == splashModeRGB8 && bitmap->mode == splashModeBGR8))) {
    return splashErrModeMismatch;
  }

  pipeInit(&pipe, NULL,
	   (Guchar)splashRound(state->fillAlpha * 255),
	   !noClip || src->alpha != NULL, nonIsolated);
  if (src->mode == splashModeMono1) {
    // in mono1 mode, pipeRun expects the source to be in mono8
    // format, so we need to extract the source color values into
    // scanBuf, expanding them from mono1 to mono8
    if (noClip) {
      if (src->alpha) {
	for (y = 0; y < h; ++y) {
	  mono1Ptr = src->data + (ySrc + y) * src->rowSize + (xSrc >> 3);
	  mono1Mask = (Guchar)(0x80 >> (xSrc & 7));
	  for (x = 0; x < w; ++x) {
	    scanBuf[x] = (*mono1Ptr & mono1Mask) ? 0xff : 0x00;
	    mono1Ptr += mono1Mask & 1;
	    mono1Mask = (Guchar)((mono1Mask << 7) | (mono1Mask >> 1));
	  }
	  // this uses shape instead of alpha, which isn't technically
	  // correct, but works out the same
	  (this->*pipe.run)(&pipe, xDest, xDest + w - 1, yDest + y,
			    src->alpha +
			      (ySrc + y) * src->alphaRowSize + xSrc,
			    scanBuf);
	}
      } else {
	for (y = 0; y < h; ++y) {
	  mono1Ptr = src->data + (ySrc + y) * src->rowSize + (xSrc >> 3);
	  mono1Mask = (Guchar)(0x80 >> (xSrc & 7));
	  for (x = 0; x < w; ++x) {
	    scanBuf[x] = (*mono1Ptr & mono1Mask) ? 0xff : 0x00;
	    mono1Ptr += mono1Mask & 1;
	    mono1Mask = (Guchar)((mono1Mask << 7) | (mono1Mask >> 1));
	  }
	  (this->*pipe.run)(&pipe, xDest, xDest + w - 1, yDest + y,
			    NULL,
			    scanBuf);
	}
      }
    } else {
      x0 = xDest;
      if ((t = state->clip->getXMinI(state->strokeAdjust)) > x0) {
	x0 = t;
      }
      x1 = xDest + w;
      if ((t = state->clip->getXMaxI(state->strokeAdjust) + 1) < x1) {
	x1 = t;
      }
      y0 = yDest;
      if ((t = state->clip->getYMinI(state->strokeAdjust)) > y0) {
	y0 = t;
      }
      y1 = yDest + h;
      if ((t = state->clip->getYMaxI(state->strokeAdjust) + 1) < y1) {
	y1 = t;
      }
      if (x0 < x1 && y0 < y1) {
	if (src->alpha) {
	  for (y = y0; y < y1; ++y) {
	    mono1Ptr = src->data
	               + (ySrc + y - yDest) * src->rowSize
	               + ((xSrc + x0 - xDest) >> 3);
	    mono1Mask = (Guchar)(0x80 >> ((xSrc + x0 - xDest) & 7));
	    for (x = x0; x < x1; ++x) {
	      scanBuf[x] = (*mono1Ptr & mono1Mask) ? 0xff : 0x00;
	      mono1Ptr += mono1Mask & 1;
	      mono1Mask = (Guchar)((mono1Mask << 7) | (mono1Mask >> 1));
	    }
	    memcpy(scanBuf2 + x0,
		   src->alpha + (ySrc + y - yDest) * src->alphaRowSize + 
		     (xSrc + x0 - xDest),
		   x1 - x0);
	    if (!state->clip->clipSpanBinary(scanBuf2, y, x0, x1 - 1,
					     state->strokeAdjust)) {
	      continue;
	    }
	    // this uses shape instead of alpha, which isn't technically
	    // correct, but works out the same
	    (this->*pipe.run)(&pipe, x0, x1 - 1, y,
			      scanBuf2 + x0,
			      scanBuf + x0);
	  }
	} else {
	  for (y = y0; y < y1; ++y) {
	    mono1Ptr = src->data
	               + (ySrc + y - yDest) * src->rowSize
	               + ((xSrc + x0 - xDest) >> 3);
	    mono1Mask = (Guchar)(0x80 >> ((xSrc + x0 - xDest) & 7));
	    for (x = x0; x < x1; ++x) {
	      scanBuf[x] = (*mono1Ptr & mono1Mask) ? 0xff : 0x00;
	      mono1Ptr += mono1Mask & 1;
	      mono1Mask = (Guchar)((mono1Mask << 7) | (mono1Mask >> 1));
	    }
	    memset(scanBuf2 + x0, 0xff, x1 - x0);
	    if (!state->clip->clipSpanBinary(scanBuf2, y, x0, x1 - 1,
					     state->strokeAdjust)) {
	      continue;
	    }
	    (this->*pipe.run)(&pipe, x0, x1 - 1, y,
			      scanBuf2 + x0,
			      scanBuf + x0);
	  }
	}
      }
    }

  } else if (src->mode == splashModeBGR8) {
    // in BGR8 mode, pipeRun expects the source to be in RGB8 format,
    // so we need to swap bytes
    lineBuf = (Guchar *)gmallocn(w, 3);
    if (noClip) {
      if (src->alpha) {
	for (y = 0; y < h; ++y) {
	  memcpy(lineBuf,
		 src->data + (ySrc + y) * src->rowSize + xSrc * 3,
		 w * 3);
	  for (x = 0, linePtr = lineBuf; x < w; ++x, linePtr += 3) {
	    b = linePtr[0];
	    linePtr[0] = linePtr[2];
	    linePtr[2] = b;
	  }
	  // this uses shape instead of alpha, which isn't technically
	  // correct, but works out the same
	  (this->*pipe.run)(&pipe, xDest, xDest + w - 1, yDest + y,
			    src->alpha +
			      (ySrc + y) * src->alphaRowSize + xSrc,
			    lineBuf);
	}
      } else {
	for (y = 0; y < h; ++y) {
	  memcpy(lineBuf,
		 src->data + (ySrc + y) * src->rowSize + xSrc * 3,
		 w * 3);
	  for (x = 0, linePtr = lineBuf; x < w; ++x, linePtr += 3) {
	    b = linePtr[0];
	    linePtr[0] = linePtr[2];
	    linePtr[2] = b;
	  }
	  (this->*pipe.run)(&pipe, xDest, xDest + w - 1, yDest + y,
			    NULL, lineBuf);
	}
      }
    } else {
      x0 = xDest;
      if ((t = state->clip->getXMinI(state->strokeAdjust)) > x0) {
	x0 = t;
      }
      x1 = xDest + w;
      if ((t = state->clip->getXMaxI(state->strokeAdjust) + 1) < x1) {
	x1 = t;
      }
      y0 = yDest;
      if ((t = state->clip->getYMinI(state->strokeAdjust)) > y0) {
	y0 = t;
      }
      y1 = yDest + h;
      if ((t = state->clip->getYMaxI(state->strokeAdjust) + 1) < y1) {
	y1 = t;
      }
      if (x0 < x1 && y0 < y1) {
	if (src->alpha) {
	  for (y = y0; y < y1; ++y) {
	    memcpy(scanBuf + x0,
		   src->alpha + (ySrc + y - yDest) * src->alphaRowSize + 
		     (xSrc + x0 - xDest),
		   x1 - x0);
	    state->clip->clipSpan(scanBuf, y, x0, x1 - 1, state->strokeAdjust);
	    memcpy(lineBuf,
		   src->data +
		     (ySrc + y - yDest) * src->rowSize +
		     (xSrc + x0 - xDest) * 3,
		   (x1 - x0) * 3);
	    for (x = 0, linePtr = lineBuf; x < x1 - x0; ++x, linePtr += 3) {
	      b = linePtr[0];
	      linePtr[0] = linePtr[2];
	      linePtr[2] = b;
	    }
	    // this uses shape instead of alpha, which isn't technically
	    // correct, but works out the same
	    (this->*pipe.run)(&pipe, x0, x1 - 1, y,
			      scanBuf + x0, lineBuf);
	  }
	} else {
	  for (y = y0; y < y1; ++y) {
	    memset(scanBuf + x0, 0xff, x1 - x0);
	    state->clip->clipSpan(scanBuf, y, x0, x1 - 1, state->strokeAdjust);
	    memcpy(lineBuf,
		   src->data +
		     (ySrc + y - yDest) * src->rowSize +
		     (xSrc + x0 - xDest) * 3,
		   (x1 - x0) * 3);
	    for (x = 0, linePtr = lineBuf; x < x1 - x0; ++x, linePtr += 3) {
	      b = linePtr[0];
	      linePtr[0] = linePtr[2];
	      linePtr[2] = b;
	    }
	    (this->*pipe.run)(&pipe, x0, x1 - 1, yDest + y,
			      scanBuf + x0,
			      src->data +
			        (ySrc + y - yDest) * src->rowSize +
			        (xSrc + x0 - xDest) * bitmapComps);
	  }
	}
      }
    }
    gfree(lineBuf);

  } else { // src->mode not mono1 or BGR8
    if (noClip) {
      if (src->alpha) {
	for (y = 0; y < h; ++y) {
	  // this uses shape instead of alpha, which isn't technically
	  // correct, but works out the same
	  (this->*pipe.run)(&pipe, xDest, xDest + w - 1, yDest + y,
			    src->alpha +
			      (ySrc + y) * src->alphaRowSize + xSrc,
			    src->data + (ySrc + y) * src->rowSize +
			      xSrc * bitmapComps);
	}
      } else {
	for (y = 0; y < h; ++y) {
	  (this->*pipe.run)(&pipe, xDest, xDest + w - 1, yDest + y,
			    NULL,
			    src->data + (ySrc + y) * src->rowSize +
			      xSrc * bitmapComps);
	}
      }
    } else {
      x0 = xDest;
      if ((t = state->clip->getXMinI(state->strokeAdjust)) > x0) {
	x0 = t;
      }
      x1 = xDest + w;
      if ((t = state->clip->getXMaxI(state->strokeAdjust) + 1) < x1) {
	x1 = t;
      }
      y0 = yDest;
      if ((t = state->clip->getYMinI(state->strokeAdjust)) > y0) {
	y0 = t;
      }
      y1 = yDest + h;
      if ((t = state->clip->getYMaxI(state->strokeAdjust) + 1) < y1) {
	y1 = t;
      }
      if (x0 < x1 && y0 < y1) {
	if (src->alpha) {
	  for (y = y0; y < y1; ++y) {
	    memcpy(scanBuf + x0,
		   src->alpha + (ySrc + y - yDest) * src->alphaRowSize + 
		     (xSrc + x0 - xDest),
		   x1 - x0);
	    state->clip->clipSpan(scanBuf, y, x0, x1 - 1, state->strokeAdjust);
	    // this uses shape instead of alpha, which isn't technically
	    // correct, but works out the same
	    (this->*pipe.run)(&pipe, x0, x1 - 1, y,
			      scanBuf + x0,
			      src->data +
			        (ySrc + y - yDest) * src->rowSize +
			        (xSrc + x0 - xDest) * bitmapComps);
	  }
	} else {
	  for (y = y0; y < y1; ++y) {
	    memset(scanBuf + x0, 0xff, x1 - x0);
	    state->clip->clipSpan(scanBuf, y, x0, x1 - 1, state->strokeAdjust);
	    (this->*pipe.run)(&pipe, x0, x1 - 1, yDest + y,
			      scanBuf + x0,
			      src->data +
			        (ySrc + y - yDest) * src->rowSize +
			        (xSrc + x0 - xDest) * bitmapComps);
	  }
	}
      }
    }
  }

  return splashOk;
}

void Splash::compositeBackground(SplashColorPtr color) {
  SplashColorPtr p;
  Guchar *q;
  Guchar alpha, alpha1, c, color0, color1, color2, mask;
#if SPLASH_CMYK
  Guchar color3;
#endif
  int x, y;

  switch (bitmap->mode) {
  case splashModeMono1:
    color0 = color[0];
    for (y = 0; y < bitmap->height; ++y) {
      p = &bitmap->data[y * bitmap->rowSize];
      q = &bitmap->alpha[y * bitmap->alphaRowSize];
      mask = 0x80;
      for (x = 0; x < bitmap->width; ++x) {
	alpha = *q++;
	if (alpha == 0) {
	  if (color0 & 0x80) {
	    *p |= mask;
	  } else {
	    *p &= (Guchar)~mask;
	  }
	} else if (alpha != 255) {
	  alpha1 = (Guchar)(255 - alpha);
	  c = (*p & mask) ? 0xff : 0x00;
	  c = div255(alpha1 * color0 + alpha * c);
	  if (c & 0x80) {
	    *p |= mask;
	  } else {
	    *p &= (Guchar)~mask;
	  }
	}
	if (!(mask = (Guchar)(mask >> 1))) {
	  mask = 0x80;
	  ++p;
	}
      }
    }
    break;
  case splashModeMono8:
    color0 = color[0];
    for (y = 0; y < bitmap->height; ++y) {
      p = &bitmap->data[y * bitmap->rowSize];
      q = &bitmap->alpha[y * bitmap->alphaRowSize];
      for (x = 0; x < bitmap->width; ++x) {
	alpha = *q++;
	if (alpha == 0) {
	  p[0] = color0;
	} else if (alpha != 255) {
	  alpha1 = (Guchar)(255 - alpha);
	  p[0] = div255(alpha1 * color0 + alpha * p[0]);
	}
	++p;
      }
    }
    break;
  case splashModeRGB8:
  case splashModeBGR8:
    color0 = color[0];
    color1 = color[1];
    color2 = color[2];
    for (y = 0; y < bitmap->height; ++y) {
      p = &bitmap->data[y * bitmap->rowSize];
      q = &bitmap->alpha[y * bitmap->alphaRowSize];
      for (x = 0; x < bitmap->width; ++x) {
	alpha = *q++;
	if (alpha == 0) {
	  p[0] = color0;
	  p[1] = color1;
	  p[2] = color2;
	} else if (alpha != 255) {
	  alpha1 = (Guchar)(255 - alpha);
	  p[0] = div255(alpha1 * color0 + alpha * p[0]);
	  p[1] = div255(alpha1 * color1 + alpha * p[1]);
	  p[2] = div255(alpha1 * color2 + alpha * p[2]);
	}
	p += 3;
      }
    }
    break;
#if SPLASH_CMYK
  case splashModeCMYK8:
    color0 = color[0];
    color1 = color[1];
    color2 = color[2];
    color3 = color[3];
    for (y = 0; y < bitmap->height; ++y) {
      p = &bitmap->data[y * bitmap->rowSize];
      q = &bitmap->alpha[y * bitmap->alphaRowSize];
      for (x = 0; x < bitmap->width; ++x) {
	alpha = *q++;
	if (alpha == 0) {
	  p[0] = color0;
	  p[1] = color1;
	  p[2] = color2;
	  p[3] = color3;
	} else if (alpha != 255) {
	  alpha1 = (Guchar)(255 - alpha);
	  p[0] = div255(alpha1 * color0 + alpha * p[0]);
	  p[1] = div255(alpha1 * color1 + alpha * p[1]);
	  p[2] = div255(alpha1 * color2 + alpha * p[2]);
	  p[3] = div255(alpha1 * color3 + alpha * p[3]);
	}
	p += 4;
      }
    }
    break;
#endif
  }
  memset(bitmap->alpha, 255, bitmap->alphaRowSize * bitmap->height);
}

SplashError Splash::blitTransparent(SplashBitmap *src, int xSrc, int ySrc,
				    int xDest, int yDest, int w, int h) {
  SplashColorPtr p, q;
  Guchar mask, srcMask;
  int x, y;

  if (src->mode != bitmap->mode) {
    return splashErrModeMismatch;
  }

  switch (bitmap->mode) {
  case splashModeMono1:
    for (y = 0; y < h; ++y) {
      p = &bitmap->data[(yDest + y) * bitmap->rowSize + (xDest >> 3)];
      mask = (Guchar)(0x80 >> (xDest & 7));
      q = &src->data[(ySrc + y) * src->rowSize + (xSrc >> 3)];
      srcMask = (Guchar)(0x80 >> (xSrc & 7));
      for (x = 0; x < w; ++x) {
	if (*q & srcMask) {
	  *p |= mask;
	} else {
	  *p &= (Guchar)~mask;
	}
	if (!(mask = (Guchar)(mask >> 1))) {
	  mask = 0x80;
	  ++p;
	}
	if (!(srcMask = (Guchar)(srcMask >> 1))) {
	  srcMask = 0x80;
	  ++q;
	}
      }
    }
    break;
  case splashModeMono8:
    for (y = 0; y < h; ++y) {
      p = &bitmap->data[(yDest + y) * bitmap->rowSize + xDest];
      q = &src->data[(ySrc + y) * src->rowSize + xSrc];
      memcpy(p, q, w);
    }
    break;
  case splashModeRGB8:
  case splashModeBGR8:
    for (y = 0; y < h; ++y) {
      p = &bitmap->data[(yDest + y) * bitmap->rowSize + 3 * xDest];
      q = &src->data[(ySrc + y) * src->rowSize + 3 * xSrc];
      memcpy(p, q, 3 * w);
    }
    break;
#if SPLASH_CMYK
  case splashModeCMYK8:
    for (y = 0; y < h; ++y) {
      p = &bitmap->data[(yDest + y) * bitmap->rowSize + 4 * xDest];
      q = &src->data[(ySrc + y) * src->rowSize + 4 * xSrc];
      memcpy(p, q, 4 * w);
    }
    break;
#endif
  }

  if (bitmap->alpha) {
    for (y = 0; y < h; ++y) {
      q = &bitmap->alpha[(yDest + y) * bitmap->alphaRowSize + xDest];
      memset(q, 0, w);
    }
  }

  return splashOk;
}

SplashError Splash::blitCorrectedAlpha(SplashBitmap *dest, int xSrc, int ySrc,
				       int xDest, int yDest, int w, int h) {
  SplashColorPtr p, q;
  Guchar *alpha0Ptr;
  Guchar alpha0, aSrc, mask, srcMask;
  int x, y;

  if (bitmap->mode != dest->mode ||
      !bitmap->alpha ||
      !dest->alpha ||
      !groupBackBitmap) {
    return splashErrModeMismatch;
  }

  switch (bitmap->mode) {
  case splashModeMono1:
    for (y = 0; y < h; ++y) {
      p = &dest->data[(yDest + y) * dest->rowSize + (xDest >> 3)];
      mask = (Guchar)(0x80 >> (xDest & 7));
      q = &bitmap->data[(ySrc + y) * bitmap->rowSize + (xSrc >> 3)];
      srcMask = (Guchar)(0x80 >> (xSrc & 7));
      for (x = 0; x < w; ++x) {
	if (*q & srcMask) {
	  *p |= mask;
	} else {
	  *p &= (Guchar)~mask;
	}
	if (!(mask = (Guchar)(mask >> 1))) {
	  mask = 0x80;
	  ++p;
	}
	if (!(srcMask = (Guchar)(srcMask >> 1))) {
	  srcMask = 0x80;
	  ++q;
	}
      }
    }
    break;
  case splashModeMono8:
    for (y = 0; y < h; ++y) {
      p = &dest->data[(yDest + y) * dest->rowSize + xDest];
      q = &bitmap->data[(ySrc + y) * bitmap->rowSize + xSrc];
      memcpy(p, q, w);
    }
    break;
  case splashModeRGB8:
  case splashModeBGR8:
    for (y = 0; y < h; ++y) {
      p = &dest->data[(yDest + y) * dest->rowSize + 3 * xDest];
      q = &bitmap->data[(ySrc + y) * bitmap->rowSize + 3 * xSrc];
      memcpy(p, q, 3 * w);
    }
    break;
#if SPLASH_CMYK
  case splashModeCMYK8:
    for (y = 0; y < h; ++y) {
      p = &dest->data[(yDest + y) * dest->rowSize + 4 * xDest];
      q = &bitmap->data[(ySrc + y) * bitmap->rowSize + 4 * xSrc];
      memcpy(p, q, 4 * w);
    }
    break;
#endif
  }

  for (y = 0; y < h; ++y) {
    p = &dest->alpha[(yDest + y) * dest->alphaRowSize + xDest];
    q = &bitmap->alpha[(ySrc + y) * bitmap->alphaRowSize + xSrc];
    alpha0Ptr = &groupBackBitmap->alpha[(groupBackY + ySrc + y)
					  * groupBackBitmap->alphaRowSize +
					(groupBackX + xSrc)];
    for (x = 0; x < w; ++x) {
      alpha0 = *alpha0Ptr++;
      aSrc = *q++;
      *p++ = (Guchar)(alpha0 + aSrc - div255(alpha0 * aSrc));
    }
  }

  return splashOk;
}

SplashPath *Splash::makeStrokePath(SplashPath *path, SplashCoord w,
				   int lineCap, int lineJoin,
				   GBool flatten) {
  SplashPath *pathIn, *dashPath, *pathOut;
  SplashCoord d, dx, dy, wdx, wdy, dxNext, dyNext, wdxNext, wdyNext;
  SplashCoord crossprod, dotprod, miter, m;
  SplashCoord angle, angleNext, dAngle, xc, yc;
  SplashCoord dxJoin, dyJoin, dJoin, kappa;
  SplashCoord cx1, cy1, cx2, cy2, cx3, cy3, cx4, cy4;
  GBool first, last, closed;
  int subpathStart0, subpathStart1, seg, i0, i1, j0, j1, k0, k1;
  int left0, left1, left2, right0, right1, right2, join0, join1, join2;
  int leftFirst, rightFirst, firstPt;

  pathOut = new SplashPath();

  if (path->length == 0) {
    return pathOut;
  }

  if (flatten) {
    pathIn = flattenPath(path, state->matrix, state->flatness);
    if (state->lineDashLength > 0) {
      dashPath = makeDashedPath(pathIn);
      delete pathIn;
      pathIn = dashPath;
      if (pathIn->length == 0) {
	delete pathIn;
	return pathOut;
      }
    }
  } else {
    pathIn = path;
  }

  subpathStart0 = subpathStart1 = 0; // make gcc happy
  seg = 0; // make gcc happy
  closed = gFalse; // make gcc happy
  left0 = left1 = right0 = right1 = join0 = join1 = 0; // make gcc happy
  leftFirst = rightFirst = firstPt = 0; // make gcc happy

  i0 = 0;
  for (i1 = i0;
       !(pathIn->flags[i1] & splashPathLast) &&
	 i1 + 1 < pathIn->length &&
	 pathIn->pts[i1+1].x == pathIn->pts[i1].x &&
	 pathIn->pts[i1+1].y == pathIn->pts[i1].y;
       ++i1) ;

  while (i1 < pathIn->length) {
    if ((first = pathIn->flags[i0] & splashPathFirst)) {
      subpathStart0 = i0;
      subpathStart1 = i1;
      seg = 0;
      closed = pathIn->flags[i0] & splashPathClosed;
    }
    j0 = i1 + 1;
    if (j0 < pathIn->length) {
      for (j1 = j0;
	   !(pathIn->flags[j1] & splashPathLast) &&
	     j1 + 1 < pathIn->length &&
	     pathIn->pts[j1+1].x == pathIn->pts[j1].x &&
	     pathIn->pts[j1+1].y == pathIn->pts[j1].y;
	   ++j1) ;
    } else {
      j1 = j0;
    }
    if (pathIn->flags[i1] & splashPathLast) {
      if (first && lineCap == splashLineCapRound) {
	// special case: zero-length subpath with round line caps -->
	// draw a circle
	pathOut->moveTo(pathIn->pts[i0].x + (SplashCoord)0.5 * w,
			pathIn->pts[i0].y);
	pathOut->curveTo(pathIn->pts[i0].x + (SplashCoord)0.5 * w,
			 pathIn->pts[i0].y + bezierCircle2 * w,
			 pathIn->pts[i0].x + bezierCircle2 * w,
			 pathIn->pts[i0].y + (SplashCoord)0.5 * w,
			 pathIn->pts[i0].x,
			 pathIn->pts[i0].y + (SplashCoord)0.5 * w);
	pathOut->curveTo(pathIn->pts[i0].x - bezierCircle2 * w,
			 pathIn->pts[i0].y + (SplashCoord)0.5 * w,
			 pathIn->pts[i0].x - (SplashCoord)0.5 * w,
			 pathIn->pts[i0].y + bezierCircle2 * w,
			 pathIn->pts[i0].x - (SplashCoord)0.5 * w,
			 pathIn->pts[i0].y);
	pathOut->curveTo(pathIn->pts[i0].x - (SplashCoord)0.5 * w,
			 pathIn->pts[i0].y - bezierCircle2 * w,
			 pathIn->pts[i0].x - bezierCircle2 * w,
			 pathIn->pts[i0].y - (SplashCoord)0.5 * w,
			 pathIn->pts[i0].x,
			 pathIn->pts[i0].y - (SplashCoord)0.5 * w);
	pathOut->curveTo(pathIn->pts[i0].x + bezierCircle2 * w,
			 pathIn->pts[i0].y - (SplashCoord)0.5 * w,
			 pathIn->pts[i0].x + (SplashCoord)0.5 * w,
			 pathIn->pts[i0].y - bezierCircle2 * w,
			 pathIn->pts[i0].x + (SplashCoord)0.5 * w,
			 pathIn->pts[i0].y);
	pathOut->close();
      }
      i0 = j0;
      i1 = j1;
      continue;
    }
    last = pathIn->flags[j1] & splashPathLast;
    if (last) {
      k0 = subpathStart1 + 1;
    } else {
      k0 = j1 + 1;
    }
    for (k1 = k0;
	 !(pathIn->flags[k1] & splashPathLast) &&
	   k1 + 1 < pathIn->length &&
	   pathIn->pts[k1+1].x == pathIn->pts[k1].x &&
	   pathIn->pts[k1+1].y == pathIn->pts[k1].y;
	 ++k1) ;

    // compute the deltas for segment (i1, j0)
#if USE_FIXEDPOINT
    // the 1/d value can be small, which introduces significant
    // inaccuracies in fixed point mode
    d = splashDist(pathIn->pts[i1].x, pathIn->pts[i1].y,
		   pathIn->pts[j0].x, pathIn->pts[j0].y);
    dx = (pathIn->pts[j0].x - pathIn->pts[i1].x) / d;
    dy = (pathIn->pts[j0].y - pathIn->pts[i1].y) / d;
#else
    d = (SplashCoord)1 / splashDist(pathIn->pts[i1].x, pathIn->pts[i1].y,
				    pathIn->pts[j0].x, pathIn->pts[j0].y);
    dx = d * (pathIn->pts[j0].x - pathIn->pts[i1].x);
    dy = d * (pathIn->pts[j0].y - pathIn->pts[i1].y);
#endif
    wdx = (SplashCoord)0.5 * w * dx;
    wdy = (SplashCoord)0.5 * w * dy;

    // draw the start cap
    if (i0 == subpathStart0) {
      firstPt = pathOut->length;
    }
    if (first && !closed) {
      switch (lineCap) {
      case splashLineCapButt:
	pathOut->moveTo(pathIn->pts[i0].x - wdy, pathIn->pts[i0].y + wdx);
	pathOut->lineTo(pathIn->pts[i0].x + wdy, pathIn->pts[i0].y - wdx);
	break;
      case splashLineCapRound:
	pathOut->moveTo(pathIn->pts[i0].x - wdy, pathIn->pts[i0].y + wdx);
	pathOut->curveTo(pathIn->pts[i0].x - wdy - bezierCircle * wdx,
			 pathIn->pts[i0].y + wdx - bezierCircle * wdy,
			 pathIn->pts[i0].x - wdx - bezierCircle * wdy,
			 pathIn->pts[i0].y - wdy + bezierCircle * wdx,
			 pathIn->pts[i0].x - wdx,
			 pathIn->pts[i0].y - wdy);
	pathOut->curveTo(pathIn->pts[i0].x - wdx + bezierCircle * wdy,
			 pathIn->pts[i0].y - wdy - bezierCircle * wdx,
			 pathIn->pts[i0].x + wdy - bezierCircle * wdx,
			 pathIn->pts[i0].y - wdx - bezierCircle * wdy,
			 pathIn->pts[i0].x + wdy,
			 pathIn->pts[i0].y - wdx);
	break;
      case splashLineCapProjecting:
	pathOut->moveTo(pathIn->pts[i0].x - wdx - wdy,
			pathIn->pts[i0].y + wdx - wdy);
	pathOut->lineTo(pathIn->pts[i0].x - wdx + wdy,
			pathIn->pts[i0].y - wdx - wdy);
	break;
      }
    } else {
      pathOut->moveTo(pathIn->pts[i0].x - wdy, pathIn->pts[i0].y + wdx);
      pathOut->lineTo(pathIn->pts[i0].x + wdy, pathIn->pts[i0].y - wdx);
    }

    // draw the left side of the segment rectangle and the end cap
    left2 = pathOut->length - 1;
    if (last && !closed) {
      switch (lineCap) {
      case splashLineCapButt:
	pathOut->lineTo(pathIn->pts[j0].x + wdy, pathIn->pts[j0].y - wdx);
	pathOut->lineTo(pathIn->pts[j0].x - wdy, pathIn->pts[j0].y + wdx);
	break;
      case splashLineCapRound:
	pathOut->lineTo(pathIn->pts[j0].x + wdy, pathIn->pts[j0].y - wdx);
	pathOut->curveTo(pathIn->pts[j0].x + wdy + bezierCircle * wdx,
			 pathIn->pts[j0].y - wdx + bezierCircle * wdy,
			 pathIn->pts[j0].x + wdx + bezierCircle * wdy,
			 pathIn->pts[j0].y + wdy - bezierCircle * wdx,
			 pathIn->pts[j0].x + wdx,
			 pathIn->pts[j0].y + wdy);
	pathOut->curveTo(pathIn->pts[j0].x + wdx - bezierCircle * wdy,
			 pathIn->pts[j0].y + wdy + bezierCircle * wdx,
			 pathIn->pts[j0].x - wdy + bezierCircle * wdx,
			 pathIn->pts[j0].y + wdx + bezierCircle * wdy,
			 pathIn->pts[j0].x - wdy,
			 pathIn->pts[j0].y + wdx);
	break;
      case splashLineCapProjecting:
	pathOut->lineTo(pathIn->pts[j0].x + wdy + wdx,
			pathIn->pts[j0].y - wdx + wdy);
	pathOut->lineTo(pathIn->pts[j0].x - wdy + wdx,
			pathIn->pts[j0].y + wdx + wdy);
	break;
      }
    } else {
      pathOut->lineTo(pathIn->pts[j0].x + wdy, pathIn->pts[j0].y - wdx);
      pathOut->lineTo(pathIn->pts[j0].x - wdy, pathIn->pts[j0].y + wdx);
    }

    // draw the right side of the segment rectangle
    // (NB: if stroke adjustment is enabled, the closepath operation MUST
    // add a segment because this segment is used for a hint)
    right2 = pathOut->length - 1;
    pathOut->close(state->strokeAdjust != splashStrokeAdjustOff);

    // draw the join
    join2 = pathOut->length;
    if (!last || closed) {

      // compute the deltas for segment (j1, k0)
#if USE_FIXEDPOINT
      // the 1/d value can be small, which introduces significant
      // inaccuracies in fixed point mode
      d = splashDist(pathIn->pts[j1].x, pathIn->pts[j1].y,
		     pathIn->pts[k0].x, pathIn->pts[k0].y);
      dxNext = (pathIn->pts[k0].x - pathIn->pts[j1].x) / d;
      dyNext = (pathIn->pts[k0].y - pathIn->pts[j1].y) / d;
#else
      d = (SplashCoord)1 / splashDist(pathIn->pts[j1].x, pathIn->pts[j1].y,
				      pathIn->pts[k0].x, pathIn->pts[k0].y);
      dxNext = d * (pathIn->pts[k0].x - pathIn->pts[j1].x);
      dyNext = d * (pathIn->pts[k0].y - pathIn->pts[j1].y);
#endif
      wdxNext = (SplashCoord)0.5 * w * dxNext;
      wdyNext = (SplashCoord)0.5 * w * dyNext;

      // compute the join parameters
      crossprod = dx * dyNext - dy * dxNext;
      dotprod = -(dx * dxNext + dy * dyNext);
      if (dotprod > 0.9999) {
	// avoid a divide-by-zero -- set miter to something arbitrary
	// such that sqrt(miter) will exceed miterLimit (and m is never
	// used in that situation)
	// (note: the comparison value (0.9999) has to be less than
	// 1-epsilon, where epsilon is the smallest value
	// representable in the fixed point format)
	miter = (state->miterLimit + 1) * (state->miterLimit + 1);
	m = 0;
      } else {
	miter = (SplashCoord)2 / ((SplashCoord)1 - dotprod);
	if (miter < 1) {
	  // this can happen because of floating point inaccuracies
	  miter = 1;
	}
	m = splashSqrt(miter - 1);
      }

      // round join
      if (lineJoin == splashLineJoinRound) {
	// join angle < 180
	if (crossprod < 0) {
	  angle = atan2((double)dx, (double)-dy);
	  angleNext = atan2((double)dxNext, (double)-dyNext);
	  if (angle < angleNext) {
	    angle += 2 * M_PI;
	  }
	  dAngle = (angle  - angleNext) / M_PI;
	  if (dAngle < 0.501) {
	    // span angle is <= 90 degrees -> draw a single arc
	    kappa = dAngle * bezierCircle * w;
	    cx1 = pathIn->pts[j0].x - wdy + kappa * dx;
	    cy1 = pathIn->pts[j0].y + wdx + kappa * dy;
	    cx2 = pathIn->pts[j0].x - wdyNext - kappa * dxNext;
	    cy2 = pathIn->pts[j0].y + wdxNext - kappa * dyNext;
	    pathOut->moveTo(pathIn->pts[j0].x, pathIn->pts[j0].y);
	    pathOut->lineTo(pathIn->pts[j0].x - wdyNext,
			    pathIn->pts[j0].y + wdxNext);
	    pathOut->curveTo(cx2, cy2, cx1, cy1,
			     pathIn->pts[j0].x - wdy,
			     pathIn->pts[j0].y + wdx);
	  } else {
	    // span angle is > 90 degrees -> split into two arcs
	    dJoin = splashDist(-wdy, wdx, -wdyNext, wdxNext);
	    if (dJoin > 0) {
	      dxJoin = (-wdyNext + wdy) / dJoin;
	      dyJoin = (wdxNext - wdx) / dJoin;
	      xc = pathIn->pts[j0].x
		   + (SplashCoord)0.5 * w
		     * cos((double)((SplashCoord)0.5 * (angle + angleNext)));
	      yc = pathIn->pts[j0].y
		   + (SplashCoord)0.5 * w
		     * sin((double)((SplashCoord)0.5 * (angle + angleNext)));
	      kappa = dAngle * bezierCircle2 * w;
	      cx1 = pathIn->pts[j0].x - wdy + kappa * dx;
	      cy1 = pathIn->pts[j0].y + wdx + kappa * dy;
	      cx2 = xc - kappa * dxJoin;
	      cy2 = yc - kappa * dyJoin;
	      cx3 = xc + kappa * dxJoin;
	      cy3 = yc + kappa * dyJoin;
	      cx4 = pathIn->pts[j0].x - wdyNext - kappa * dxNext;
	      cy4 = pathIn->pts[j0].y + wdxNext - kappa * dyNext;
	      pathOut->moveTo(pathIn->pts[j0].x, pathIn->pts[j0].y);
	      pathOut->lineTo(pathIn->pts[j0].x - wdyNext,
			      pathIn->pts[j0].y + wdxNext);
	      pathOut->curveTo(cx4, cy4, cx3, cy3, xc, yc);
	      pathOut->curveTo(cx2, cy2, cx1, cy1,
			       pathIn->pts[j0].x - wdy,
			       pathIn->pts[j0].y + wdx);
	    }
	  }

	// join angle >= 180
	} else {
	  angle = atan2((double)-dx, (double)dy);
	  angleNext = atan2((double)-dxNext, (double)dyNext);
	  if (angleNext < angle) {
	    angleNext += 2 * M_PI;
	  }
	  dAngle = (angleNext - angle) / M_PI;
	  if (dAngle < 0.501) {
	    // span angle is <= 90 degrees -> draw a single arc
	    kappa = dAngle * bezierCircle * w;
	      cx1 = pathIn->pts[j0].x + wdy + kappa * dx;
	      cy1 = pathIn->pts[j0].y - wdx + kappa * dy;
	      cx2 = pathIn->pts[j0].x + wdyNext - kappa * dxNext;
	      cy2 = pathIn->pts[j0].y - wdxNext - kappa * dyNext;
	      pathOut->moveTo(pathIn->pts[j0].x, pathIn->pts[j0].y);
	      pathOut->lineTo(pathIn->pts[j0].x + wdy,
			      pathIn->pts[j0].y - wdx);
	      pathOut->curveTo(cx1, cy1, cx2, cy2,
			       pathIn->pts[j0].x + wdyNext,
			       pathIn->pts[j0].y - wdxNext);
	  } else {
	    // span angle is > 90 degrees -> split into two arcs
	    dJoin = splashDist(wdy, -wdx, wdyNext, -wdxNext);
	    if (dJoin > 0) {
	      dxJoin = (wdyNext - wdy) / dJoin;
	      dyJoin = (-wdxNext + wdx) / dJoin;
	      xc = pathIn->pts[j0].x
		   + (SplashCoord)0.5 * w
		     * cos((double)((SplashCoord)0.5 * (angle + angleNext)));
	      yc = pathIn->pts[j0].y
		   + (SplashCoord)0.5 * w
		     * sin((double)((SplashCoord)0.5 * (angle + angleNext)));
	      kappa = dAngle * bezierCircle2 * w;
	      cx1 = pathIn->pts[j0].x + wdy + kappa * dx;
	      cy1 = pathIn->pts[j0].y - wdx + kappa * dy;
	      cx2 = xc - kappa * dxJoin;
	      cy2 = yc - kappa * dyJoin;
	      cx3 = xc + kappa * dxJoin;
	      cy3 = yc + kappa * dyJoin;
	      cx4 = pathIn->pts[j0].x + wdyNext - kappa * dxNext;
	      cy4 = pathIn->pts[j0].y - wdxNext - kappa * dyNext;
	      pathOut->moveTo(pathIn->pts[j0].x, pathIn->pts[j0].y);
	      pathOut->lineTo(pathIn->pts[j0].x + wdy,
			      pathIn->pts[j0].y - wdx);
	      pathOut->curveTo(cx1, cy1, cx2, cy2, xc, yc);
	      pathOut->curveTo(cx3, cy3, cx4, cy4,
			       pathIn->pts[j0].x + wdyNext,
			       pathIn->pts[j0].y - wdxNext);
	    }
	  }
	}

      } else {
	pathOut->moveTo(pathIn->pts[j0].x, pathIn->pts[j0].y);

	// join angle < 180
	if (crossprod < 0) {
	  pathOut->lineTo(pathIn->pts[j0].x - wdyNext,
			  pathIn->pts[j0].y + wdxNext);
	  // miter join inside limit
	  if (lineJoin == splashLineJoinMiter &&
	      splashSqrt(miter) <= state->miterLimit) {
	    pathOut->lineTo(pathIn->pts[j0].x - wdy + wdx * m,
			    pathIn->pts[j0].y + wdx + wdy * m);
	    pathOut->lineTo(pathIn->pts[j0].x - wdy,
			    pathIn->pts[j0].y + wdx);
	  // bevel join or miter join outside limit
	  } else {
	    pathOut->lineTo(pathIn->pts[j0].x - wdy,
			    pathIn->pts[j0].y + wdx);
	  }

	// join angle >= 180
	} else {
	  pathOut->lineTo(pathIn->pts[j0].x + wdy,
			  pathIn->pts[j0].y - wdx);
	  // miter join inside limit
	  if (lineJoin == splashLineJoinMiter &&
	      splashSqrt(miter) <= state->miterLimit) {
	    pathOut->lineTo(pathIn->pts[j0].x + wdy + wdx * m,
			    pathIn->pts[j0].y - wdx + wdy * m);
	    pathOut->lineTo(pathIn->pts[j0].x + wdyNext,
			    pathIn->pts[j0].y - wdxNext);
	  // bevel join or miter join outside limit
	  } else {
	    pathOut->lineTo(pathIn->pts[j0].x + wdyNext,
			    pathIn->pts[j0].y - wdxNext);
	  }
	}
      }

      pathOut->close();
    }

    // add stroke adjustment hints
    if (state->strokeAdjust != splashStrokeAdjustOff) {

      // subpath with one segment
      if (seg == 0 && last) {
	switch (lineCap) {
	case splashLineCapButt:
	  pathOut->addStrokeAdjustHint(firstPt, left2 + 1,
				       firstPt, pathOut->length - 1);
	  break;
	case splashLineCapProjecting:
	  pathOut->addStrokeAdjustHint(firstPt, left2 + 1,
				       firstPt, pathOut->length - 1, gTrue);
	  break;
	case splashLineCapRound:
	  break;
	}
	pathOut->addStrokeAdjustHint(left2, right2,
				     firstPt, pathOut->length - 1);
      } else {

	// start of subpath
	if (seg == 1) {

	  // start cap
	  if (!closed) {
	    switch (lineCap) {
	    case splashLineCapButt:
	      pathOut->addStrokeAdjustHint(firstPt, left1 + 1,
					   firstPt, firstPt + 1);
	      pathOut->addStrokeAdjustHint(firstPt, left1 + 1,
					   right1 + 1, right1 + 1);
	      break;
	    case splashLineCapProjecting:
	      pathOut->addStrokeAdjustHint(firstPt, left1 + 1,
					   firstPt, firstPt + 1, gTrue);
	      pathOut->addStrokeAdjustHint(firstPt, left1 + 1,
					   right1 + 1, right1 + 1, gTrue);
	      break;
	    case splashLineCapRound:
	      break;
	    }
	  }

	  // first segment
	  pathOut->addStrokeAdjustHint(left1, right1, firstPt, left2);
	  pathOut->addStrokeAdjustHint(left1, right1, right2 + 1, right2 + 1);
	}

	// middle of subpath
	if (seg > 1) {
	  pathOut->addStrokeAdjustHint(left1, right1, left0 + 1, right0);
	  pathOut->addStrokeAdjustHint(left1, right1, join0, left2);
	  pathOut->addStrokeAdjustHint(left1, right1, right2 + 1, right2 + 1);
	}

	// end of subpath
	if (last) {

	  if (closed) {
	    // first segment
	    pathOut->addStrokeAdjustHint(leftFirst, rightFirst,
					 left2 + 1, right2);
	    pathOut->addStrokeAdjustHint(leftFirst, rightFirst,
					 join2, pathOut->length - 1);

	    // last segment
	    pathOut->addStrokeAdjustHint(left2, right2,
					 left1 + 1, right1);
	    pathOut->addStrokeAdjustHint(left2, right2,
					 join1, pathOut->length - 1);
	    pathOut->addStrokeAdjustHint(left2, right2,
					 leftFirst - 1, leftFirst);
	    pathOut->addStrokeAdjustHint(left2, right2,
					 rightFirst + 1, rightFirst + 1);

	  } else {

	    // last segment
	    pathOut->addStrokeAdjustHint(left2, right2,
					 left1 + 1, right1);
	    pathOut->addStrokeAdjustHint(left2, right2,
					 join1, pathOut->length - 1);

	    // end cap
	    switch (lineCap) {
	    case splashLineCapButt:
	      pathOut->addStrokeAdjustHint(left2 - 1, left2 + 1,
					   left2 + 1, left2 + 2);
	      break;
	    case splashLineCapProjecting:
	      pathOut->addStrokeAdjustHint(left2 - 1, left2 + 1,
					   left2 + 1, left2 + 2, gTrue);
	      break;
	    case splashLineCapRound:
	      break;
	    }
	  }
	}
      }

      left0 = left1;
      left1 = left2;
      right0 = right1;
      right1 = right2;
      join0 = join1;
      join1 = join2;
      if (seg == 0) {
	leftFirst = left2;
	rightFirst = right2;
      }
    }

    i0 = j0;
    i1 = j1;
    ++seg;
  }

  if (pathIn != path) {
    delete pathIn;
  }

  return pathOut;
}

SplashClipResult Splash::limitRectToClipRect(int *xMin, int *yMin,
					     int *xMax, int *yMax) {
  int t;

  if ((t = state->clip->getXMinI(state->strokeAdjust)) > *xMin) {
    *xMin = t;
  }
  if ((t = state->clip->getXMaxI(state->strokeAdjust) + 1) < *xMax) {
    *xMax = t;
  }
  if ((t = state->clip->getYMinI(state->strokeAdjust)) > *yMin) {
    *yMin = t;
  }
  if ((t = state->clip->getYMaxI(state->strokeAdjust) + 1) < *yMax) {
    *yMax = t;
  }
  if (*xMin >= *xMax || *yMin >= *yMax) {
    return splashClipAllOutside;
  }
  return state->clip->testRect(*xMin, *yMin, *xMax - 1, *yMax - 1,
			       state->strokeAdjust);
}

void Splash::dumpPath(SplashPath *path) {
  int i;

  for (i = 0; i < path->length; ++i) {
    printf("  %3d: x=%8.2f y=%8.2f%s%s%s%s\n",
	   i, (double)path->pts[i].x, (double)path->pts[i].y,
	   (path->flags[i] & splashPathFirst) ? " first" : "",
	   (path->flags[i] & splashPathLast) ? " last" : "",
	   (path->flags[i] & splashPathClosed) ? " closed" : "",
	   (path->flags[i] & splashPathCurve) ? " curve" : "");
  }
  if (path->hintsLength == 0) {
    printf("  no hints\n");
  } else {
    for (i = 0; i < path->hintsLength; ++i) {
      printf("  hint %3d: ctrl0=%d ctrl1=%d pts=%d..%d\n",
	     i, path->hints[i].ctrl0, path->hints[i].ctrl1,
	     path->hints[i].firstPt, path->hints[i].lastPt);
    }
  }
}

void Splash::dumpXPath(SplashXPath *path) {
  int i;

  for (i = 0; i < path->length; ++i) {
    printf("  %4d: x0=%8.2f y0=%8.2f x1=%8.2f y1=%8.2f count=%d\n",
	   i, (double)path->segs[i].x0, (double)path->segs[i].y0,
	   (double)path->segs[i].x1, (double)path->segs[i].y1,
	   path->segs[i].count);
  }
}

