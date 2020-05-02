//========================================================================
//
// Function.cc
//
// Copyright 2001-2003 Glyph & Cog, LLC
//
//========================================================================

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "gmem.h"
#include "gmempp.h"
#include "GList.h"
#include "Object.h"
#include "Dict.h"
#include "Stream.h"
#include "Error.h"
#include "Function.h"

//------------------------------------------------------------------------

// Max depth of nested functions.  This is used to catch infinite
// loops in the function object structure.
#define recursionLimit 8

//------------------------------------------------------------------------
// Function
//------------------------------------------------------------------------

Function::Function() {
}

Function::~Function() {
}

Function *Function::parse(Object *funcObj, int recursion) {
  Function *func;
  Dict *dict;
  int funcType;
  Object obj1;

  if (recursion > recursionLimit) {
    error(errSyntaxError, -1, "Loop detected in function objects");
    return NULL;
  }

  if (funcObj->isStream()) {
    dict = funcObj->streamGetDict();
  } else if (funcObj->isDict()) {
    dict = funcObj->getDict();
  } else if (funcObj->isName("Identity")) {
    return new IdentityFunction();
  } else {
    error(errSyntaxError, -1, "Expected function dictionary or stream");
    return NULL;
  }

  if (!dict->lookup("FunctionType", &obj1)->isInt()) {
    error(errSyntaxError, -1, "Function type is missing or wrong type");
    obj1.free();
    return NULL;
  }
  funcType = obj1.getInt();
  obj1.free();

  if (funcType == 0) {
    func = new SampledFunction(funcObj, dict);
  } else if (funcType == 2) {
    func = new ExponentialFunction(funcObj, dict);
  } else if (funcType == 3) {
    func = new StitchingFunction(funcObj, dict, recursion);
  } else if (funcType == 4) {
    func = new PostScriptFunction(funcObj, dict);
  } else {
    error(errSyntaxError, -1, "Unimplemented function type ({0:d})", funcType);
    return NULL;
  }
  if (!func->isOk()) {
    delete func;
    return NULL;
  }

  return func;
}

GBool Function::init(Dict *dict) {
  Object obj1, obj2;
  int i;

  //----- Domain
  if (!dict->lookup("Domain", &obj1)->isArray()) {
    error(errSyntaxError, -1, "Function is missing domain");
    goto err2;
  }
  m = obj1.arrayGetLength() / 2;
  if (m > funcMaxInputs) {
    error(errSyntaxError, -1,
	  "Functions with more than {0:d} inputs are unsupported",
	  funcMaxInputs);
    goto err2;
  }
  for (i = 0; i < m; ++i) {
    obj1.arrayGet(2*i, &obj2);
    if (!obj2.isNum()) {
      error(errSyntaxError, -1, "Illegal value in function domain array");
      goto err1;
    }
    domain[i][0] = obj2.getNum();
    obj2.free();
    obj1.arrayGet(2*i+1, &obj2);
    if (!obj2.isNum()) {
      error(errSyntaxError, -1, "Illegal value in function domain array");
      goto err1;
    }
    domain[i][1] = obj2.getNum();
    obj2.free();
  }
  obj1.free();

  //----- Range
  hasRange = gFalse;
  n = 0;
  if (dict->lookup("Range", &obj1)->isArray()) {
    hasRange = gTrue;
    n = obj1.arrayGetLength() / 2;
    if (n > funcMaxOutputs) {
      error(errSyntaxError, -1,
	    "Functions with more than {0:d} outputs are unsupported",
	    funcMaxOutputs);
      goto err2;
    }
    for (i = 0; i < n; ++i) {
      obj1.arrayGet(2*i, &obj2);
      if (!obj2.isNum()) {
	error(errSyntaxError, -1, "Illegal value in function range array");
	goto err1;
      }
      range[i][0] = obj2.getNum();
      obj2.free();
      obj1.arrayGet(2*i+1, &obj2);
      if (!obj2.isNum()) {
	error(errSyntaxError, -1, "Illegal value in function range array");
	goto err1;
      }
      range[i][1] = obj2.getNum();
      obj2.free();
    }
  }
  obj1.free();

  return gTrue;

 err1:
  obj2.free();
 err2:
  obj1.free();
  return gFalse;
}

//------------------------------------------------------------------------
// IdentityFunction
//------------------------------------------------------------------------

IdentityFunction::IdentityFunction() {
  int i;

  // fill these in with arbitrary values just in case they get used
  // somewhere
  m = funcMaxInputs;
  n = funcMaxOutputs;
  for (i = 0; i < funcMaxInputs; ++i) {
    domain[i][0] = 0;
    domain[i][1] = 1;
  }
  hasRange = gFalse;
}

IdentityFunction::~IdentityFunction() {
}

void IdentityFunction::transform(double *in, double *out) {
  int i;

  for (i = 0; i < funcMaxOutputs; ++i) {
    out[i] = in[i];
  }
}

//------------------------------------------------------------------------
// SampledFunction
//------------------------------------------------------------------------

SampledFunction::SampledFunction(Object *funcObj, Dict *dict) {
  Stream *str;
  int sampleBits;
  double sampleMul;
  Object obj1, obj2;
  Guint buf, bitMask;
  int bits;
  Guint s;
  double in[funcMaxInputs];
  int i, j, t, bit, idx;

  idxOffset = NULL;
  samples = NULL;
  sBuf = NULL;
  ok = gFalse;

  //----- initialize the generic stuff
  if (!init(dict)) {
    goto err1;
  }
  if (!hasRange) {
    error(errSyntaxError, -1, "Type 0 function is missing range");
    goto err1;
  }
  if (m > sampledFuncMaxInputs) {
    error(errSyntaxError, -1,
	  "Sampled functions with more than {0:d} inputs are unsupported",
	  sampledFuncMaxInputs);
    goto err1;
  }

  //----- buffer
  sBuf = (double *)gmallocn(1 << m, sizeof(double));

  //----- get the stream
  if (!funcObj->isStream()) {
    error(errSyntaxError, -1, "Type 0 function isn't a stream");
    goto err1;
  }
  str = funcObj->getStream();

  //----- Size
  if (!dict->lookup("Size", &obj1)->isArray() ||
      obj1.arrayGetLength() != m) {
    error(errSyntaxError, -1, "Function has missing or invalid size array");
    goto err2;
  }
  for (i = 0; i < m; ++i) {
    obj1.arrayGet(i, &obj2);
    if (!obj2.isInt()) {
      error(errSyntaxError, -1, "Illegal value in function size array");
      goto err3;
    }
    sampleSize[i] = obj2.getInt();
    if (sampleSize[i] <= 0) {
      error(errSyntaxError, -1, "Illegal non-positive value in function size array");
      goto err3;
    }
    obj2.free();
  }
  obj1.free();
  idxOffset = (int *)gmallocn(1 << m, sizeof(int));
  for (i = 0; i < (1<<m); ++i) {
    idx = 0;
    for (j = m - 1, t = i; j >= 1; --j, t <<= 1) {
      if (sampleSize[j] == 1) {
	bit = 0;
      } else {
	bit = (t >> (m - 1)) & 1;
      }
      idx = (idx + bit) * sampleSize[j-1];
    }
    if (sampleSize[0] == 1) {
      bit = 0;
    } else {
      bit = (t >> (m - 1)) & 1;
    }
    idxOffset[i] = (idx + bit) * n;
  }

  //----- BitsPerSample
  if (!dict->lookup("BitsPerSample", &obj1)->isInt()) {
    error(errSyntaxError, -1, "Function has missing or invalid BitsPerSample");
    goto err2;
  }
  sampleBits = obj1.getInt();
  sampleMul = 1.0 / (pow(2.0, (double)sampleBits) - 1);
  obj1.free();

  //----- Encode
  if (dict->lookup("Encode", &obj1)->isArray() &&
      obj1.arrayGetLength() == 2*m) {
    for (i = 0; i < m; ++i) {
      obj1.arrayGet(2*i, &obj2);
      if (!obj2.isNum()) {
	error(errSyntaxError, -1, "Illegal value in function encode array");
	goto err3;
      }
      encode[i][0] = obj2.getNum();
      obj2.free();
      obj1.arrayGet(2*i+1, &obj2);
      if (!obj2.isNum()) {
	error(errSyntaxError, -1, "Illegal value in function encode array");
	goto err3;
      }
      encode[i][1] = obj2.getNum();
      obj2.free();
    }
  } else {
    for (i = 0; i < m; ++i) {
      encode[i][0] = 0;
      encode[i][1] = sampleSize[i] - 1;
    }
  }
  obj1.free();
  for (i = 0; i < m; ++i) {
    inputMul[i] = (encode[i][1] - encode[i][0]) /
                  (domain[i][1] - domain[i][0]);
  }

  //----- Decode
  if (dict->lookup("Decode", &obj1)->isArray() &&
      obj1.arrayGetLength() == 2*n) {
    for (i = 0; i < n; ++i) {
      obj1.arrayGet(2*i, &obj2);
      if (!obj2.isNum()) {
	error(errSyntaxError, -1, "Illegal value in function decode array");
	goto err3;
      }
      decode[i][0] = obj2.getNum();
      obj2.free();
      obj1.arrayGet(2*i+1, &obj2);
      if (!obj2.isNum()) {
	error(errSyntaxError, -1, "Illegal value in function decode array");
	goto err3;
      }
      decode[i][1] = obj2.getNum();
      obj2.free();
    }
  } else {
    for (i = 0; i < n; ++i) {
      decode[i][0] = range[i][0];
      decode[i][1] = range[i][1];
    }
  }
  obj1.free();

  //----- samples
  nSamples = n;
  for (i = 0; i < m; ++i)
    nSamples *= sampleSize[i];
  samples = (double *)gmallocn(nSamples, sizeof(double));
  buf = 0;
  bits = 0;
  bitMask = (sampleBits < 32) ? ((1 << sampleBits) - 1) : 0xffffffffU;
  str->reset();
  for (i = 0; i < nSamples; ++i) {
    if (sampleBits == 8) {
      s = str->getChar();
    } else if (sampleBits == 16) {
      s = str->getChar();
      s = (s << 8) + str->getChar();
    } else if (sampleBits == 32) {
      s = str->getChar();
      s = (s << 8) + str->getChar();
      s = (s << 8) + str->getChar();
      s = (s << 8) + str->getChar();
    } else {
      while (bits < sampleBits) {
	buf = (buf << 8) | (str->getChar() & 0xff);
	bits += 8;
      }
      s = (buf >> (bits - sampleBits)) & bitMask;
      bits -= sampleBits;
    }
    samples[i] = (double)s * sampleMul;
  }
  str->close();

  // set up the cache
  for (i = 0; i < m; ++i) {
    in[i] = domain[i][0];
    cacheIn[i] = in[i] - 1;
  }
  transform(in, cacheOut);

  ok = gTrue;
  return;

 err3:
  obj2.free();
 err2:
  obj1.free();
 err1:
  return;
}

SampledFunction::~SampledFunction() {
  if (idxOffset) {
    gfree(idxOffset);
  }
  if (samples) {
    gfree(samples);
  }
  if (sBuf) {
    gfree(sBuf);
  }
}

SampledFunction::SampledFunction(SampledFunction *func) {
  memcpy((void *)this, (void *)func, sizeof(SampledFunction));
  idxOffset = (int *)gmallocn(1 << m, sizeof(int));
  memcpy(idxOffset, func->idxOffset, (1 << m) * (int)sizeof(int));
  samples = (double *)gmallocn(nSamples, sizeof(double));
  memcpy(samples, func->samples, nSamples * sizeof(double));
  sBuf = (double *)gmallocn(1 << m, sizeof(double));
}

void SampledFunction::transform(double *in, double *out) {
  double x;
  int e[funcMaxInputs];
  double efrac0[funcMaxInputs];
  double efrac1[funcMaxInputs];
  int i, j, k, idx0, t;

  // check the cache
  for (i = 0; i < m; ++i) {
    if (in[i] != cacheIn[i]) {
      break;
    }
  }
  if (i == m) {
    for (i = 0; i < n; ++i) {
      out[i] = cacheOut[i];
    }
    return;
  }

  // map input values into sample array
  for (i = 0; i < m; ++i) {
    x = (in[i] - domain[i][0]) * inputMul[i] + encode[i][0];
    if (x < 0 || x != x) {  // x!=x is a more portable version of isnan(x)
      x = 0;
    } else if (x > sampleSize[i] - 1) {
      x = sampleSize[i] - 1;
    }
    e[i] = (int)x;
    if (e[i] == sampleSize[i] - 1 && sampleSize[i] > 1) {
      // this happens if in[i] = domain[i][1]
      e[i] = sampleSize[i] - 2;
    }
    efrac1[i] = x - e[i];
    efrac0[i] = 1 - efrac1[i];
  }

  // compute index for the first sample to be used
  idx0 = 0;
  for (k = m - 1; k >= 1; --k) {
    idx0 = (idx0 + e[k]) * sampleSize[k-1];
  }
  idx0 = (idx0 + e[0]) * n;

  // for each output, do m-linear interpolation
  for (i = 0; i < n; ++i) {

    // pull 2^m values out of the sample array
    for (j = 0; j < (1<<m); ++j) {
      sBuf[j] = samples[idx0 + idxOffset[j] + i];
    }

    // do m sets of interpolations
    for (j = 0, t = (1<<m); j < m; ++j, t >>= 1) {
      for (k = 0; k < t; k += 2) {
	sBuf[k >> 1] = efrac0[j] * sBuf[k] + efrac1[j] * sBuf[k+1];
      }
    }

    // map output value to range
    out[i] = sBuf[0] * (decode[i][1] - decode[i][0]) + decode[i][0];
    if (out[i] < range[i][0]) {
      out[i] = range[i][0];
    } else if (out[i] > range[i][1]) {
      out[i] = range[i][1];
    }
  }

  // save current result in the cache
  for (i = 0; i < m; ++i) {
    cacheIn[i] = in[i];
  }
  for (i = 0; i < n; ++i) {
    cacheOut[i] = out[i];
  }
}

//------------------------------------------------------------------------
// ExponentialFunction
//------------------------------------------------------------------------

ExponentialFunction::ExponentialFunction(Object *funcObj, Dict *dict) {
  Object obj1, obj2;
  int i;

  ok = gFalse;

  //----- initialize the generic stuff
  if (!init(dict)) {
    goto err1;
  }
  if (m != 1) {
    error(errSyntaxError, -1, "Exponential function with more than one input");
    goto err1;
  }

  //----- C0
  if (dict->lookup("C0", &obj1)->isArray()) {
    if (hasRange && obj1.arrayGetLength() != n) {
      error(errSyntaxError, -1, "Function's C0 array is wrong length");
      goto err2;
    }
    n = obj1.arrayGetLength();
    if (n > funcMaxOutputs) {
      error(errSyntaxError, -1,
	    "Functions with more than {0:d} outputs are unsupported",
	    funcMaxOutputs);
      goto err2;
    }
    for (i = 0; i < n; ++i) {
      obj1.arrayGet(i, &obj2);
      if (!obj2.isNum()) {
	error(errSyntaxError, -1, "Illegal value in function C0 array");
	goto err3;
      }
      c0[i] = obj2.getNum();
      obj2.free();
    }
  } else {
    if (hasRange && n != 1) {
      error(errSyntaxError, -1, "Function's C0 array is wrong length");
      goto err2;
    }
    n = 1;
    c0[0] = 0;
  }
  obj1.free();

  //----- C1
  if (dict->lookup("C1", &obj1)->isArray()) {
    if (obj1.arrayGetLength() != n) {
      error(errSyntaxError, -1, "Function's C1 array is wrong length");
      goto err2;
    }
    for (i = 0; i < n; ++i) {
      obj1.arrayGet(i, &obj2);
      if (!obj2.isNum()) {
	error(errSyntaxError, -1, "Illegal value in function C1 array");
	goto err3;
      }
      c1[i] = obj2.getNum();
      obj2.free();
    }
  } else {
    if (n != 1) {
      error(errSyntaxError, -1, "Function's C1 array is wrong length");
      goto err2;
    }
    c1[0] = 1;
  }
  obj1.free();

  //----- N (exponent)
  if (!dict->lookup("N", &obj1)->isNum()) {
    error(errSyntaxError, -1, "Function has missing or invalid N");
    goto err2;
  }
  e = obj1.getNum();
  obj1.free();

  ok = gTrue;
  return;

 err3:
  obj2.free();
 err2:
  obj1.free();
 err1:
  return;
}

ExponentialFunction::~ExponentialFunction() {
}

ExponentialFunction::ExponentialFunction(ExponentialFunction *func) {
  memcpy((void *)this, (void *)func, sizeof(ExponentialFunction));
}

void ExponentialFunction::transform(double *in, double *out) {
  double x;
  int i;

  if (in[0] < domain[0][0]) {
    x = domain[0][0];
  } else if (in[0] > domain[0][1]) {
    x = domain[0][1];
  } else {
    x = in[0];
  }
  for (i = 0; i < n; ++i) {
    out[i] = c0[i] + pow(x, e) * (c1[i] - c0[i]);
    if (hasRange) {
      if (out[i] < range[i][0]) {
	out[i] = range[i][0];
      } else if (out[i] > range[i][1]) {
	out[i] = range[i][1];
      }
    }
  }
  return;
}

//------------------------------------------------------------------------
// StitchingFunction
//------------------------------------------------------------------------

StitchingFunction::StitchingFunction(Object *funcObj, Dict *dict,
				     int recursion) {
  Object obj1, obj2;
  int i;

  ok = gFalse;
  funcs = NULL;
  bounds = NULL;
  encode = NULL;
  scale = NULL;

  //----- initialize the generic stuff
  if (!init(dict)) {
    goto err1;
  }
  if (m != 1) {
    error(errSyntaxError, -1, "Stitching function with more than one input");
    goto err1;
  }

  //----- Functions
  if (!dict->lookup("Functions", &obj1)->isArray()) {
    error(errSyntaxError, -1,
	  "Missing 'Functions' entry in stitching function");
    goto err1;
  }
  k = obj1.arrayGetLength();
  funcs = (Function **)gmallocn(k, sizeof(Function *));
  bounds = (double *)gmallocn(k + 1, sizeof(double));
  encode = (double *)gmallocn(2 * k, sizeof(double));
  scale = (double *)gmallocn(k, sizeof(double));
  for (i = 0; i < k; ++i) {
    funcs[i] = NULL;
  }
  for (i = 0; i < k; ++i) {
    if (!(funcs[i] = Function::parse(obj1.arrayGet(i, &obj2),
				     recursion + 1))) {
      goto err2;
    }
    if (funcs[i]->getInputSize() != 1 ||
	(i > 0 && funcs[i]->getOutputSize() != funcs[0]->getOutputSize())) {
      error(errSyntaxError, -1,
	    "Incompatible subfunctions in stitching function");
      goto err2;
    }
    obj2.free();
  }
  obj1.free();

  //----- Bounds
  if (!dict->lookup("Bounds", &obj1)->isArray() ||
      obj1.arrayGetLength() != k - 1) {
    error(errSyntaxError, -1,
	  "Missing or invalid 'Bounds' entry in stitching function");
    goto err1;
  }
  bounds[0] = domain[0][0];
  for (i = 1; i < k; ++i) {
    if (!obj1.arrayGet(i - 1, &obj2)->isNum()) {
      error(errSyntaxError, -1,
	    "Invalid type in 'Bounds' array in stitching function");
      goto err2;
    }
    bounds[i] = obj2.getNum();
    obj2.free();
  }
  bounds[k] = domain[0][1];
  obj1.free();

  //----- Encode
  if (!dict->lookup("Encode", &obj1)->isArray() ||
      obj1.arrayGetLength() != 2 * k) {
    error(errSyntaxError, -1,
	  "Missing or invalid 'Encode' entry in stitching function");
    goto err1;
  }
  for (i = 0; i < 2 * k; ++i) {
    if (!obj1.arrayGet(i, &obj2)->isNum()) {
      error(errSyntaxError, -1,
	    "Invalid type in 'Encode' array in stitching function");
      goto err2;
    }
    encode[i] = obj2.getNum();
    obj2.free();
  }
  obj1.free();

  //----- pre-compute the scale factors
  for (i = 0; i < k; ++i) {
    if (bounds[i] == bounds[i+1]) {
      // avoid a divide-by-zero -- in this situation, function i will
      // never be used anyway
      scale[i] = 0;
    } else {
      scale[i] = (encode[2*i+1] - encode[2*i]) / (bounds[i+1] - bounds[i]);
    }
  }

  ok = gTrue;
  return;

 err2:
  obj2.free();
 err1:
  obj1.free();
}

StitchingFunction::StitchingFunction(StitchingFunction *func) {
  int i;

  memcpy((void *)this, (void *)func, sizeof(StitchingFunction));
  funcs = (Function **)gmallocn(k, sizeof(Function *));
  for (i = 0; i < k; ++i) {
    funcs[i] = func->funcs[i]->copy();
  }
  bounds = (double *)gmallocn(k + 1, sizeof(double));
  memcpy(bounds, func->bounds, (k + 1) * sizeof(double));
  encode = (double *)gmallocn(2 * k, sizeof(double));
  memcpy(encode, func->encode, 2 * k * sizeof(double));
  scale = (double *)gmallocn(k, sizeof(double));
  memcpy(scale, func->scale, k * sizeof(double));
  ok = gTrue;
}

StitchingFunction::~StitchingFunction() {
  int i;

  if (funcs) {
    for (i = 0; i < k; ++i) {
      if (funcs[i]) {
	delete funcs[i];
      }
    }
  }
  gfree(funcs);
  gfree(bounds);
  gfree(encode);
  gfree(scale);
}

void StitchingFunction::transform(double *in, double *out) {
  double x;
  int i;

  if (in[0] < domain[0][0]) {
    x = domain[0][0];
  } else if (in[0] > domain[0][1]) {
    x = domain[0][1];
  } else {
    x = in[0];
  }
  for (i = 0; i < k - 1; ++i) {
    if (x < bounds[i+1]) {
      break;
    }
  }
  x = encode[2*i] + (x - bounds[i]) * scale[i];
  funcs[i]->transform(&x, out);
}

//------------------------------------------------------------------------
// PostScriptFunction
//------------------------------------------------------------------------

// This is not an enum, because we can't foreward-declare the enum
// type in Function.h
//
// NB: This must be kept in sync with psOpNames[] below.
#define psOpAbs       0
#define psOpAdd       1
#define psOpAnd       2
#define psOpAtan      3
#define psOpBitshift  4
#define psOpCeiling   5
#define psOpCopy      6
#define psOpCos       7
#define psOpCvi       8
#define psOpCvr       9
#define psOpDiv      10
#define psOpDup      11
#define psOpEq       12
#define psOpExch     13
#define psOpExp      14
#define psOpFalse    15
#define psOpFloor    16
#define psOpGe       17
#define psOpGt       18
#define psOpIdiv     19
#define psOpIndex    20
#define psOpLe       21
#define psOpLn       22
#define psOpLog      23
#define psOpLt       24
#define psOpMod      25
#define psOpMul      26
#define psOpNe       27
#define psOpNeg      28
#define psOpNot      29
#define psOpOr       30
#define psOpPop      31
#define psOpRoll     32
#define psOpRound    33
#define psOpSin      34
#define psOpSqrt     35
#define psOpSub      36
#define psOpTrue     37
#define psOpTruncate 38
#define psOpXor      39
// the push/j/jz ops are used internally (and are not listed in psOpNames[])
#define psOpPush     40
#define psOpJ        41
#define psOpJz       42

#define nPSOps (sizeof(psOpNames) / sizeof(const char *))

// Note: 'if' and 'ifelse' are parsed separately.
// The rest are listed here in alphabetical order.
//
// NB: This must be kept in sync with the psOpXXX defines above.
static const char *psOpNames[] = {
  "abs",
  "add",
  "and",
  "atan",
  "bitshift",
  "ceiling",
  "copy",
  "cos",
  "cvi",
  "cvr",
  "div",
  "dup",
  "eq",
  "exch",
  "exp",
  "false",
  "floor",
  "ge",
  "gt",
  "idiv",
  "index",
  "le",
  "ln",
  "log",
  "lt",
  "mod",
  "mul",
  "ne",
  "neg",
  "not",
  "or",
  "pop",
  "roll",
  "round",
  "sin",
  "sqrt",
  "sub",
  "true",
  "truncate",
  "xor"
};

struct PSCode {
  int op;
  union {
    double d;
    int i;
  } val;
};

#define psStackSize 100

PostScriptFunction::PostScriptFunction(Object *funcObj, Dict *dict) {
  Stream *str;
  GList *tokens;
  GString *tok;
  double in[funcMaxInputs];
  int tokPtr, codePtr, i;

  codeString = NULL;
  code = NULL;
  codeSize = 0;
  ok = gFalse;

  //----- initialize the generic stuff
  if (!init(dict)) {
    goto err1;
  }
  if (!hasRange) {
    error(errSyntaxError, -1, "Type 4 function is missing range");
    goto err1;
  }

  //----- get the stream
  if (!funcObj->isStream()) {
    error(errSyntaxError, -1, "Type 4 function isn't a stream");
    goto err1;
  }
  str = funcObj->getStream();

  //----- tokenize the function
  codeString = new GString();
  tokens = new GList();
  str->reset();
  while ((tok = getToken(str))) {
    tokens->append(tok);
  }
  str->close();

  //----- parse the function
  if (tokens->getLength() < 1 ||
      ((GString *)tokens->get(0))->cmp("{")) {
    error(errSyntaxError, -1, "Expected '{{' at start of PostScript function");
    goto err2;
  }
  tokPtr = 1;
  codePtr = 0;
  if (!parseCode(tokens, &tokPtr, &codePtr)) {
    goto err2;
  }
  codeLen = codePtr;

  //----- set up the cache
  for (i = 0; i < m; ++i) {
    in[i] = domain[i][0];
    cacheIn[i] = in[i] - 1;
  }
  transform(in, cacheOut);

  ok = gTrue;

 err2:
  deleteGList(tokens, GString);
 err1:
  return;
}

PostScriptFunction::PostScriptFunction(PostScriptFunction *func) {
  memcpy((void *)this, (void *)func, sizeof(PostScriptFunction));
  codeString = func->codeString->copy();
  code = (PSCode *)gmallocn(codeSize, sizeof(PSCode));
  memcpy(code, func->code, codeSize * sizeof(PSCode));
}

PostScriptFunction::~PostScriptFunction() {
  gfree(code);
  if (codeString) {
    delete codeString;
  }
}

void PostScriptFunction::transform(double *in, double *out) {
  double stack[psStackSize];
  double x;
  int sp, i;

  // check the cache
  for (i = 0; i < m; ++i) {
    if (in[i] != cacheIn[i]) {
      break;
    }
  }
  if (i == m) {
    for (i = 0; i < n; ++i) {
      out[i] = cacheOut[i];
    }
    return;
  }

  for (i = 0; i < m; ++i) {
    stack[psStackSize - 1 - i] = in[i];
  }
  sp = exec(stack, psStackSize - m);
  // if (sp < psStackSize - n) {
  //   error(errSyntaxWarning, -1,
  // 	  "Extra values on stack at end of PostScript function");
  // }
  if (sp > psStackSize - n) {
    error(errSyntaxError, -1, "Stack underflow in PostScript function");
    sp = psStackSize - n;
  }
  for (i = 0; i < n; ++i) {
    x = stack[sp + n - 1 - i];
    if (x < range[i][0]) {
      out[i] = range[i][0];
    } else if (x > range[i][1]) {
      out[i] = range[i][1];
    } else {
      out[i] = x;
    }
  }

  // save current result in the cache
  for (i = 0; i < m; ++i) {
    cacheIn[i] = in[i];
  }
  for (i = 0; i < n; ++i) {
    cacheOut[i] = out[i];
  }
}

GBool PostScriptFunction::parseCode(GList *tokens, int *tokPtr, int *codePtr) {
  GString *tok;
  char *p;
  int a, b, mid, cmp;
  int codePtr0, codePtr1;

  while (1) {
    if (*tokPtr >= tokens->getLength()) {
      error(errSyntaxError, -1,
	    "Unexpected end of PostScript function stream");
      return gFalse;
    }
    tok = (GString *)tokens->get((*tokPtr)++);
    p = tok->getCString();
    if (isdigit(*p) || *p == '.' || *p == '-') {
      addCodeD(codePtr, psOpPush, atof(tok->getCString()));
    } else if (!tok->cmp("{")) {
      codePtr0 = *codePtr;
      addCodeI(codePtr, psOpJz, 0);
      if (!parseCode(tokens, tokPtr, codePtr)) {
	return gFalse;
      }
      if (*tokPtr >= tokens->getLength()) {
	error(errSyntaxError, -1,
	      "Unexpected end of PostScript function stream");
	return gFalse;
      }
      tok = (GString *)tokens->get((*tokPtr)++);
      if (!tok->cmp("if")) {
	code[codePtr0].val.i = *codePtr;
      } else if (!tok->cmp("{")) {
	codePtr1 = *codePtr;
	addCodeI(codePtr, psOpJ, 0);
	code[codePtr0].val.i = *codePtr;
	if (!parseCode(tokens, tokPtr, codePtr)) {
	  return gFalse;
	}
	if (*tokPtr >= tokens->getLength()) {
	  error(errSyntaxError, -1,
		"Unexpected end of PostScript function stream");
	  return gFalse;
	}
	tok = (GString *)tokens->get((*tokPtr)++);
	if (!tok->cmp("ifelse")) {
	  code[codePtr1].val.i = *codePtr;
	} else {
	  error(errSyntaxError, -1,
		"Expected 'ifelse' in PostScript function stream");
	  return gFalse;
	}
      } else {
	error(errSyntaxError, -1,
	      "Expected 'if' in PostScript function stream");
	return gFalse;
      }
    } else if (!tok->cmp("}")) {
      break;
    } else if (!tok->cmp("if")) {
      error(errSyntaxError, -1,
	    "Unexpected 'if' in PostScript function stream");
      return gFalse;
    } else if (!tok->cmp("ifelse")) {
      error(errSyntaxError, -1,
	    "Unexpected 'ifelse' in PostScript function stream");
      return gFalse;
    } else {
      a = -1;
      b = nPSOps;
      cmp = 0; // make gcc happy
      // invariant: psOpNames[a] < tok < psOpNames[b]
      while (b - a > 1) {
	mid = (a + b) / 2;
	cmp = tok->cmp(psOpNames[mid]);
	if (cmp > 0) {
	  a = mid;
	} else if (cmp < 0) {
	  b = mid;
	} else {
	  a = b = mid;
	}
      }
      if (cmp != 0) {
	error(errSyntaxError, -1,
	      "Unknown operator '{0:t}' in PostScript function",
	      tok);
	return gFalse;
      }
      addCode(codePtr, a);
    }
  }
  return gTrue;
}

void PostScriptFunction::addCode(int *codePtr, int op) {
  if (*codePtr >= codeSize) {
    if (codeSize) {
      codeSize *= 2;
    } else {
      codeSize = 16;
    }
    code = (PSCode *)greallocn(code, codeSize, sizeof(PSCode));
  }
  code[*codePtr].op = op;
  ++(*codePtr);
}

void PostScriptFunction::addCodeI(int *codePtr, int op, int x) {
  if (*codePtr >= codeSize) {
    if (codeSize) {
      codeSize *= 2;
    } else {
      codeSize = 16;
    }
    code = (PSCode *)greallocn(code, codeSize, sizeof(PSCode));
  }
  code[*codePtr].op = op;
  code[*codePtr].val.i = x;
  ++(*codePtr);
}

void PostScriptFunction::addCodeD(int *codePtr, int op, double x) {
  if (*codePtr >= codeSize) {
    if (codeSize) {
      codeSize *= 2;
    } else {
      codeSize = 16;
    }
    code = (PSCode *)greallocn(code, codeSize, sizeof(PSCode));
  }
  code[*codePtr].op = op;
  code[*codePtr].val.d = x;
  ++(*codePtr);
}

GString *PostScriptFunction::getToken(Stream *str) {
  GString *s;
  int c;
  GBool comment;

  s = new GString();
  comment = gFalse;
  while (1) {
    if ((c = str->getChar()) == EOF) {
      delete s;
      return NULL;
    }
    codeString->append((char)c);
    if (comment) {
      if (c == '\x0a' || c == '\x0d') {
	comment = gFalse;
      }
    } else if (c == '%') {
      comment = gTrue;
    } else if (!isspace(c)) {
      break;
    }
  }
  if (c == '{' || c == '}') {
    s->append((char)c);
  } else if (isdigit(c) || c == '.' || c == '-') {
    while (1) {
      s->append((char)c);
      c = str->lookChar();
      if (c == EOF || !(isdigit(c) || c == '.' || c == '-')) {
	break;
      }
      str->getChar();
      codeString->append((char)c);
    }
  } else {
    while (1) {
      s->append((char)c);
      c = str->lookChar();
      if (c == EOF || !isalnum(c)) {
	break;
      }
      str->getChar();
      codeString->append((char)c);
    }
  }
  return s;
}

int PostScriptFunction::exec(double *stack, int sp0) {
  PSCode *c;
  double tmp[psStackSize];
  double t;
  int sp, ip, nn, k, i;

  sp = sp0;
  ip = 0;
  while (ip < codeLen) {
    c = &code[ip++];
    switch(c->op) {
    case psOpAbs:
      if (sp >= psStackSize) {
	goto underflow;
      }
      stack[sp] = fabs(stack[sp]);
      break;
    case psOpAdd:
      if (sp + 1 >= psStackSize) {
	goto underflow;
      }
      stack[sp + 1] = stack[sp + 1] + stack[sp];
      ++sp;
      break;
    case psOpAnd:
      if (sp + 1 >= psStackSize) {
	goto underflow;
      }
      stack[sp + 1] = (int)stack[sp + 1] & (int)stack[sp];
      ++sp;
      break;
    case psOpAtan:
      if (sp + 1 >= psStackSize) {
	goto underflow;
      }
      stack[sp + 1] = atan2(stack[sp + 1], stack[sp]);
      ++sp;
      break;
    case psOpBitshift:
      if (sp + 1 >= psStackSize) {
	goto underflow;
      }
      k = (int)stack[sp + 1];
      nn = (int)stack[sp];
      if (nn > 0) {
	stack[sp + 1] = k << nn;
      } else if (nn < 0) {
	stack[sp + 1] = k >> -nn;
      } else {
	stack[sp + 1] = k;
      }
      ++sp;
      break;
    case psOpCeiling:
      if (sp >= psStackSize) {
	goto underflow;
      }
      stack[sp] = ceil(stack[sp]);
      break;
    case psOpCopy:
      if (sp >= psStackSize) {
	goto underflow;
      }
      nn = (int)stack[sp++];
      if (nn < 0) {
	goto invalidArg;
      }
      if (sp + nn > psStackSize) {
	goto underflow;
      }
      if (sp - nn < 0) {
	goto overflow;
      }
      for (i = 0; i < nn; ++i) {
	stack[sp - nn + i] = stack[sp + i];
      }
      sp -= nn;
      break;
    case psOpCos:
      if (sp >= psStackSize) {
	goto underflow;
      }
      stack[sp] = cos(stack[sp]);
      break;
    case psOpCvi:
      if (sp >= psStackSize) {
	goto underflow;
      }
      stack[sp] = (int)stack[sp];
      break;
    case psOpCvr:
      if (sp >= psStackSize) {
	goto underflow;
      }
      break;
    case psOpDiv:
      if (sp + 1 >= psStackSize) {
	goto underflow;
      }
      stack[sp + 1] = stack[sp + 1] / stack[sp];
      ++sp;
      break;
    case psOpDup:
      if (sp >= psStackSize) {
	goto underflow;
      }
      if (sp < 1) {
	goto overflow;
      }
      stack[sp - 1] = stack[sp];
      --sp;
      break;
    case psOpEq:
      if (sp + 1 >= psStackSize) {
	goto underflow;
      }
      stack[sp + 1] = stack[sp + 1] == stack[sp] ? 1 : 0;
      ++sp;
      break;
    case psOpExch:
      if (sp + 1 >= psStackSize) {
	goto underflow;
      }
      t = stack[sp];
      stack[sp] = stack[sp + 1];
      stack[sp + 1] = t;
      break;
    case psOpExp:
      if (sp + 1 >= psStackSize) {
	goto underflow;
      }
      stack[sp + 1] = pow(stack[sp + 1], stack[sp]);
      ++sp;
      break;
    case psOpFalse:
      if (sp < 1) {
	goto overflow;
      }
      stack[sp - 1] = 0;
      --sp;
      break;
    case psOpFloor:
      if (sp >= psStackSize) {
	goto underflow;
      }
      stack[sp] = floor(stack[sp]);
      break;
    case psOpGe:
      if (sp + 1 >= psStackSize) {
	goto underflow;
      }
      stack[sp + 1] = stack[sp + 1] >= stack[sp] ? 1 : 0;
      ++sp;
      break;
    case psOpGt:
      if (sp + 1 >= psStackSize) {
	goto underflow;
      }
      stack[sp + 1] = stack[sp + 1] > stack[sp] ? 1 : 0;
      ++sp;
      break;
    case psOpIdiv:
      if (sp + 1 >= psStackSize) {
	goto underflow;
      }
      stack[sp + 1] = (int)stack[sp + 1] / (int)stack[sp];
      ++sp;
      break;
    case psOpIndex:
      if (sp >= psStackSize) {
	goto underflow;
      }
      k = (int)stack[sp];
      if (k < 0) {
	goto invalidArg;
      }
      if (sp + 1 + k >= psStackSize) {
	goto underflow;
      }
      stack[sp] = stack[sp + 1 + k];
      break;
    case psOpLe:
      if (sp + 1 >= psStackSize) {
	goto underflow;
      }
      stack[sp + 1] = stack[sp + 1] <= stack[sp] ? 1 : 0;
      ++sp;
      break;
    case psOpLn:
      if (sp >= psStackSize) {
	goto underflow;
      }
      stack[sp] = log(stack[sp]);
      break;
    case psOpLog:
      if (sp >= psStackSize) {
	goto underflow;
      }
      stack[sp] = log10(stack[sp]);
      break;
    case psOpLt:
      if (sp + 1 >= psStackSize) {
	goto underflow;
      }
      stack[sp + 1] = stack[sp + 1] < stack[sp] ? 1 : 0;
      ++sp;
      break;
    case psOpMod:
      if (sp + 1 >= psStackSize) {
	goto underflow;
      }
      stack[sp + 1] = (int)stack[sp + 1] % (int)stack[sp];
      ++sp;
      break;
    case psOpMul:
      if (sp + 1 >= psStackSize) {
	goto underflow;
      }
      stack[sp + 1] = stack[sp + 1] * stack[sp];
      ++sp;
      break;
    case psOpNe:
      if (sp + 1 >= psStackSize) {
	goto underflow;
      }
      stack[sp + 1] = stack[sp + 1] != stack[sp] ? 1 : 0;
      ++sp;
      break;
    case psOpNeg:
      if (sp >= psStackSize) {
	goto underflow;
      }
      stack[sp] = -stack[sp];
      break;
    case psOpNot:
      if (sp >= psStackSize) {
	goto underflow;
      }
      stack[sp] = stack[sp] == 0 ? 1 : 0;
      break;
    case psOpOr:
      if (sp + 1 >= psStackSize) {
	goto underflow;
      }
      stack[sp + 1] = (int)stack[sp + 1] | (int)stack[sp];
      ++sp;
      break;
    case psOpPop:
      if (sp >= psStackSize) {
	goto underflow;
      }
      ++sp;
      break;
    case psOpRoll:
      if (sp + 1 >= psStackSize) {
	goto underflow;
      }
      k = (int)stack[sp++];
      nn = (int)stack[sp++];
      if (nn < 0) {
	goto invalidArg;
      }
      if (sp + nn > psStackSize) {
	goto underflow;
      }
      if (k >= 0) {
	k %= nn;
      } else {
	k = -k % nn;
	if (k) {
	  k = nn - k;
	}
      }
      for (i = 0; i < nn; ++i) {
	tmp[i] = stack[sp + i];
      }
      for (i = 0; i < nn; ++i) {
	stack[sp + i] = tmp[(i + k) % nn];
      }
      break;
    case psOpRound:
      if (sp >= psStackSize) {
	goto underflow;
      }
      t = stack[sp];
      stack[sp] = (t >= 0) ? floor(t + 0.5) : ceil(t - 0.5);
      break;
    case psOpSin:
      if (sp >= psStackSize) {
	goto underflow;
      }
      stack[sp] = sin(stack[sp]);
      break;
    case psOpSqrt:
      if (sp >= psStackSize) {
	goto underflow;
      }
      stack[sp] = sqrt(stack[sp]);
      break;
    case psOpSub:
      if (sp + 1 >= psStackSize) {
	goto underflow;
      }
      stack[sp + 1] = stack[sp + 1] - stack[sp];
      ++sp;
      break;
    case psOpTrue:
      if (sp < 1) {
	goto overflow;
      }
      stack[sp - 1] = 1;
      --sp;
      break;
    case psOpTruncate:
      if (sp >= psStackSize) {
	goto underflow;
      }
      t = stack[sp];
      stack[sp] = (t >= 0) ? floor(t) : ceil(t);
      break;
    case psOpXor:
      if (sp + 1 >= psStackSize) {
	goto underflow;
      }
      stack[sp + 1] = (int)stack[sp + 1] ^ (int)stack[sp];
      ++sp;
      break;
    case psOpPush:
      if (sp < 1) {
	goto overflow;
      }
      stack[--sp] = c->val.d;
      break;
    case psOpJ:
      ip = c->val.i;
      break;
    case psOpJz:
      if (sp >= psStackSize) {
	goto underflow;
      }
      k = (int)stack[sp++];
      if (k == 0) {
	ip = c->val.i;
      }
      break;
    }
  }
  return sp;

 underflow:
  error(errSyntaxError, -1, "Stack underflow in PostScript function");
  return sp;
 overflow:
  error(errSyntaxError, -1, "Stack overflow in PostScript function");
  return sp;
 invalidArg:
  error(errSyntaxError, -1, "Invalid arg in PostScript function");
  return sp;
}
