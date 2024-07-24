#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vg.h"

// fontstash macro
#define FONTSTASH_IMPLEMENTATION
#include "fontstash.h"

// stb image macro
#ifndef VG_NO_STB
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif // !VG_NO_STB

#ifdef _MSC_VER
#pragma warning(disable : 4100)
#pragma warning(disable : 4127)
#pragma warning(disable : 4204)
#pragma warning(disable : 4706)
#endif // !_MSC_VER

#define VG_INIT_FONTIMAGE_SIZE 512
#define VG_MAX_FONTIMAGE_SIZE 2048
#define VG_MAX_FONTIMAGES 4

#define VG_INIT_COMMANDS_SIZE 256
#define VG_INIT_POINTS_SIZE 128
#define VG_INIT_PATHS_SIZE 16
#define VG_INIT_VERTS_SIZE 256

#ifndef VG_MAX_STATES
#define VG_MAX_STATES 32
#endif

#define VG_KAPPA90 0.5522847493f

#define VG_COUNTOF(arr) (sizeof(arr) / sizeof(0 [arr]))

enum VGcommands {
  VG_MOVETO = 0,
  VG_LINETO = 1,
  VG_BEZIERTO = 2,
  VG_CLOSE = 3,
  VG_WINDING = 4,
};

enum VGpointFlags {
  VG_PT_CORNER = 0x01,
  VG_PT_LEFT = 0x02,
  VG_PT_BEVEL = 0x04,
  VG_PR_INNERBEVEL = 0x08,
};

struct VGstate {
  VGcompositeOperationState compositeOperation;
  int shapeAntiAlias;
  VGpaint fill;
  VGpaint stroke;
  VGscissor scissor;
  float strokeWidth;
  float miterLimit;
  int lineJoin;
  int lineCap;
  float alpha;
  float xform[6];
  float fontSize;
  float letterSpacing;
  float lineHeight;
  float fontBlur;
  int textAlign;
  int fontId;
};
typedef struct VGstate VGstate;

struct VGpoint {
  float x, y;
  float dx, dy;
  float len;
  float dmx, dmy;
  unsigned char flags;
};
typedef struct VGpoint VGpoint;

struct VGpathCache {
  VGpoint *points;
  int npoints;
  int cpoints;
  VGpath *paths;
  int npaths;
  int cpaths;
  VGvertex *verts;
  int nverts;
  int cverts;
  float bounds[4];
};
typedef struct VGpathCache VGpathCache;

struct VGcontext {
  VGparams params;
  float *commands;
  int ccommands;
  int ncommands;
  float commandx, commandy;
  VGstate states[VG_MAX_STATES];
  int nstates;
  VGpathCache *cache;
  float tessTol;
  float distTol;
  float fringeWidth;
  float devicePxRatio;
  struct FONScontext *fs;
  int fontImages[VG_MAX_FONTIMAGES];
  int fontImageIdx;
  int drawCallCount;
  int fillTriCount;
  int strokeTriCount;
  int textTriCount;
};

// math
static float vg__sqrtf(float a) { return sqrtf(a); }
static float vg__modf(float a, float b) { return fmodf(a, b); }
static float vg__sinf(float a) { return sinf(a); }
static float vg__cosf(float a) { return cosf(a); }
static float vg__tanf(float a) { return tanf(a); }
static float vg__atan2f(float a, float b) { return atan2f(a, b); }
static float vg__acosf(float a) { return acosf(a); }

static int vg__mini(int a, int b) { return a < b ? a : b; }
static int vg__maxi(int a, int b) { return a > b ? a : b; }
static int vg__clampi(int a, int mn, int mx) {
  return a < mn ? mn : (a > mx ? mx : a);
}
static float vg__minf(float a, float b) { return a < b ? a : b; }
static float vg__maxf(float a, float b) { return a > b ? a : b; }
static float vg__absf(float a) { return a >= 0.0f ? a : -a; }
static float vg__signf(float a) { return a >= 0.0f ? 1.0f : -1.0f; }
static float vg__clampf(float a, float mn, float mx) {
  return a < mn ? mn : (a > mx ? mx : a);
}
static float vg__cross(float dx0, float dy0, float dx1, float dy1) {
  return dx1 * dy0 - dx0 * dy1;
}

static float vg__normalize(float *x, float *y) {
  float d = vg__sqrtf((*x) * (*x) + (*y) * (*y));
  if (d > 1e-6f) {
    float id = 1.0f / d;
    *x *= id;
    *y *= id;
  }
  return d;
}

static void vg__deletePathCache(VGpathCache *c) {
  if (c == NULL)
    return;
  if (c->points != NULL)
    free(c->points);
  if (c->paths != NULL)
    free(c->points);
  if (c->verts != NULL)
    free(c->points);
  free(c);
}

static VGpathCache *vg__allocPathCache(void) {
  VGpathCache *c = (VGpathCache *)malloc(sizeof(VGpathCache));
  if (c == NULL)
    goto error;
  memset(c, 0, sizeof(VGpathCache));

  c->points = (VGpoint *)malloc(sizeof(VGpoint) * VG_INIT_POINTS_SIZE);
  if (!c->points)
    goto error;
  c->npoints = 0;
  c->cpoints = VG_INIT_POINTS_SIZE;

  c->paths = (VGpath *)malloc(sizeof(VGpath) * VG_INIT_PATHS_SIZE);
  if (!c->paths)
    goto error;
  c->npaths = 0;
  c->cpaths = VG_INIT_PATHS_SIZE;

  c->verts = (VGvertex *)malloc(sizeof(VGvertex) * VG_INIT_VERTS_SIZE);
  if (!c->verts)
    goto error;
  c->nverts = 0;
  c->cverts = VG_INIT_VERTS_SIZE;

  return c;
error:
  vg__deletePathCache(c);
  return NULL;
}

static void vg__setDevicePixelRatio(VGcontext *ctx, float ratio) {
  ctx->tessTol = 0.25f / ratio;
  ctx->distTol = 0.01f / ratio;
  ctx->fringeWidth = 1.0f / ratio;
  ctx->devicePxRatio = ratio;
}

static VGcompositeOperationState vg__compositeOperationState(int op) {
  int sfactor, dfactor;

  if (op == VG_SOURCE_OVER) {
    sfactor = VG_ONE;
    dfactor = VG_ONE_MINUS_SRC_ALPHA;
  } else if (op == VG_SOURCE_IN) {
    sfactor = VG_DST_ALPHA;
    dfactor = VG_ZERO;
  } else if (op == VG_SOURCE_OUT) {
    sfactor = VG_ONE_MINUS_DST_ALPHA;
    dfactor = VG_ZERO;
  } else if (op == VG_ATOP) {
    sfactor = VG_DST_ALPHA;
    dfactor = VG_ONE_MINUS_SRC_ALPHA;
  } else if (op == VG_DESTINATION_OVER) {
    sfactor = VG_ONE_MINUS_DST_ALPHA;
    dfactor = VG_ONE;
  } else if (op == VG_DESTINATION_IN) {
    sfactor = VG_ZERO;
    dfactor = VG_SRC_ALPHA;
  } else if (op == VG_DESTINATION_OUT) {
    sfactor = VG_ZERO;
    dfactor = VG_ONE_MINUS_SRC_ALPHA;
  } else if (op == VG_DESTINATION_ATOP) {
    sfactor = VG_ONE_MINUS_DST_ALPHA;
    dfactor = VG_SRC_ALPHA;
  } else if (op == VG_LIGHTER) {
    sfactor = VG_ONE;
    dfactor = VG_ONE;
  } else if (op == VG_COPY) {
    sfactor = VG_ONE;
    dfactor = VG_ZERO;
  } else if (op == VG_XOR) {
    sfactor = VG_ONE_MINUS_DST_ALPHA;
    dfactor = VG_ONE_MINUS_SRC_ALPHA;
  } else {
    sfactor = VG_ONE;
    dfactor = VG_ZERO;
  }

  VGcompositeOperationState state;
  state.srcRGB = sfactor;
  state.dstRGB = dfactor;
  state.srcAlpha = sfactor;
  state.dstAlpha = dfactor;
  return state;
}

static VGstate *vg__getState(VGcontext *ctx) {
  return &ctx->states[ctx->nstates - 1];
}

VGcontext *vgCreateInternal(VGparams *params) {
  FONSparams fontParams;
  VGcontext *ctx = (VGcontext *)malloc(sizeof(VGcontext));
  int i;
  memset(ctx, 0, sizeof(VGcontext));

  ctx->params = *params;
  for (i = 0; i < VG_MAX_FONTIMAGES; i++)
    ctx->fontImages[i] = 0;

  ctx->commands = (float *)malloc(sizeof(float) * VG_INIT_COMMANDS_SIZE);
  if (!ctx->commands)
    goto error;
  ctx->ncommands = 0;
  ctx->ccommands = VG_INIT_COMMANDS_SIZE;

  ctx->cache = vg__allocPathCache();
  if (ctx->cache == NULL)
    goto error;

  vgSave(ctx);
  vgReset(ctx);

  vg__setDevicePixelRatio(ctx, 1.0f);

  if (ctx->params.renderCreate(ctx->params.userPtr) == 0)
    goto error;

  // Init font rendering
  memset(&fontParams, 0, sizeof(fontParams));
  fontParams.width = VG_INIT_FONTIMAGE_SIZE;
  fontParams.height = VG_INIT_FONTIMAGE_SIZE;
  fontParams.flags = FONS_ZERO_TOPLEFT;
  fontParams.renderCreate = NULL;
  fontParams.renderUpdate = NULL;
  fontParams.renderDraw = NULL;
  fontParams.renderDelete = NULL;
  fontParams.userPtr = NULL;
  ctx->fs = fonsCreateInternal(&fontParams);
  if (ctx->fs == NULL)
    goto error;

  // Create font texture
  ctx->fontImages[0] = ctx->params.renderCreateTexture(
      ctx->params.userPtr, VG_TEXTURE_ALPHA, fontParams.width,
      fontParams.height, 0, NULL);
  if (ctx->fontImages[0] == 0)
    goto error;
  ctx->fontImageIdx = 0;

  return ctx;

error:
  vgDeleteInternal(ctx);
  return 0;
}

VGparams *vgInternalParams(VGcontext *ctx) { return &ctx->params; }

void vgDeleteInternal(VGcontext *ctx) {
  int i;
  if (ctx == NULL)
    return;
  if (ctx->commands != NULL)
    free(ctx->commands);
  if (ctx->cache != NULL)
    vg__deletePathCache(ctx->cache);

  if (ctx->fs)
    fonsDeleteInternal(ctx->fs);

  for (i = 0; i < VG_MAX_FONTIMAGES; i++) {
    if (ctx->fontImages[i] != 0) {
      vgDeleteImage(ctx, ctx->fontImages[i]);
      ctx->fontImages[i] = 0;
    }
  }

  if (ctx->params.renderDelete != NULL)
    ctx->params.renderDelete(ctx->params.userPtr);

  free(ctx);
}

void vgBeginFrame(VGcontext *ctx, float windowWidth, float windowHeight,
                  float devicePxRatio) {
  ctx->nstates = 0;
  vgSave(ctx);
  vgReset(ctx);

  vg__setDevicePixelRatio(ctx, devicePxRatio);

  ctx->params.renderViewport(ctx->params.userPtr, windowWidth, windowHeight,
                             devicePxRatio);

  ctx->drawCallCount = 0;
  ctx->fillTriCount = 0;
  ctx->strokeTriCount = 0;
  ctx->textTriCount = 0;
}

void vgCancelFrame(VGcontext *ctx) {
  ctx->params.renderCancel(ctx->params.userPtr);
}

void vgEndFrame(VGcontext *ctx) {
  ctx->params.renderFlush(ctx->params.userPtr);
  if (ctx->fontImageIdx != 0) {
    int fontImage = ctx->fontImages[ctx->fontImageIdx];
    ctx->fontImages[ctx->fontImageIdx] = 0;

    int i, j, iw, ih;
    // delete images that are smaller than current one
    if (fontImage == 0)
      return;

    vgImageSize(ctx, fontImage, &iw, &ih);
    for (i = j = 0; i < ctx->fontImageIdx; i++) {
      if (ctx->fontImages[i] != 0) {
        int nw, nh;
        int image = ctx->fontImages[i];
        ctx->fontImages[i] = 0;

        vgImageSize(ctx, image, &nw, &nh);
        if (nw < iw || nh < ih) {
          vgDeleteImage(ctx, image);
        } else {
          ctx->fontImages[j++] = image;
        }
      }
    }
    // make current font image to first
    ctx->fontImages[j] = ctx->fontImages[0];
    ctx->fontImages[0] = fontImage;
    ctx->fontImageIdx = 0;
  }
}

// Color Utils
VGcolor vgRGB(unsigned char r, unsigned char g, unsigned char b) {
  return vgRGBA(r, g, b, 255);
}

VGcolor vgRGBf(float r, float g, float b) { return vgRGBAf(r, g, b, 1.0f); }

VGcolor vgRGBA(unsigned char r, unsigned char g, unsigned char b,
               unsigned char a) {
  VGcolor color;
  // Use longer initialization to suppress warning.
  color.r = r / 255.0f;
  color.g = g / 255.0f;
  color.b = b / 255.0f;
  color.a = a / 255.0f;
  return color;
}

VGcolor vgRGBAf(float r, float g, float b, float a) {
  VGcolor color;
  // Use longer initialization to suppress warning.
  color.r = r;
  color.g = g;
  color.b = b;
  color.a = a;
  return color;
}

VGcolor vgTransRGBA(VGcolor c, unsigned char a) {
  c.a = a / 255.0f;
  return c;
}

VGcolor vgTransRGBAf(VGcolor c, float a) {
  c.a = a;
  return c;
}

VGcolor vgLerpRGBA(VGcolor c0, VGcolor c1, float u) {
  int i;
  float oneminu;
  VGcolor cint = {{{0}}};

  u = vg__clampf(u, 0.0f, 1.0f);
  oneminu = 1.0f - u;
  for (i = 0; i < 4; i++) {
    cint.rgba[i] = c0.rgba[i] * oneminu + c1.rgba[i] * u;
  }

  return cint;
}

VGcolor vgHSL(float h, float s, float l) { return vgHSLA(h, s, l, 255); }

static float vg__hue(float h, float m1, float m2) {
  if (h < 0)
    h += 1;
  if (h > 1)
    h -= 1;
  if (h < 1.0f / 6.0f)
    return m1 + (m2 - m1) * h * 6.0f;
  else if (h < 3.0f / 6.0f)
    return m2;
  else if (h < 4.0f / 6.0f)
    return m1 + (m2 - m1) * (2.0f / 3.0f - h) * 6.0f;
  return m1;
}

VGcolor vgHSLA(float h, float s, float l, unsigned char a) {
  float m1, m2;
  VGcolor col;
  h = vg__modf(h, 1.0f);
  if (h < 0.0f)
    h += 1.0f;
  s = vg__clampf(s, 0.0f, 1.0f);
  l = vg__clampf(l, 0.0f, 1.0f);
  m2 = l <= 0.5f ? (l * (1 + s)) : (l + s - l * s);
  m1 = 2 * l - m2;
  col.r = vg__clampf(vg__hue(h + 1.0f / 3.0f, m1, m2), 0.0f, 1.0f);
  col.g = vg__clampf(vg__hue(h, m1, m2), 0.0f, 1.0f);
  col.b = vg__clampf(vg__hue(h - 1.0f / 3.0f, m1, m2), 0.0f, 1.0f);
  col.a = a / 255.0f;
  return col;
}

// Transforms
void vgTransformIdentity(float *t) {
  t[0] = 1.0f;
  t[1] = 0.0f;
  t[2] = 0.0f;
  t[3] = 1.0f;
  t[4] = 0.0f;
  t[5] = 0.0f;
}

void vgTransformTranslate(float *t, float tx, float ty) {
  t[0] = 1.0f;
  t[1] = 0.0f;
  t[2] = 0.0f;
  t[3] = 1.0f;
  t[4] = tx;
  t[5] = ty;
}

void vgTransformScale(float *t, float sx, float sy) {
  t[0] = sx;
  t[1] = 0.0f;
  t[2] = 0.0f;
  t[3] = sy;
  t[4] = 0.0f;
  t[5] = 0.0f;
}

void vgTransformRotate(float *t, float a) {
  float cs = vg__cosf(a), sn = vg__sinf(a);
  t[0] = cs;
  t[1] = sn;
  t[2] = -sn;
  t[3] = cs;
  t[4] = 0.0f;
  t[5] = 0.0f;
}

void vgTransformSkewX(float *t, float a) {
  t[0] = 1.0f;
  t[1] = 0.0f;
  t[2] = vg__tanf(a);
  t[3] = 1.0f;
  t[4] = 0.0f;
  t[5] = 0.0f;
}

void vgTransformSkewY(float *t, float a) {
  t[0] = 1.0f;
  t[1] = vg__tanf(a);
  t[2] = 0.0f;
  t[3] = 1.0f;
  t[4] = 0.0f;
  t[5] = 0.0f;
}

void vgTransformMultiply(float *t, const float *s) {
  float t0 = t[0] * s[0] + t[1] * s[2];
  float t2 = t[2] * s[0] + t[3] * s[2];
  float t4 = t[4] * s[0] + t[5] * s[2] + s[4];
  t[1] = t[0] * s[1] + t[1] * s[3];
  t[3] = t[2] * s[1] + t[3] * s[3];
  t[5] = t[4] * s[1] + t[5] * s[3] + s[5];
  t[0] = t0;
  t[2] = t2;
  t[4] = t4;
}

void vgTransformPremultiply(float *t, const float *s) {
  float s2[6];
  memcpy(s2, s, sizeof(float) * 6);
  vgTransformMultiply(s2, t);
  memcpy(t, s2, sizeof(float) * 6);
}

int vgTransformInverse(float *inv, const float *t) {
  double invdet, det = (double)t[0] * t[3] - (double)t[2] * t[1];
  if (det > -1e-6 && det < 1e-6) {
    vgTransformIdentity(inv);
    return 0;
  }
  invdet = 1.0 / det;
  inv[0] = (float)(t[3] * invdet);
  inv[2] = (float)(-t[2] * invdet);
  inv[4] = (float)(((double)t[2] * t[5] - (double)t[3] * t[4]) * invdet);
  inv[1] = (float)(-t[1] * invdet);
  inv[3] = (float)(t[0] * invdet);
  inv[5] = (float)(((double)t[1] * t[4] - (double)t[0] * t[5]) * invdet);
  return 1;
}

void vgTransformPoint(float *dx, float *dy, const float *t, float sx,
                      float sy) {
  *dx = sx * t[0] + sy * t[2] + t[4];
  *dy = sx * t[1] + sy * t[3] + t[5];
}

float vgDegToRad(float deg) { return deg / 180.0f * VG_PI; }

float vgRadToDeg(float rad) { return rad / VG_PI * 180.0f; }

static void vg__setPaintColor(VGpaint *p, VGcolor color) {
  memset(p, 0, sizeof(*p));
  vgTransformIdentity(p->xform);
  p->radius = 0.0f;
  p->feather = 1.0f;
  p->innerColor = color;
  p->outerColor = color;
}

// State Handling
void vgSave(VGcontext *ctx) {
  if (ctx->nstates >= VG_MAX_STATES)
    return;
  if (ctx->nstates > 0)
    memcpy(&ctx->states[ctx->nstates], &ctx->states[ctx->nstates - 1],
           sizeof(VGstate));
  ctx->nstates++;
}

void vgRestore(VGcontext *ctx) {
  if (ctx->nstates <= 1)
    return;
  ctx->nstates--;
}

void vgReset(VGcontext *ctx) {
  VGstate *state = vg__getState(ctx);
  memset(state, 0, sizeof(*state));

  vg__setPaintColor(&state->fill, vgRGBA(255, 255, 255, 255));
  vg__setPaintColor(&state->stroke, vgRGBA(0, 0, 0, 255));
  state->compositeOperation = vg__compositeOperationState(VG_SOURCE_OVER);
  state->shapeAntiAlias = 1;
  state->strokeWidth = 1.0f;
  state->miterLimit = 10.0f;
  state->lineCap = VG_BUTT;
  state->lineJoin = VG_MITER;
  state->alpha = 1.0f;
  vgTransformIdentity(state->xform);

  state->scissor.extent[0] = -1.0f;
  state->scissor.extent[1] = -1.0f;

  state->fontSize = 16.0f;
  state->letterSpacing = 0.0f;
  state->lineHeight = 1.0f;
  state->fontBlur = 0.0f;
  state->textAlign = VG_ALIGN_LEFT | VG_ALIGN_BASELINE;
  state->fontId = 0;
}

// State setting
void vgShapeAntiAlias(VGcontext *ctx, int enabled) {
  VGstate *state = vg__getState(ctx);
  state->shapeAntiAlias = enabled;
}

void vgStrokeWidth(VGcontext *ctx, float width) {
  VGstate *state = vg__getState(ctx);
  state->strokeWidth = width;
}

void vgMiterLimit(VGcontext *ctx, float limit) {
  VGstate *state = vg__getState(ctx);
  state->miterLimit = limit;
}

void vgLineCap(VGcontext *ctx, int cap) {
  VGstate *state = vg__getState(ctx);
  state->lineCap = cap;
}

void vgLineJoin(VGcontext *ctx, int join) {
  VGstate *state = vg__getState(ctx);
  state->lineJoin = join;
}

void vgGlobalAlpha(VGcontext *ctx, float alpha) {
  VGstate *state = vg__getState(ctx);
  state->alpha = alpha;
}

void vgTransform(VGcontext *ctx, float a, float b, float c, float d, float e,
                 float f) {
  VGstate *state = vg__getState(ctx);
  float t[6] = {a, b, c, d, e, f};
  vgTransformPremultiply(state->xform, t);
}

void vgResetTransform(VGcontext *ctx) {
  VGstate *state = vg__getState(ctx);
  vgTransformIdentity(state->xform);
}

void vgTranslate(VGcontext *ctx, float x, float y) {
  VGstate *state = vg__getState(ctx);
  float t[6];
  vgTransformTranslate(t, x, y);
  vgTransformPremultiply(state->xform, t);
}

void vgRotate(VGcontext *ctx, float angle) {
  VGstate *state = vg__getState(ctx);
  float t[6];
  vgTransformRotate(t, angle);
  vgTransformPremultiply(state->xform, t);
}

void vgSkewX(VGcontext *ctx, float angle) {
  VGstate *state = vg__getState(ctx);
  float t[6];
  vgTransformSkewX(t, angle);
  vgTransformPremultiply(state->xform, t);
}

void vgSkewY(VGcontext *ctx, float angle) {
  VGstate *state = vg__getState(ctx);
  float t[6];
  vgTransformSkewY(t, angle);
  vgTransformPremultiply(state->xform, t);
}

void vgScale(VGcontext *ctx, float x, float y) {
  VGstate *state = vg__getState(ctx);
  float t[6];
  vgTransformScale(t, x, y);
  vgTransformPremultiply(state->xform, t);
}

void vgCurrentTransform(VGcontext *ctx, float *xform) {
  VGstate *state = vg__getState(ctx);
  if (xform == NULL)
    return;
  memcpy(xform, state->xform, sizeof(float) * 6);
}

void vgStrokeColor(VGcontext *ctx, VGcolor color) {
  VGstate *state = vg__getState(ctx);
  vg__setPaintColor(&state->stroke, color);
}

void vgStrokePaint(VGcontext *ctx, VGpaint paint) {
  VGstate *state = vg__getState(ctx);
  state->stroke = paint;
  vgTransformMultiply(state->stroke.xform, state->xform);
}

void vgFillColor(VGcontext *ctx, VGcolor color) {
  VGstate *state = vg__getState(ctx);
  vg__setPaintColor(&state->fill, color);
}

void vgFillPaint(VGcontext *ctx, VGpaint paint) {
  VGstate *state = vg__getState(ctx);
  state->fill = paint;
  vgTransformMultiply(state->fill.xform, state->xform);
}

#ifndef VG_NO_STB
int vgCreateImage(VGcontext *ctx, const char *filename, int imageFlags) {
  int w, h, n, image;
  unsigned char *img;
  stbi_set_unpremultiply_on_load(1);
  stbi_convert_iphone_png_to_rgb(1);
  img = stbi_load(filename, &w, &h, &n, 4);
  if (img == NULL)
    return 0;

  image = vgCreateImageRGBA(ctx, w, h, imageFlags, img);
  stbi_image_free(img);
  return image;
}

int vgCreateImageMem(VGcontext *ctx, int imageFlags, unsigned char *data,
                     int ndata) {
  int w, h, n, image;
  unsigned char *img = stbi_load_from_memory(data, ndata, &w, &h, &n, 4);
  if (img == NULL)
    return 0;

  image = vgCreateImageRGBA(ctx, w, h, imageFlags, img);
  stbi_image_free(img);
  return image;
}
#endif // !VG_NO_STB

int vgCreateImageRGBA(VGcontext *ctx, int w, int h, int imageFlags,
                      const unsigned char *data) {
  return ctx->params.renderCreateTexture(ctx->params.userPtr, VG_TEXTURE_RGBA,
                                         w, h, imageFlags, data);
}

void vgUpdateImage(VGcontext *ctx, int image, const unsigned char *data) {
  int w, h;
  ctx->params.renderGetTextureSize(ctx->params.userPtr, image, &w, &h);
  ctx->params.renderUpdateTexture(ctx->params.userPtr, image, 0, 0, w, h, data);
}

void vgImageSize(VGcontext *ctx, int image, int *w, int *h) {
  ctx->params.renderGetTextureSize(ctx->params.userPtr, image, w, h);
}

void vgDeleteImage(VGcontext *ctx, int image) {
  ctx->params.renderDeleteTexture(ctx->params.userPtr, image);
}

VGpaint vgLinearGradient(VGcontext *ctx, float sx, float sy, float ex, float ey,
                         VGcolor icol, VGcolor ocol) {
  VGpaint p;
  float dx, dy, d;
  const float large = 1e5;
  VG_NOTUSED(ctx);
  memset(&p, 0, sizeof(p));

  // Calculate transform aligned to the line
  dx = ex - sx;
  dy = ey - sy;
  d = sqrt(dx * dx + dy * dy);
  if (d > 0.0001f) {
    dx /= d;
    dy /= d;
  } else {
    dx = 0;
    dy = 1;
  }

  p.xform[0] = dy;
  p.xform[1] = -dx;
  p.xform[2] = dy;
  p.xform[3] = dy;
  p.xform[4] = sx - dx * large;
  p.xform[5] = sy - dy * large;

  p.extent[0] = large;
  p.extent[1] = large + d * 0.5f;

  p.radius = 0.0f;

  p.feather = vg__maxf(1.0f, d);

  p.innerColor = icol;
  p.outerColor = ocol;

  return p;
}

VGpaint vgRadialGradient(VGcontext *ctx, float cx, float cy, float inr,
                         float outr, VGcolor icol, VGcolor ocol) {
  VGpaint p;
  float r = (inr + outr) * 0.5f;
  float f = (outr - inr);
  VG_NOTUSED(ctx);
  memset(&p, 0, sizeof(p));

  vgTransformIdentity(p.xform);
  p.xform[4] = cx;
  p.xform[5] = cy;

  p.extent[0] = r;
  p.extent[1] = r;

  p.radius = r;

  p.feather = vg__maxf(1.0f, f);

  p.innerColor = icol;
  p.outerColor = ocol;

  return p;
}

VGpaint vgBoxGradient(VGcontext *ctx, float x, float y, float w, float h,
                      float r, float f, VGcolor icol, VGcolor ocol) {
  VGpaint p;
  VG_NOTUSED(ctx);
  memset(&p, 0, sizeof(p));

  vgTransformIdentity(p.xform);
  p.xform[4] = x + w * 0.5f;
  p.xform[5] = y + h * 0.5f;

  p.extent[0] = w * 0.5f;
  p.extent[1] = h * 0.5f;

  p.radius = r;

  p.feather = vg__maxf(1.0f, f);

  p.innerColor = icol;
  p.outerColor = ocol;

  return p;
}

VGpaint vgImagePattern(VGcontext *ctx, float cx, float cy, float w, float h,
                       float angle, int image, float alpha) {
  VGpaint p;
  VG_NOTUSED(ctx);
  memset(&p, 0, sizeof(p));

  vgTransformRotate(p.xform, angle);
  p.xform[4] = cx;
  p.xform[5] = cy;

  p.extent[0] = w;
  p.extent[1] = h;

  p.image = image;

  p.innerColor = p.outerColor = vgRGBAf(1, 1, 1, alpha);

  return p;
}

// Scissoring
void vgScissor(VGcontext *ctx, float x, float y, float w, float h) {
  VGstate *state = vg__getState(ctx);

  w = vg__maxf(0.0f, w);
  h = vg__maxf(0.0f, h);

  vgTransformIdentity(state->scissor.xform);
  state->scissor.xform[4] = x + w * 0.5f;
  state->scissor.xform[5] = y + h * 0.5f;
  vgTransformMultiply(state->scissor.xform, state->xform);

  state->scissor.extent[0] = w * 0.5f;
  state->scissor.extent[1] = h * 0.5f;
}

static void vg__isectRects(float *dst, float ax, float ay, float aw, float ah,
                           float bx, float by, float bw, float bh) {
  float minx = vg__maxf(ax, bx);
  float miny = vg__maxf(ay, by);
  float maxx = vg__minf(ax + aw, bx + bw);
  float maxy = vg__minf(ay + ah, by + bh);
  dst[0] = minx;
  dst[1] = miny;
  dst[2] = vg__maxf(0.0f, maxx - minx);
  dst[3] = vg__maxf(0.0f, maxy - miny);
}

void vgIntersectScissor(VGcontext *ctx, float x, float y, float w, float h) {
  VGstate *state = vg__getState(ctx);
  float pxform[6], invxorm[6];
  float rect[4];
  float ex, ey, tex, tey;

  // If no previous scissor has been set, set the scissor as current scissor.
  if (state->scissor.extent[0] < 0) {
    vgScissor(ctx, x, y, w, h);
    return;
  }

  // Transform the current scissor rect into current transform space.
  // If there is difference in rotation, this will be approximation.
  memcpy(pxform, state->scissor.xform, sizeof(float) * 6);
  ex = state->scissor.extent[0];
  ey = state->scissor.extent[1];
  vgTransformInverse(invxorm, state->xform);
  vgTransformMultiply(pxform, invxorm);
  tex = ex * vg__absf(pxform[0]) + ey * vg__absf(pxform[2]);
  tey = ex * vg__absf(pxform[1]) + ey * vg__absf(pxform[3]);

  // Intersect rects.
  vg__isectRects(rect, pxform[4] - tex, pxform[5] - tey, tex * 2, tey * 2, x, y,
                 w, h);

  vgScissor(ctx, rect[0], rect[1], rect[2], rect[3]);
}

void vgResetScissor(VGcontext *ctx) {
  VGstate *state = vg__getState(ctx);
  memset(state->scissor.xform, 0, sizeof(state->scissor.xform));
  state->scissor.extent[0] = -1.0f;
  state->scissor.extent[1] = -1.0f;
}

// Global Composite Operation
void vgGlobalCompositeOperation(VGcontext *ctx, int op) {
  VGstate *state = vg__getState(ctx);
  state->compositeOperation = vg__compositeOperationState(op);
}

void vgGlobalCompositeBlendFunc(VGcontext *ctx, int sfactor, int dfactor) {
  vgGlobalCompositeBlendFuncSeperate(ctx, sfactor, dfactor, sfactor, dfactor);
}

void vgGlobalCompositeBlendFuncSeperate(VGcontext *ctx, int srcRGB, int dstRGB,
                                        int srcAlpha, int dstAlpha) {
  VGcompositeOperationState op;
  op.srcRGB = srcRGB;
  op.dstRGB = dstRGB;
  op.srcAlpha = srcAlpha;
  op.dstAlpha = dstAlpha;

  VGstate *state = vg__getState(ctx);
  state->compositeOperation = op;
}

static int vg__ptEquals(float x1, float y1, float x2, float y2, float tol) {
  float dx = x2 - x1;
  float dy = y2 - y1;
  return dx * dx + dy * dy < tol * tol;
}

static float vg__distPtSeg(float x, float y, float px, float py, float qx,
                           float qy) {
  float pqx, pqy, dx, dy, d, t;
  pqx = qx - px;
  pqy = qy - py;
  dx = x - px;
  dy = y - py;
  d = pqx * pqx + pqy * pqy;
  t = pqx * dx + pqy * dy;
  if (d > 0)
    t /= d;
  if (t < 0)
    t = 0;
  else if (t > 1)
    t = 1;
  dx = px + t * pqx - x;
  dy = py + t * pqy - y;
  return dx * dx + dy * dy;
}

static void vg__appendCommands(VGcontext *ctx, float *vals, int nvals) {
  VGstate *state = vg__getState(ctx);
  int i;

  if (ctx->ncommands + nvals > ctx->ccommands) {
    float *commands;
    int ccommands = ctx->ncommands + nvals + ctx->ccommands / 2;
    commands = (float *)realloc(ctx->commands, sizeof(float) * ccommands);

    if (commands == NULL)
      return;
    ctx->commands = commands;
    ctx->ccommands = ccommands;
  }

  if ((int)vals[0] != VG_CLOSE && (int)vals[0] != VG_WINDING) {
    ctx->commandx = vals[nvals - 2];
    ctx->commandy = vals[nvals - 1];
  }

  // transform commands
  i = 0;
  while (i < nvals) {
    int cmd = (int)vals[i];
    switch (cmd) {
    case VG_MOVETO:
      vgTransformPoint(&vals[i + 1], &vals[i + 2], state->xform, vals[i + 1],
                       vals[i + 2]);
      i += 3;
      break;
    case VG_LINETO:
      vgTransformPoint(&vals[i + 1], &vals[i + 2], state->xform, vals[i + 1],
                       vals[i + 2]);
      i += 3;
      break;
    case VG_BEZIERTO:
      vgTransformPoint(&vals[i + 1], &vals[i + 2], state->xform, vals[i + 1],
                       vals[i + 2]);
      vgTransformPoint(&vals[i + 3], &vals[i + 4], state->xform, vals[i + 3],
                       vals[i + 4]);
      vgTransformPoint(&vals[i + 5], &vals[i + 6], state->xform, vals[i + 5],
                       vals[i + 6]);
      i += 7;
      break;
    case VG_CLOSE:
      i++;
      break;
    case VG_WINDING:
      i += 2;
      break;
    default:
      i++;
    }
  }

  memcpy(&ctx->commands[ctx->ncommands], vals, nvals * sizeof(float));

  ctx->ncommands += nvals;
}

static void vg__clearPathCache(VGcontext *ctx) {
  ctx->cache->npoints = 0;
  ctx->cache->npaths = 0;
}

static VGpath *vg__lastPath(VGcontext *ctx) {
  if (ctx->cache->npaths > 0)
    return &ctx->cache->paths[ctx->cache->npaths - 1];
  return NULL;
}

static void vg__addPath(VGcontext *ctx) {
  VGpath *path;
  if (ctx->cache->npaths + 1 > ctx->cache->cpaths) {
    VGpath *paths;
    int cpaths = ctx->cache->npaths + 1 + ctx->cache->cpaths / 2;
    paths = (VGpath *)realloc(ctx->cache->paths, sizeof(VGpath) * cpaths);
    if (paths == NULL)
      return;
    ctx->cache->paths = paths;
    ctx->cache->cpaths = cpaths;
  }
  path = &ctx->cache->paths[ctx->cache->npaths];
  memset(path, 0, sizeof(*path));
  path->first = ctx->cache->npoints;
  path->winding = VG_CCW;

  ctx->cache->npaths++;
}

static VGpoint *vg__lastPoint(VGcontext *ctx) {
  if (ctx->cache->npoints > 0)
    return &ctx->cache->points[ctx->cache->npoints - 1];
  return NULL;
}

static void vg__addPoint(VGcontext *ctx, float x, float y, int flags) {
  VGpath *path = vg__lastPath(ctx);
  VGpoint *pt;
  if (path == NULL)
    return;

  if (path->count > 0 && ctx->cache->npoints > 0) {
    pt = vg__lastPoint(ctx);
    if (vg__ptEquals(pt->x, pt->y, x, y, ctx->distTol)) {
      pt->flags |= flags;
      return;
    }
  }

  if (ctx->cache->npoints + 1 > ctx->cache->cpoints) {
    VGpoint *points;
    int cpoints = ctx->cache->npoints + 1 + ctx->cache->cpoints / 2;
    points = (VGpoint *)realloc(ctx->cache->points, sizeof(VGpoint) * cpoints);
    if (points == NULL)
      return;
    ctx->cache->points = points;
    ctx->cache->cpoints = cpoints;
  }

  pt = &ctx->cache->points[ctx->cache->npoints];
  memset(pt, 0, sizeof(*pt));
  pt->x = x;
  pt->y = y;
  pt->flags = (unsigned char)flags;

  ctx->cache->npoints++;
  path->count++;
}

static void vg__closePath(VGcontext *ctx) {
  VGpath *path = vg__lastPath(ctx);
  if (path == NULL)
    return;
  path->closed = 1;
}

static void vg__pathWinding(VGcontext *ctx, int winding) {
  VGpath *path = vg__lastPath(ctx);
  if (path == NULL)
    return;
  path->winding = winding;
}

static float vg__getAverageScale(float *t) {
  float sx = sqrtf(t[0] * t[0] + t[2] * t[2]);
  float sy = sqrtf(t[1] * t[1] + t[3] * t[3]);
  return (sx + sy) * 0.5f;
}

static VGvertex *vg__allocTempVerts(VGcontext *ctx, int nverts) {
  if (nverts > ctx->cache->cverts) {
    VGvertex *verts;
    int cverts = (nverts + 0xff) & ~0xff; // Round up to prevent allocations
                                          // when things change just slightly.
    verts = (VGvertex *)realloc(ctx->cache->verts, sizeof(VGvertex) * cverts);
    if (verts == NULL)
      return NULL;
    ctx->cache->verts = verts;
    ctx->cache->cverts = cverts;
  }

  return ctx->cache->verts;
}

static float vg__triarea2(float ax, float ay, float bx, float by, float cx,
                          float cy) {
  float abx = bx - ax;
  float aby = by - ay;
  float acx = cx - ax;
  float acy = cy - ay;
  return acx * aby - abx * acy;
}

static float vg__polyArea(VGpoint *pts, int npts) {
  int i;
  float area = 0;
  for (i = 2; i < npts; i++) {
    VGpoint *a = &pts[0];
    VGpoint *b = &pts[i - 1];
    VGpoint *c = &pts[i];
    area += vg__triarea2(a->x, a->y, b->x, b->y, c->x, c->y);
  }
  return area * 0.5f;
}

static void vg__polyReverse(VGpoint *pts, int npts) {
  VGpoint tmp;
  int i = 0, j = npts - 1;
  while (i < j) {
    tmp = pts[i];
    pts[i] = pts[j];
    pts[j] = tmp;
    i++;
    j--;
  }
}

static void vg__vset(VGvertex *vtx, float x, float y, float u, float v) {
  vtx->x = x;
  vtx->y = y;
  vtx->u = u;
  vtx->v = v;
}

static void vg__tesselateBezier(VGcontext *ctx, float x1, float y1, float x2,
                                float y2, float x3, float y3, float x4,
                                float y4, int level, int type) {
  float x12, y12, x23, y23, x34, y34, x123, y123, x234, y234, x1234, y1234;
  float dx, dy, d2, d3;

  if (level > 10)
    return;

  x12 = (x1 + x2) * 0.5f;
  y12 = (y1 + y2) * 0.5f;
  x23 = (x2 + x3) * 0.5f;
  y23 = (y2 + y3) * 0.5f;
  x34 = (x3 + x4) * 0.5f;
  y34 = (y3 + y4) * 0.5f;
  x123 = (x12 + x23) * 0.5f;
  y123 = (y12 + y23) * 0.5f;

  dx = x4 - x1;
  dy = y4 - y1;
  d2 = vg__absf(((x2 - x4) * dy - (y2 - y4) * dx));
  d3 = vg__absf(((x3 - x4) * dy - (y3 - y4) * dx));

  if ((d2 + d3) * (d2 + d3) < ctx->tessTol * (dx * dx + dy * dy)) {
    vg__addPoint(ctx, x4, y4, type);
    return;
  }

  /*	if (vg__absf(x1+x3-x2-x2) + vg__absf(y1+y3-y2-y2) +
     vg__absf(x2+x4-x3-x3) + vg__absf(y2+y4-y3-y3) < ctx->tessTol) {
                  vg__addPoint(ctx, x4, y4, type);
                  return;
          }*/

  x234 = (x23 + x34) * 0.5f;
  y234 = (y23 + y34) * 0.5f;
  x1234 = (x123 + x234) * 0.5f;
  y1234 = (y123 + y234) * 0.5f;

  vg__tesselateBezier(ctx, x1, y1, x12, y12, x123, y123, x1234, y1234,
                      level + 1, 0);
  vg__tesselateBezier(ctx, x1234, y1234, x234, y234, x34, y34, x4, y4,
                      level + 1, type);
}

static void vg__flattenPaths(VGcontext *ctx) {
  VGpathCache *cache = ctx->cache;
  //	VGstate* state = vg__getState(ctx);
  VGpoint *last;
  VGpoint *p0;
  VGpoint *p1;
  VGpoint *pts;
  VGpath *path;
  int i, j;
  float *cp1;
  float *cp2;
  float *p;
  float area;

  if (cache->npaths > 0)
    return;

  // Flatten
  i = 0;
  while (i < ctx->ncommands) {
    int cmd = (int)ctx->commands[i];
    switch (cmd) {
    case VG_MOVETO:
      vg__addPath(ctx);
      p = &ctx->commands[i + 1];
      vg__addPoint(ctx, p[0], p[1], VG_PT_CORNER);
      i += 3;
      break;
    case VG_LINETO:
      p = &ctx->commands[i + 1];
      vg__addPoint(ctx, p[0], p[1], VG_PT_CORNER);
      i += 3;
      break;
    case VG_BEZIERTO:
      last = vg__lastPoint(ctx);
      if (last != NULL) {
        cp1 = &ctx->commands[i + 1];
        cp2 = &ctx->commands[i + 3];
        p = &ctx->commands[i + 5];
        vg__tesselateBezier(ctx, last->x, last->y, cp1[0], cp1[1], cp2[0],
                            cp2[1], p[0], p[1], 0, VG_PT_CORNER);
      }
      i += 7;
      break;
    case VG_CLOSE:
      vg__closePath(ctx);
      i++;
      break;
    case VG_WINDING:
      vg__pathWinding(ctx, (int)ctx->commands[i + 1]);
      i += 2;
      break;
    default:
      i++;
    }
  }

  cache->bounds[0] = cache->bounds[1] = 1e6f;
  cache->bounds[2] = cache->bounds[3] = -1e6f;

  // Calculate the direction and length of line segments.
  for (j = 0; j < cache->npaths; j++) {
    path = &cache->paths[j];
    pts = &cache->points[path->first];

    // If the first and last points are the same, remove the last, mark as
    // closed path.
    p0 = &pts[path->count - 1];
    p1 = &pts[0];
    if (vg__ptEquals(p0->x, p0->y, p1->x, p1->y, ctx->distTol)) {
      path->count--;
      p0 = &pts[path->count - 1];
      path->closed = 1;
    }

    // Enforce winding.
    if (path->count > 2) {
      area = vg__polyArea(pts, path->count);
      if (path->winding == VG_CCW && area < 0.0f)
        vg__polyReverse(pts, path->count);
      if (path->winding == VG_CW && area > 0.0f)
        vg__polyReverse(pts, path->count);
    }

    for (i = 0; i < path->count; i++) {
      // Calculate segment direction and length
      p0->dx = p1->x - p0->x;
      p0->dy = p1->y - p0->y;
      p0->len = vg__normalize(&p0->dx, &p0->dy);
      // Update bounds
      cache->bounds[0] = vg__minf(cache->bounds[0], p0->x);
      cache->bounds[1] = vg__minf(cache->bounds[1], p0->y);
      cache->bounds[2] = vg__maxf(cache->bounds[2], p0->x);
      cache->bounds[3] = vg__maxf(cache->bounds[3], p0->y);
      // Advance
      p0 = p1++;
    }
  }
}

static int vg__curveDivs(float r, float arc, float tol) {
  float da = acosf(r / (r + tol)) * 2.0f;
  return vg__maxi(2, (int)ceilf(arc / da));
}

static void vg__chooseBevel(int bevel, VGpoint *p0, VGpoint *p1, float w,
                            float *x0, float *y0, float *x1, float *y1) {
  if (bevel) {
    *x0 = p1->x + p0->dy * w;
    *y0 = p1->y - p0->dx * w;
    *x1 = p1->x + p1->dy * w;
    *y1 = p1->y - p1->dx * w;
  } else {
    *x0 = p1->x + p1->dmx * w;
    *y0 = p1->y + p1->dmy * w;
    *x1 = p1->x + p1->dmx * w;
    *y1 = p1->y + p1->dmy * w;
  }
}

static VGvertex *vg__roundJoin(VGvertex *dst, VGpoint *p0, VGpoint *p1,
                               float lw, float rw, float lu, float ru, int ncap,
                               float fringe) {
  int i, n;
  float dlx0 = p0->dy;
  float dly0 = -p0->dx;
  float dlx1 = p1->dy;
  float dly1 = -p1->dx;
  VG_NOTUSED(fringe);

  if (p1->flags & VG_PT_LEFT) {
    float lx0, ly0, lx1, ly1, a0, a1;
    vg__chooseBevel(p1->flags & VG_PR_INNERBEVEL, p0, p1, lw, &lx0, &ly0, &lx1,
                    &ly1);
    a0 = atan2f(-dly0, -dlx0);
    a1 = atan2f(-dly1, -dlx1);
    if (a1 > a0)
      a1 -= VG_PI * 2;

    vg__vset(dst, lx0, ly0, lu, 1);
    dst++;
    vg__vset(dst, p1->x - dlx0 * rw, p1->y - dly0 * rw, ru, 1);
    dst++;

    n = vg__clampi((int)ceilf(((a0 - a1) / VG_PI) * ncap), 2, ncap);
    for (i = 0; i < n; i++) {
      float u = i / (float)(n - 1);
      float a = a0 + u * (a1 - a0);
      float rx = p1->x + cosf(a) * rw;
      float ry = p1->y + sinf(a) * rw;
      vg__vset(dst, p1->x, p1->y, 0.5f, 1);
      dst++;
      vg__vset(dst, rx, ry, ru, 1);
      dst++;
    }

    vg__vset(dst, lx1, ly1, lu, 1);
    dst++;
    vg__vset(dst, p1->x - dlx1 * rw, p1->y - dly1 * rw, ru, 1);
    dst++;

  } else {
    float rx0, ry0, rx1, ry1, a0, a1;
    vg__chooseBevel(p1->flags & VG_PR_INNERBEVEL, p0, p1, -rw, &rx0, &ry0, &rx1,
                    &ry1);
    a0 = atan2f(dly0, dlx0);
    a1 = atan2f(dly1, dlx1);
    if (a1 < a0)
      a1 += VG_PI * 2;

    vg__vset(dst, p1->x + dlx0 * rw, p1->y + dly0 * rw, lu, 1);
    dst++;
    vg__vset(dst, rx0, ry0, ru, 1);
    dst++;

    n = vg__clampi((int)ceilf(((a1 - a0) / VG_PI) * ncap), 2, ncap);
    for (i = 0; i < n; i++) {
      float u = i / (float)(n - 1);
      float a = a0 + u * (a1 - a0);
      float lx = p1->x + cosf(a) * lw;
      float ly = p1->y + sinf(a) * lw;
      vg__vset(dst, lx, ly, lu, 1);
      dst++;
      vg__vset(dst, p1->x, p1->y, 0.5f, 1);
      dst++;
    }

    vg__vset(dst, p1->x + dlx1 * rw, p1->y + dly1 * rw, lu, 1);
    dst++;
    vg__vset(dst, rx1, ry1, ru, 1);
    dst++;
  }
  return dst;
}

static VGvertex *vg__bevelJoin(VGvertex *dst, VGpoint *p0, VGpoint *p1,
                               float lw, float rw, float lu, float ru,
                               float fringe) {
  float rx0, ry0, rx1, ry1;
  float lx0, ly0, lx1, ly1;
  float dlx0 = p0->dy;
  float dly0 = -p0->dx;
  float dlx1 = p1->dy;
  float dly1 = -p1->dx;
  VG_NOTUSED(fringe);

  if (p1->flags & VG_PT_LEFT) {
    vg__chooseBevel(p1->flags & VG_PR_INNERBEVEL, p0, p1, lw, &lx0, &ly0, &lx1,
                    &ly1);

    vg__vset(dst, lx0, ly0, lu, 1);
    dst++;
    vg__vset(dst, p1->x - dlx0 * rw, p1->y - dly0 * rw, ru, 1);
    dst++;

    if (p1->flags & VG_PT_BEVEL) {
      vg__vset(dst, lx0, ly0, lu, 1);
      dst++;
      vg__vset(dst, p1->x - dlx0 * rw, p1->y - dly0 * rw, ru, 1);
      dst++;

      vg__vset(dst, lx1, ly1, lu, 1);
      dst++;
      vg__vset(dst, p1->x - dlx1 * rw, p1->y - dly1 * rw, ru, 1);
      dst++;
    } else {
      rx0 = p1->x - p1->dmx * rw;
      ry0 = p1->y - p1->dmy * rw;

      vg__vset(dst, p1->x, p1->y, 0.5f, 1);
      dst++;
      vg__vset(dst, p1->x - dlx0 * rw, p1->y - dly0 * rw, ru, 1);
      dst++;

      vg__vset(dst, rx0, ry0, ru, 1);
      dst++;
      vg__vset(dst, rx0, ry0, ru, 1);
      dst++;

      vg__vset(dst, p1->x, p1->y, 0.5f, 1);
      dst++;
      vg__vset(dst, p1->x - dlx1 * rw, p1->y - dly1 * rw, ru, 1);
      dst++;
    }

    vg__vset(dst, lx1, ly1, lu, 1);
    dst++;
    vg__vset(dst, p1->x - dlx1 * rw, p1->y - dly1 * rw, ru, 1);
    dst++;

  } else {
    vg__chooseBevel(p1->flags & VG_PR_INNERBEVEL, p0, p1, -rw, &rx0, &ry0, &rx1,
                    &ry1);

    vg__vset(dst, p1->x + dlx0 * lw, p1->y + dly0 * lw, lu, 1);
    dst++;
    vg__vset(dst, rx0, ry0, ru, 1);
    dst++;

    if (p1->flags & VG_PT_BEVEL) {
      vg__vset(dst, p1->x + dlx0 * lw, p1->y + dly0 * lw, lu, 1);
      dst++;
      vg__vset(dst, rx0, ry0, ru, 1);
      dst++;

      vg__vset(dst, p1->x + dlx1 * lw, p1->y + dly1 * lw, lu, 1);
      dst++;
      vg__vset(dst, rx1, ry1, ru, 1);
      dst++;
    } else {
      lx0 = p1->x + p1->dmx * lw;
      ly0 = p1->y + p1->dmy * lw;

      vg__vset(dst, p1->x + dlx0 * lw, p1->y + dly0 * lw, lu, 1);
      dst++;
      vg__vset(dst, p1->x, p1->y, 0.5f, 1);
      dst++;

      vg__vset(dst, lx0, ly0, lu, 1);
      dst++;
      vg__vset(dst, lx0, ly0, lu, 1);
      dst++;

      vg__vset(dst, p1->x + dlx1 * lw, p1->y + dly1 * lw, lu, 1);
      dst++;
      vg__vset(dst, p1->x, p1->y, 0.5f, 1);
      dst++;
    }

    vg__vset(dst, p1->x + dlx1 * lw, p1->y + dly1 * lw, lu, 1);
    dst++;
    vg__vset(dst, rx1, ry1, ru, 1);
    dst++;
  }

  return dst;
}

static VGvertex *vg__buttCapStart(VGvertex *dst, VGpoint *p, float dx, float dy,
                                  float w, float d, float aa, float u0,
                                  float u1) {
  float px = p->x - dx * d;
  float py = p->y - dy * d;
  float dlx = dy;
  float dly = -dx;
  vg__vset(dst, px + dlx * w - dx * aa, py + dly * w - dy * aa, u0, 0);
  dst++;
  vg__vset(dst, px - dlx * w - dx * aa, py - dly * w - dy * aa, u1, 0);
  dst++;
  vg__vset(dst, px + dlx * w, py + dly * w, u0, 1);
  dst++;
  vg__vset(dst, px - dlx * w, py - dly * w, u1, 1);
  dst++;
  return dst;
}

static VGvertex *vg__buttCapEnd(VGvertex *dst, VGpoint *p, float dx, float dy,
                                float w, float d, float aa, float u0,
                                float u1) {
  float px = p->x + dx * d;
  float py = p->y + dy * d;
  float dlx = dy;
  float dly = -dx;
  vg__vset(dst, px + dlx * w, py + dly * w, u0, 1);
  dst++;
  vg__vset(dst, px - dlx * w, py - dly * w, u1, 1);
  dst++;
  vg__vset(dst, px + dlx * w + dx * aa, py + dly * w + dy * aa, u0, 0);
  dst++;
  vg__vset(dst, px - dlx * w + dx * aa, py - dly * w + dy * aa, u1, 0);
  dst++;
  return dst;
}

static VGvertex *vg__roundCapStart(VGvertex *dst, VGpoint *p, float dx,
                                   float dy, float w, int ncap, float aa,
                                   float u0, float u1) {
  int i;
  float px = p->x;
  float py = p->y;
  float dlx = dy;
  float dly = -dx;
  VG_NOTUSED(aa);
  for (i = 0; i < ncap; i++) {
    float a = i / (float)(ncap - 1) * VG_PI;
    float ax = cosf(a) * w, ay = sinf(a) * w;
    vg__vset(dst, px - dlx * ax - dx * ay, py - dly * ax - dy * ay, u0, 1);
    dst++;
    vg__vset(dst, px, py, 0.5f, 1);
    dst++;
  }
  vg__vset(dst, px + dlx * w, py + dly * w, u0, 1);
  dst++;
  vg__vset(dst, px - dlx * w, py - dly * w, u1, 1);
  dst++;
  return dst;
}

static VGvertex *vg__roundCapEnd(VGvertex *dst, VGpoint *p, float dx, float dy,
                                 float w, int ncap, float aa, float u0,
                                 float u1) {
  int i;
  float px = p->x;
  float py = p->y;
  float dlx = dy;
  float dly = -dx;
  VG_NOTUSED(aa);
  vg__vset(dst, px + dlx * w, py + dly * w, u0, 1);
  dst++;
  vg__vset(dst, px - dlx * w, py - dly * w, u1, 1);
  dst++;
  for (i = 0; i < ncap; i++) {
    float a = i / (float)(ncap - 1) * VG_PI;
    float ax = cosf(a) * w, ay = sinf(a) * w;
    vg__vset(dst, px, py, 0.5f, 1);
    dst++;
    vg__vset(dst, px - dlx * ax + dx * ay, py - dly * ax + dy * ay, u0, 1);
    dst++;
  }
  return dst;
}

static void vg__calculateJoins(VGcontext *ctx, float w, int lineJoin,
                               float miterLimit) {
  VGpathCache *cache = ctx->cache;
  int i, j;
  float iw = 0.0f;

  if (w > 0.0f)
    iw = 1.0f / w;

  // Calculate which joins needs extra vertices to append, and gather vertex
  // count.
  for (i = 0; i < cache->npaths; i++) {
    VGpath *path = &cache->paths[i];
    VGpoint *pts = &cache->points[path->first];
    VGpoint *p0 = &pts[path->count - 1];
    VGpoint *p1 = &pts[0];
    int nleft = 0;

    path->nbevel = 0;

    for (j = 0; j < path->count; j++) {
      float dlx0, dly0, dlx1, dly1, dmr2, cross, limit;
      dlx0 = p0->dy;
      dly0 = -p0->dx;
      dlx1 = p1->dy;
      dly1 = -p1->dx;
      // Calculate extrusions
      p1->dmx = (dlx0 + dlx1) * 0.5f;
      p1->dmy = (dly0 + dly1) * 0.5f;
      dmr2 = p1->dmx * p1->dmx + p1->dmy * p1->dmy;
      if (dmr2 > 0.000001f) {
        float scale = 1.0f / dmr2;
        if (scale > 600.0f) {
          scale = 600.0f;
        }
        p1->dmx *= scale;
        p1->dmy *= scale;
      }

      // Clear flags, but keep the corner.
      p1->flags = (p1->flags & VG_PT_CORNER) ? VG_PT_CORNER : 0;

      // Keep track of left turns.
      cross = p1->dx * p0->dy - p0->dx * p1->dy;
      if (cross > 0.0f) {
        nleft++;
        p1->flags |= VG_PT_LEFT;
      }

      // Calculate if we should use bevel or miter for inner join.
      limit = vg__maxf(1.01f, vg__minf(p0->len, p1->len) * iw);
      if ((dmr2 * limit * limit) < 1.0f)
        p1->flags |= VG_PR_INNERBEVEL;

      // Check to see if the corner needs to be beveled.
      if (p1->flags & VG_PT_CORNER) {
        if ((dmr2 * miterLimit * miterLimit) < 1.0f || lineJoin == VG_BEVEL ||
            lineJoin == VG_ROUND) {
          p1->flags |= VG_PT_BEVEL;
        }
      }

      if ((p1->flags & (VG_PT_BEVEL | VG_PR_INNERBEVEL)) != 0)
        path->nbevel++;

      p0 = p1++;
    }

    path->convex = (nleft == path->count) ? 1 : 0;
  }
}

static int vg__expandStroke(VGcontext *ctx, float w, float fringe, int lineCap,
                            int lineJoin, float miterLimit) {
  VGpathCache *cache = ctx->cache;
  VGvertex *verts;
  VGvertex *dst;
  int cverts, i, j;
  float aa = fringe; // ctx->fringeWidth;
  float u0 = 0.0f, u1 = 1.0f;
  int ncap = vg__curveDivs(
      w, VG_PI, ctx->tessTol); // Calculate divisions per half circle.

  w += aa * 0.5f;

  // Disable the gradient used for antialiasing when antialiasing is not used.
  if (aa == 0.0f) {
    u0 = 0.5f;
    u1 = 0.5f;
  }

  vg__calculateJoins(ctx, w, lineJoin, miterLimit);

  // Calculate max vertex usage.
  cverts = 0;
  for (i = 0; i < cache->npaths; i++) {
    VGpath *path = &cache->paths[i];
    int loop = (path->closed == 0) ? 0 : 1;
    if (lineJoin == VG_ROUND)
      cverts += (path->count + path->nbevel * (ncap + 2) + 1) *
                2; // plus one for loop
    else
      cverts += (path->count + path->nbevel * 5 + 1) * 2; // plus one for loop
    if (loop == 0) {
      // space for caps
      if (lineCap == VG_ROUND) {
        cverts += (ncap * 2 + 2) * 2;
      } else {
        cverts += (3 + 3) * 2;
      }
    }
  }

  verts = vg__allocTempVerts(ctx, cverts);
  if (verts == NULL)
    return 0;

  for (i = 0; i < cache->npaths; i++) {
    VGpath *path = &cache->paths[i];
    VGpoint *pts = &cache->points[path->first];
    VGpoint *p0;
    VGpoint *p1;
    int s, e, loop;
    float dx, dy;

    path->fill = 0;
    path->nfill = 0;

    // Calculate fringe or stroke
    loop = (path->closed == 0) ? 0 : 1;
    dst = verts;
    path->stroke = dst;

    if (loop) {
      // Looping
      p0 = &pts[path->count - 1];
      p1 = &pts[0];
      s = 0;
      e = path->count;
    } else {
      // Add cap
      p0 = &pts[0];
      p1 = &pts[1];
      s = 1;
      e = path->count - 1;
    }

    if (loop == 0) {
      // Add cap
      dx = p1->x - p0->x;
      dy = p1->y - p0->y;
      vg__normalize(&dx, &dy);
      if (lineCap == VG_BUTT)
        dst = vg__buttCapStart(dst, p0, dx, dy, w, -aa * 0.5f, aa, u0, u1);
      else if (lineCap == VG_BUTT || lineCap == VG_SQUARE)
        dst = vg__buttCapStart(dst, p0, dx, dy, w, w - aa, aa, u0, u1);
      else if (lineCap == VG_ROUND)
        dst = vg__roundCapStart(dst, p0, dx, dy, w, ncap, aa, u0, u1);
    }

    for (j = s; j < e; ++j) {
      if ((p1->flags & (VG_PT_BEVEL | VG_PR_INNERBEVEL)) != 0) {
        if (lineJoin == VG_ROUND) {
          dst = vg__roundJoin(dst, p0, p1, w, w, u0, u1, ncap, aa);
        } else {
          dst = vg__bevelJoin(dst, p0, p1, w, w, u0, u1, aa);
        }
      } else {
        vg__vset(dst, p1->x + (p1->dmx * w), p1->y + (p1->dmy * w), u0, 1);
        dst++;
        vg__vset(dst, p1->x - (p1->dmx * w), p1->y - (p1->dmy * w), u1, 1);
        dst++;
      }
      p0 = p1++;
    }

    if (loop) {
      // Loop it
      vg__vset(dst, verts[0].x, verts[0].y, u0, 1);
      dst++;
      vg__vset(dst, verts[1].x, verts[1].y, u1, 1);
      dst++;
    } else {
      // Add cap
      dx = p1->x - p0->x;
      dy = p1->y - p0->y;
      vg__normalize(&dx, &dy);
      if (lineCap == VG_BUTT)
        dst = vg__buttCapEnd(dst, p1, dx, dy, w, -aa * 0.5f, aa, u0, u1);
      else if (lineCap == VG_BUTT || lineCap == VG_SQUARE)
        dst = vg__buttCapEnd(dst, p1, dx, dy, w, w - aa, aa, u0, u1);
      else if (lineCap == VG_ROUND)
        dst = vg__roundCapEnd(dst, p1, dx, dy, w, ncap, aa, u0, u1);
    }

    path->nstroke = (int)(dst - verts);

    verts = dst;
  }

  return 1;
}

static int vg__expandFill(VGcontext *ctx, float w, int lineJoin,
                          float miterLimit) {
  VGpathCache *cache = ctx->cache;
  VGvertex *verts;
  VGvertex *dst;
  int cverts, convex, i, j;
  float aa = ctx->fringeWidth;
  int fringe = w > 0.0f;

  vg__calculateJoins(ctx, w, lineJoin, miterLimit);

  // Calculate max vertex usage.
  cverts = 0;
  for (i = 0; i < cache->npaths; i++) {
    VGpath *path = &cache->paths[i];
    cverts += path->count + path->nbevel + 1;
    if (fringe)
      cverts += (path->count + path->nbevel * 5 + 1) * 2; // plus one for loop
  }

  verts = vg__allocTempVerts(ctx, cverts);
  if (verts == NULL)
    return 0;

  convex = cache->npaths == 1 && cache->paths[0].convex;

  for (i = 0; i < cache->npaths; i++) {
    VGpath *path = &cache->paths[i];
    VGpoint *pts = &cache->points[path->first];
    VGpoint *p0;
    VGpoint *p1;
    float rw, lw, woff;
    float ru, lu;

    // Calculate shape vertices.
    woff = 0.5f * aa;
    dst = verts;
    path->fill = dst;

    if (fringe) {
      // Looping
      p0 = &pts[path->count - 1];
      p1 = &pts[0];
      for (j = 0; j < path->count; ++j) {
        if (p1->flags & VG_PT_BEVEL) {
          float dlx0 = p0->dy;
          float dly0 = -p0->dx;
          float dlx1 = p1->dy;
          float dly1 = -p1->dx;
          if (p1->flags & VG_PT_LEFT) {
            float lx = p1->x + p1->dmx * woff;
            float ly = p1->y + p1->dmy * woff;
            vg__vset(dst, lx, ly, 0.5f, 1);
            dst++;
          } else {
            float lx0 = p1->x + dlx0 * woff;
            float ly0 = p1->y + dly0 * woff;
            float lx1 = p1->x + dlx1 * woff;
            float ly1 = p1->y + dly1 * woff;
            vg__vset(dst, lx0, ly0, 0.5f, 1);
            dst++;
            vg__vset(dst, lx1, ly1, 0.5f, 1);
            dst++;
          }
        } else {
          vg__vset(dst, p1->x + (p1->dmx * woff), p1->y + (p1->dmy * woff),
                   0.5f, 1);
          dst++;
        }
        p0 = p1++;
      }
    } else {
      for (j = 0; j < path->count; ++j) {
        vg__vset(dst, pts[j].x, pts[j].y, 0.5f, 1);
        dst++;
      }
    }

    path->nfill = (int)(dst - verts);
    verts = dst;

    // Calculate fringe
    if (fringe) {
      lw = w + woff;
      rw = w - woff;
      lu = 0;
      ru = 1;
      dst = verts;
      path->stroke = dst;

      // Create only half a fringe for convex shapes so that
      // the shape can be rendered without stenciling.
      if (convex) {
        lw = woff; // This should generate the same vertex as fill inset above.
        lu = 0.5f; // Set outline fade at middle.
      }

      // Looping
      p0 = &pts[path->count - 1];
      p1 = &pts[0];

      for (j = 0; j < path->count; ++j) {
        if ((p1->flags & (VG_PT_BEVEL | VG_PR_INNERBEVEL)) != 0) {
          dst = vg__bevelJoin(dst, p0, p1, lw, rw, lu, ru, ctx->fringeWidth);
        } else {
          vg__vset(dst, p1->x + (p1->dmx * lw), p1->y + (p1->dmy * lw), lu, 1);
          dst++;
          vg__vset(dst, p1->x - (p1->dmx * rw), p1->y - (p1->dmy * rw), ru, 1);
          dst++;
        }
        p0 = p1++;
      }

      // Loop it
      vg__vset(dst, verts[0].x, verts[0].y, lu, 1);
      dst++;
      vg__vset(dst, verts[1].x, verts[1].y, ru, 1);
      dst++;

      path->nstroke = (int)(dst - verts);
      verts = dst;
    } else {
      path->stroke = NULL;
      path->nstroke = 0;
    }
  }

  return 1;
}

// Draw
void vgBeginPath(VGcontext *ctx) {
  ctx->ncommands = 0;
  vg__clearPathCache(ctx);
}

void vgMoveTo(VGcontext *ctx, float x, float y) {
  float vals[] = {VG_MOVETO, x, y};
  vg__appendCommands(ctx, vals, VG_COUNTOF(vals));
}

void vgLineTo(VGcontext *ctx, float x, float y) {
  float vals[] = {VG_LINETO, x, y};
  vg__appendCommands(ctx, vals, VG_COUNTOF(vals));
}

void vgBezierTo(VGcontext *ctx, float c1x, float c1y, float c2x, float c2y,
                float x, float y) {
  float vals[] = {VG_BEZIERTO, c1x, c1y, c2x, c2y, x, y};
  vg__appendCommands(ctx, vals, VG_COUNTOF(vals));
}

void vgQuadTo(VGcontext *ctx, float cx, float cy, float x, float y) {
  float x0 = ctx->commandx;
  float y0 = ctx->commandy;
  float vals[] = {VG_BEZIERTO,
                  x0 + 2.0f / 3.0f * (cx - x0),
                  y0 + 2.0f / 3.0f * (cy - y0),
                  x + 2.0f / 3.0f * (cx - x),
                  y + 2.0f / 3.0f * (cy - y),
                  x,
                  y};
  vg__appendCommands(ctx, vals, VG_COUNTOF(vals));
}

void vgArcTo(VGcontext *ctx, float x1, float y1, float x2, float y2,
             float radius) {
  float x0 = ctx->commandx;
  float y0 = ctx->commandy;
  float dx0, dy0, dx1, dy1, a, d, cx, cy, a0, a1;
  int dir;

  if (ctx->ncommands == 0) {
    return;
  }

  // Handle degenerate cases.
  if (vg__ptEquals(x0, y0, x1, y1, ctx->distTol) ||
      vg__ptEquals(x1, y1, x2, y2, ctx->distTol) ||
      vg__distPtSeg(x1, y1, x0, y0, x2, y2) < ctx->distTol * ctx->distTol ||
      radius < ctx->distTol) {
    vgLineTo(ctx, x1, y1);
    return;
  }

  // Calculate tangential circle to lines (x0,y0)-(x1,y1) and (x1,y1)-(x2,y2).
  dx0 = x0 - x1;
  dy0 = y0 - y1;
  dx1 = x2 - x1;
  dy1 = y2 - y1;
  vg__normalize(&dx0, &dy0);
  vg__normalize(&dx1, &dy1);
  a = vg__acosf(dx0 * dx1 + dy0 * dy1);
  d = radius / vg__tanf(a / 2.0f);

  //	printf("a=%f° d=%f\n", a/VG_PI*180.0f, d);

  if (d > 10000.0f) {
    vgLineTo(ctx, x1, y1);
    return;
  }

  if (vg__cross(dx0, dy0, dx1, dy1) > 0.0f) {
    cx = x1 + dx0 * d + dy0 * radius;
    cy = y1 + dy0 * d + -dx0 * radius;
    a0 = vg__atan2f(dx0, -dy0);
    a1 = vg__atan2f(-dx1, dy1);
    dir = VG_CW;
    //		printf("CW c=(%f, %f) a0=%f° a1=%f°\n", cx, cy,
    // a0/VG_PI*180.0f, a1/VG_PI*180.0f);
  } else {
    cx = x1 + dx0 * d + -dy0 * radius;
    cy = y1 + dy0 * d + dx0 * radius;
    a0 = vg__atan2f(-dx0, dy0);
    a1 = vg__atan2f(dx1, -dy1);
    dir = VG_CCW;
    //		printf("CCW c=(%f, %f) a0=%f° a1=%f°\n", cx, cy,
    // a0/VG_PI*180.0f, a1/VG_PI*180.0f);
  }

  vgArc(ctx, cx, cy, radius, a0, a1, dir);
}

void vgClosePath(VGcontext *ctx) {
  float vals[] = {VG_CLOSE};
  vg__appendCommands(ctx, vals, VG_COUNTOF(vals));
}

void vgPathWinding(VGcontext *ctx, int dir) {
  float vals[] = {VG_WINDING, (float)dir};
  vg__appendCommands(ctx, vals, VG_COUNTOF(vals));
}

void vgArc(VGcontext *ctx, float cx, float cy, float r, float a0, float a1,
           int dir) {
  float a = 0, da = 0, hda = 0, kappa = 0;
  float dx = 0, dy = 0, x = 0, y = 0, tanx = 0, tany = 0;
  float px = 0, py = 0, ptanx = 0, ptany = 0;
  float vals[3 + 5 * 7 + 100];
  int i, ndivs, nvals;
  int move = ctx->ncommands > 0 ? VG_LINETO : VG_MOVETO;

  // Clamp angles
  da = a1 - a0;
  if (dir == VG_CW) {
    if (vg__absf(da) >= VG_PI * 2) {
      da = VG_PI * 2;
    } else {
      while (da < 0.0f)
        da += VG_PI * 2;
    }
  } else {
    if (vg__absf(da) >= VG_PI * 2) {
      da = -VG_PI * 2;
    } else {
      while (da > 0.0f)
        da -= VG_PI * 2;
    }
  }

  // Split arc into max 90 degree segments.
  ndivs = vg__maxi(1, vg__mini((int)(vg__absf(da) / (VG_PI * 0.5f) + 0.5f), 5));
  hda = (da / (float)ndivs) / 2.0f;
  kappa = vg__absf(4.0f / 3.0f * (1.0f - vg__cosf(hda)) / vg__sinf(hda));

  if (dir == VG_CCW)
    kappa = -kappa;

  nvals = 0;
  for (i = 0; i <= ndivs; i++) {
    a = a0 + da * (i / (float)ndivs);
    dx = vg__cosf(a);
    dy = vg__sinf(a);
    x = cx + dx * r;
    y = cy + dy * r;
    tanx = -dy * r * kappa;
    tany = dx * r * kappa;

    if (i == 0) {
      vals[nvals++] = (float)move;
      vals[nvals++] = x;
      vals[nvals++] = y;
    } else {
      vals[nvals++] = VG_BEZIERTO;
      vals[nvals++] = px + ptanx;
      vals[nvals++] = py + ptany;
      vals[nvals++] = x - tanx;
      vals[nvals++] = y - tany;
      vals[nvals++] = x;
      vals[nvals++] = y;
    }
    px = x;
    py = y;
    ptanx = tanx;
    ptany = tany;
  }

  vg__appendCommands(ctx, vals, nvals);
}

void vgRect(VGcontext *ctx, float x, float y, float w, float h) {
  float vals[] = {VG_MOVETO, x,     y,         VG_LINETO, x, y + h,   VG_LINETO,
                  x + w,     y + h, VG_LINETO, x + w,     y, VG_CLOSE};
  vg__appendCommands(ctx, vals, VG_COUNTOF(vals));
}

void vgRoundedRect(VGcontext *ctx, float x, float y, float w, float h,
                   float r) {
  vgRoundedRectVarying(ctx, x, y, w, h, r, r, r, r);
}

void vgRoundedRectVarying(VGcontext *ctx, float x, float y, float w, float h,
                          float radTopLeft, float radTopRight,
                          float radBottomRight, float radBottomLeft) {
  if (radTopLeft < 0.1f && radTopRight < 0.1f && radBottomRight < 0.1f &&
      radBottomLeft < 0.1f) {
    vgRect(ctx, x, y, w, h);
    return;
  } else {
    float halfw = vg__absf(w) * 0.5f;
    float halfh = vg__absf(h) * 0.5f;
    float rxBL = vg__minf(radBottomLeft, halfw) * vg__signf(w),
          ryBL = vg__minf(radBottomLeft, halfh) * vg__signf(h);
    float rxBR = vg__minf(radBottomRight, halfw) * vg__signf(w),
          ryBR = vg__minf(radBottomRight, halfh) * vg__signf(h);
    float rxTR = vg__minf(radTopRight, halfw) * vg__signf(w),
          ryTR = vg__minf(radTopRight, halfh) * vg__signf(h);
    float rxTL = vg__minf(radTopLeft, halfw) * vg__signf(w),
          ryTL = vg__minf(radTopLeft, halfh) * vg__signf(h);
    float vals[] = {VG_MOVETO,
                    x,
                    y + ryTL,
                    VG_LINETO,
                    x,
                    y + h - ryBL,
                    VG_BEZIERTO,
                    x,
                    y + h - ryBL * (1 - VG_KAPPA90),
                    x + rxBL * (1 - VG_KAPPA90),
                    y + h,
                    x + rxBL,
                    y + h,
                    VG_LINETO,
                    x + w - rxBR,
                    y + h,
                    VG_BEZIERTO,
                    x + w - rxBR * (1 - VG_KAPPA90),
                    y + h,
                    x + w,
                    y + h - ryBR * (1 - VG_KAPPA90),
                    x + w,
                    y + h - ryBR,
                    VG_LINETO,
                    x + w,
                    y + ryTR,
                    VG_BEZIERTO,
                    x + w,
                    y + ryTR * (1 - VG_KAPPA90),
                    x + w - rxTR * (1 - VG_KAPPA90),
                    y,
                    x + w - rxTR,
                    y,
                    VG_LINETO,
                    x + rxTL,
                    y,
                    VG_BEZIERTO,
                    x + rxTL * (1 - VG_KAPPA90),
                    y,
                    x,
                    y + ryTL * (1 - VG_KAPPA90),
                    x,
                    y + ryTL,
                    VG_CLOSE};
    vg__appendCommands(ctx, vals, VG_COUNTOF(vals));
  }
}

void vgEllipse(VGcontext *ctx, float cx, float cy, float rx, float ry) {
  float vals[] = {VG_MOVETO,
                  cx - rx,
                  cy,
                  VG_BEZIERTO,
                  cx - rx,
                  cy + ry * VG_KAPPA90,
                  cx - rx * VG_KAPPA90,
                  cy + ry,
                  cx,
                  cy + ry,
                  VG_BEZIERTO,
                  cx + rx * VG_KAPPA90,
                  cy + ry,
                  cx + rx,
                  cy + ry * VG_KAPPA90,
                  cx + rx,
                  cy,
                  VG_BEZIERTO,
                  cx + rx,
                  cy - ry * VG_KAPPA90,
                  cx + rx * VG_KAPPA90,
                  cy - ry,
                  cx,
                  cy - ry,
                  VG_BEZIERTO,
                  cx - rx * VG_KAPPA90,
                  cy - ry,
                  cx - rx,
                  cy - ry * VG_KAPPA90,
                  cx - rx,
                  cy,
                  VG_CLOSE};
  vg__appendCommands(ctx, vals, VG_COUNTOF(vals));
}

void vgCircle(VGcontext *ctx, float cx, float cy, float r) {
  vgEllipse(ctx, cx, cy, r, r);
}

void vgDebugDumpPathCache(VGcontext *ctx) {
  const VGpath *path;
  int i, j;

  printf("Dumping %d cached paths\n", ctx->cache->npaths);
  for (i = 0; i < ctx->cache->npaths; i++) {
    path = &ctx->cache->paths[i];
    printf(" - Path %d\n", i);
    if (path->nfill) {
      printf("   - fill: %d\n", path->nfill);
      for (j = 0; j < path->nfill; j++)
        printf("%f\t%f\n", path->fill[j].x, path->fill[j].y);
    }
    if (path->nstroke) {
      printf("   - stroke: %d\n", path->nstroke);
      for (j = 0; j < path->nstroke; j++)
        printf("%f\t%f\n", path->stroke[j].x, path->stroke[j].y);
    }
  }
}

void vgFill(VGcontext *ctx) {
  VGstate *state = vg__getState(ctx);
  const VGpath *path;
  VGpaint fillPaint = state->fill;
  int i;

  vg__flattenPaths(ctx);
  if (ctx->params.edgeAntiAlias && state->shapeAntiAlias)
    vg__expandFill(ctx, ctx->fringeWidth, VG_MITER, 2.4f);
  else
    vg__expandFill(ctx, 0.0f, VG_MITER, 2.4f);

  // Apply global alpha
  fillPaint.innerColor.a *= state->alpha;
  fillPaint.outerColor.a *= state->alpha;

  ctx->params.renderFill(ctx->params.userPtr, &fillPaint,
                         state->compositeOperation, &state->scissor,
                         ctx->fringeWidth, ctx->cache->bounds,
                         ctx->cache->paths, ctx->cache->npaths);

  // Count triangles
  for (i = 0; i < ctx->cache->npaths; i++) {
    path = &ctx->cache->paths[i];
    ctx->fillTriCount += path->nfill - 2;
    ctx->fillTriCount += path->nstroke - 2;
    ctx->drawCallCount += 2;
  }
}

void vgStroke(VGcontext *ctx) {
  VGstate *state = vg__getState(ctx);
  float scale = vg__getAverageScale(state->xform);
  float strokeWidth = vg__clampf(state->strokeWidth * scale, 0.0f, 200.0f);
  VGpaint strokePaint = state->stroke;
  const VGpath *path;
  int i;

  if (strokeWidth < ctx->fringeWidth) {
    // If the stroke width is less than pixel size, use alpha to emulate
    // coverage. Since coverage is area, scale by alpha*alpha.
    float alpha = vg__clampf(strokeWidth / ctx->fringeWidth, 0.0f, 1.0f);
    strokePaint.innerColor.a *= alpha * alpha;
    strokePaint.outerColor.a *= alpha * alpha;
    strokeWidth = ctx->fringeWidth;
  }

  // Apply global alpha
  strokePaint.innerColor.a *= state->alpha;
  strokePaint.outerColor.a *= state->alpha;

  vg__flattenPaths(ctx);

  if (ctx->params.edgeAntiAlias && state->shapeAntiAlias)
    vg__expandStroke(ctx, strokeWidth * 0.5f, ctx->fringeWidth, state->lineCap,
                     state->lineJoin, state->miterLimit);
  else
    vg__expandStroke(ctx, strokeWidth * 0.5f, 0.0f, state->lineCap,
                     state->lineJoin, state->miterLimit);

  ctx->params.renderStroke(ctx->params.userPtr, &strokePaint,
                           state->compositeOperation, &state->scissor,
                           ctx->fringeWidth, strokeWidth, ctx->cache->paths,
                           ctx->cache->npaths);

  // Count triangles
  for (i = 0; i < ctx->cache->npaths; i++) {
    path = &ctx->cache->paths[i];
    ctx->strokeTriCount += path->nstroke - 2;
    ctx->drawCallCount++;
  }
}

// Add fonts
int vgCreateFont(VGcontext *ctx, const char *name, const char *filename) {
  return fonsAddFont(ctx->fs, name, filename, 0);
}

int vgCreateFontAtIndex(VGcontext *ctx, const char *name, const char *filename,
                        const int fontIndex) {
  return fonsAddFont(ctx->fs, name, filename, fontIndex);
}

int vgCreateFontMem(VGcontext *ctx, const char *name, unsigned char *data,
                    int ndata, int freeData) {
  return fonsAddFontMem(ctx->fs, name, data, ndata, freeData, 0);
}

int vgCreateFontMemAtIndex(VGcontext *ctx, const char *name,
                           unsigned char *data, int ndata, int freeData,
                           const int fontIndex) {
  return fonsAddFontMem(ctx->fs, name, data, ndata, freeData, fontIndex);
}

int vgFindFont(VGcontext *ctx, const char *name) {
  if (name == NULL)
    return -1;
  return fonsGetFontByName(ctx->fs, name);
}

int vgAddFallbackFontId(VGcontext *ctx, int baseFont, int fallbackFont) {
  if (baseFont == -1 || fallbackFont == -1)
    return 0;
  return fonsAddFallbackFont(ctx->fs, baseFont, fallbackFont);
}

int vgAddFallbackFont(VGcontext *ctx, const char *baseFont,
                      const char *fallbackFont) {
  return vgAddFallbackFontId(ctx, vgFindFont(ctx, baseFont),
                             vgFindFont(ctx, fallbackFont));
}

void vgResetFallbackFontsId(VGcontext *ctx, int baseFont) {
  fonsResetFallbackFont(ctx->fs, baseFont);
}

void vgResetFallbackFonts(VGcontext *ctx, const char *baseFont) {
  vgResetFallbackFontsId(ctx, vgFindFont(ctx, baseFont));
}

// State setting
void vgFontSize(VGcontext *ctx, float size) {
  VGstate *state = vg__getState(ctx);
  state->fontSize = size;
}

void vgFontBlur(VGcontext *ctx, float blur) {
  VGstate *state = vg__getState(ctx);
  state->fontBlur = blur;
}

void vgTextLetterSpacing(VGcontext *ctx, float spacing) {
  VGstate *state = vg__getState(ctx);
  state->letterSpacing = spacing;
}

void vgTextLineHeight(VGcontext *ctx, float lineHeight) {
  VGstate *state = vg__getState(ctx);
  state->lineHeight = lineHeight;
}

void vgTextAlign(VGcontext *ctx, int align) {
  VGstate *state = vg__getState(ctx);
  state->textAlign = align;
}

void vgFontFaceId(VGcontext *ctx, int font) {
  VGstate *state = vg__getState(ctx);
  state->fontId = font;
}

void vgFontFace(VGcontext *ctx, const char *font) {
  VGstate *state = vg__getState(ctx);
  state->fontId = fonsGetFontByName(ctx->fs, font);
}

static float vg__quantize(float a, float d) {
  return ((int)(a / d + 0.5f)) * d;
}

static float vg__getFontScale(VGstate *state) {
  return vg__minf(vg__quantize(vg__getAverageScale(state->xform), 0.01f), 4.0f);
}

static void vg__flushTextTexture(VGcontext *ctx) {
  int dirty[4];

  if (fonsValidateTexture(ctx->fs, dirty)) {
    int fontImage = ctx->fontImages[ctx->fontImageIdx];
    // Update texture
    if (fontImage != 0) {
      int iw, ih;
      const unsigned char *data = fonsGetTextureData(ctx->fs, &iw, &ih);
      int x = dirty[0];
      int y = dirty[1];
      int w = dirty[2] - dirty[0];
      int h = dirty[3] - dirty[1];
      ctx->params.renderUpdateTexture(ctx->params.userPtr, fontImage, x, y, w,
                                      h, data);
    }
  }
}

static int vg__allocTextAtlas(VGcontext *ctx) {
  int iw, ih;
  vg__flushTextTexture(ctx);
  if (ctx->fontImageIdx >= VG_MAX_FONTIMAGES - 1)
    return 0;
  // if next fontImage already have a texture
  if (ctx->fontImages[ctx->fontImageIdx + 1] != 0)
    vgImageSize(ctx, ctx->fontImages[ctx->fontImageIdx + 1], &iw, &ih);
  else { // calculate the new font image size and create it.
    vgImageSize(ctx, ctx->fontImages[ctx->fontImageIdx], &iw, &ih);
    if (iw > ih)
      ih *= 2;
    else
      iw *= 2;
    if (iw > VG_MAX_FONTIMAGE_SIZE || ih > VG_MAX_FONTIMAGE_SIZE)
      iw = ih = VG_MAX_FONTIMAGE_SIZE;
    ctx->fontImages[ctx->fontImageIdx + 1] = ctx->params.renderCreateTexture(
        ctx->params.userPtr, VG_TEXTURE_ALPHA, iw, ih, 0, NULL);
  }
  ++ctx->fontImageIdx;
  fonsResetAtlas(ctx->fs, iw, ih);
  return 1;
}

static void vg__renderText(VGcontext *ctx, VGvertex *verts, int nverts) {
  VGstate *state = vg__getState(ctx);
  VGpaint paint = state->fill;

  // Render triangles.
  paint.image = ctx->fontImages[ctx->fontImageIdx];

  // Apply global alpha
  paint.innerColor.a *= state->alpha;
  paint.outerColor.a *= state->alpha;

  ctx->params.renderTriangles(ctx->params.userPtr, &paint,
                              state->compositeOperation, &state->scissor, verts,
                              nverts, ctx->fringeWidth);

  ctx->drawCallCount++;
  ctx->textTriCount += nverts / 3;
}

static int vg__isTransformFlipped(const float *xform) {
  float det = xform[0] * xform[3] - xform[2] * xform[1];
  return (det < 0);
}

float vgText(VGcontext *ctx, float x, float y, const char *string,
             const char *end) {
  VGstate *state = vg__getState(ctx);
  FONStextIter iter, prevIter;
  FONSquad q;
  VGvertex *verts;
  float scale = vg__getFontScale(state) * ctx->devicePxRatio;
  float invscale = 1.0f / scale;
  int cverts = 0;
  int nverts = 0;
  int isFlipped = vg__isTransformFlipped(state->xform);

  if (end == NULL)
    end = string + strlen(string);

  if (state->fontId == FONS_INVALID)
    return x;

  fonsSetSize(ctx->fs, state->fontSize * scale);
  fonsSetSpacing(ctx->fs, state->letterSpacing * scale);
  fonsSetBlur(ctx->fs, state->fontBlur * scale);
  fonsSetAlign(ctx->fs, state->textAlign);
  fonsSetFont(ctx->fs, state->fontId);

  cverts = vg__maxi(2, (int)(end - string)) * 6; // conservative estimate.
  verts = vg__allocTempVerts(ctx, cverts);
  if (verts == NULL)
    return x;

  fonsTextIterInit(ctx->fs, &iter, x * scale, y * scale, string, end,
                   FONS_GLYPH_BITMAP_REQUIRED);
  prevIter = iter;
  while (fonsTextIterNext(ctx->fs, &iter, &q)) {
    float c[4 * 2];
    if (iter.prevGlyphIndex == -1) { // can not retrieve glyph?
      if (nverts != 0) {
        vg__renderText(ctx, verts, nverts);
        nverts = 0;
      }
      if (!vg__allocTextAtlas(ctx))
        break; // no memory :(
      iter = prevIter;
      fonsTextIterNext(ctx->fs, &iter, &q); // try again
      if (iter.prevGlyphIndex == -1)        // still can not find glyph?
        break;
    }
    prevIter = iter;
    if (isFlipped) {
      float tmp;

      tmp = q.y0;
      q.y0 = q.y1;
      q.y1 = tmp;
      tmp = q.t0;
      q.t0 = q.t1;
      q.t1 = tmp;
    }
    // Transform corners.
    vgTransformPoint(&c[0], &c[1], state->xform, q.x0 * invscale,
                     q.y0 * invscale);
    vgTransformPoint(&c[2], &c[3], state->xform, q.x1 * invscale,
                     q.y0 * invscale);
    vgTransformPoint(&c[4], &c[5], state->xform, q.x1 * invscale,
                     q.y1 * invscale);
    vgTransformPoint(&c[6], &c[7], state->xform, q.x0 * invscale,
                     q.y1 * invscale);
    // Create triangles
    if (nverts + 6 <= cverts) {
      vg__vset(&verts[nverts], c[0], c[1], q.s0, q.t0);
      nverts++;
      vg__vset(&verts[nverts], c[4], c[5], q.s1, q.t1);
      nverts++;
      vg__vset(&verts[nverts], c[2], c[3], q.s1, q.t0);
      nverts++;
      vg__vset(&verts[nverts], c[0], c[1], q.s0, q.t0);
      nverts++;
      vg__vset(&verts[nverts], c[6], c[7], q.s0, q.t1);
      nverts++;
      vg__vset(&verts[nverts], c[4], c[5], q.s1, q.t1);
      nverts++;
    }
  }

  // TODO: add back-end bit to do this just once per frame.
  vg__flushTextTexture(ctx);

  vg__renderText(ctx, verts, nverts);

  return iter.nextx / scale;
}

void vgTextBox(VGcontext *ctx, float x, float y, float breakRowWidth,
               const char *string, const char *end) {
  VGstate *state = vg__getState(ctx);
  VGtextRow rows[2];
  int nrows = 0, i;
  int oldAlign = state->textAlign;
  int halign =
      state->textAlign & (VG_ALIGN_LEFT | VG_ALIGN_CENTER | VG_ALIGN_RIGHT);
  int valign = state->textAlign & (VG_ALIGN_TOP | VG_ALIGN_MIDDLE |
                                   VG_ALIGN_BOTTOM | VG_ALIGN_BASELINE);
  float lineh = 0;

  if (state->fontId == FONS_INVALID)
    return;

  vgTextMetrics(ctx, NULL, NULL, &lineh);

  state->textAlign = VG_ALIGN_LEFT | valign;

  while ((nrows = vgTextBreakLines(ctx, string, end, breakRowWidth, rows, 2))) {
    for (i = 0; i < nrows; i++) {
      VGtextRow *row = &rows[i];
      if (halign & VG_ALIGN_LEFT)
        vgText(ctx, x, y, row->start, row->end);
      else if (halign & VG_ALIGN_CENTER)
        vgText(ctx, x + breakRowWidth * 0.5f - row->width * 0.5f, y, row->start,
               row->end);
      else if (halign & VG_ALIGN_RIGHT)
        vgText(ctx, x + breakRowWidth - row->width, y, row->start, row->end);
      y += lineh * state->lineHeight;
    }
    string = rows[nrows - 1].next;
  }

  state->textAlign = oldAlign;
}

int vgTextGlyphPositions(VGcontext *ctx, float x, float y, const char *string,
                         const char *end, VGglyphPosition *positions,
                         int maxPositions) {
  VGstate *state = vg__getState(ctx);
  float scale = vg__getFontScale(state) * ctx->devicePxRatio;
  float invscale = 1.0f / scale;
  FONStextIter iter, prevIter;
  FONSquad q;
  int npos = 0;

  if (state->fontId == FONS_INVALID)
    return 0;

  if (end == NULL)
    end = string + strlen(string);

  if (string == end)
    return 0;

  fonsSetSize(ctx->fs, state->fontSize * scale);
  fonsSetSpacing(ctx->fs, state->letterSpacing * scale);
  fonsSetBlur(ctx->fs, state->fontBlur * scale);
  fonsSetAlign(ctx->fs, state->textAlign);
  fonsSetFont(ctx->fs, state->fontId);

  fonsTextIterInit(ctx->fs, &iter, x * scale, y * scale, string, end,
                   FONS_GLYPH_BITMAP_OPTIONAL);
  prevIter = iter;
  while (fonsTextIterNext(ctx->fs, &iter, &q)) {
    if (iter.prevGlyphIndex < 0 &&
        vg__allocTextAtlas(ctx)) { // can not retrieve glyph?
      iter = prevIter;
      fonsTextIterNext(ctx->fs, &iter, &q); // try again
    }
    prevIter = iter;
    positions[npos].str = iter.str;
    positions[npos].x = iter.x * invscale;
    positions[npos].minx = vg__minf(iter.x, q.x0) * invscale;
    positions[npos].maxx = vg__maxf(iter.nextx, q.x1) * invscale;
    npos++;
    if (npos >= maxPositions)
      break;
  }

  return npos;
}

enum VGcodepointType {
  VG_SPACE,
  VG_NEWLINE,
  VG_CHAR,
  VG_CJK_CHAR,
};

int vgTextBreakLines(VGcontext *ctx, const char *string, const char *end,
                     float breakRowWidth, VGtextRow *rows, int maxRows) {
  VGstate *state = vg__getState(ctx);
  float scale = vg__getFontScale(state) * ctx->devicePxRatio;
  float invscale = 1.0f / scale;
  FONStextIter iter, prevIter;
  FONSquad q;
  int nrows = 0;
  float rowStartX = 0;
  float rowWidth = 0;
  float rowMinX = 0;
  float rowMaxX = 0;
  const char *rowStart = NULL;
  const char *rowEnd = NULL;
  const char *wordStart = NULL;
  float wordStartX = 0;
  float wordMinX = 0;
  const char *breakEnd = NULL;
  float breakWidth = 0;
  float breakMaxX = 0;
  int type = VG_SPACE, ptype = VG_SPACE;
  unsigned int pcodepoint = 0;

  if (maxRows == 0)
    return 0;
  if (state->fontId == FONS_INVALID)
    return 0;

  if (end == NULL)
    end = string + strlen(string);

  if (string == end)
    return 0;

  fonsSetSize(ctx->fs, state->fontSize * scale);
  fonsSetSpacing(ctx->fs, state->letterSpacing * scale);
  fonsSetBlur(ctx->fs, state->fontBlur * scale);
  fonsSetAlign(ctx->fs, state->textAlign);
  fonsSetFont(ctx->fs, state->fontId);

  breakRowWidth *= scale;

  fonsTextIterInit(ctx->fs, &iter, 0, 0, string, end,
                   FONS_GLYPH_BITMAP_OPTIONAL);
  prevIter = iter;
  while (fonsTextIterNext(ctx->fs, &iter, &q)) {
    if (iter.prevGlyphIndex < 0 &&
        vg__allocTextAtlas(ctx)) { // can not retrieve glyph?
      iter = prevIter;
      fonsTextIterNext(ctx->fs, &iter, &q); // try again
    }
    prevIter = iter;
    switch (iter.codepoint) {
    case 9:      // \t
    case 11:     // \v
    case 12:     // \f
    case 32:     // space
    case 0x00a0: // NBSP
      type = VG_SPACE;
      break;
    case 10: // \n
      type = pcodepoint == 13 ? VG_SPACE : VG_NEWLINE;
      break;
    case 13: // \r
      type = pcodepoint == 10 ? VG_SPACE : VG_NEWLINE;
      break;
    case 0x0085: // NEL
      type = VG_NEWLINE;
      break;
    default:
      if ((iter.codepoint >= 0x4E00 && iter.codepoint <= 0x9FFF) ||
          (iter.codepoint >= 0x3000 && iter.codepoint <= 0x30FF) ||
          (iter.codepoint >= 0xFF00 && iter.codepoint <= 0xFFEF) ||
          (iter.codepoint >= 0x1100 && iter.codepoint <= 0x11FF) ||
          (iter.codepoint >= 0x3130 && iter.codepoint <= 0x318F) ||
          (iter.codepoint >= 0xAC00 && iter.codepoint <= 0xD7AF))
        type = VG_CJK_CHAR;
      else
        type = VG_CHAR;
      break;
    }

    if (type == VG_NEWLINE) {
      // Always handle new lines.
      rows[nrows].start = rowStart != NULL ? rowStart : iter.str;
      rows[nrows].end = rowEnd != NULL ? rowEnd : iter.str;
      rows[nrows].width = rowWidth * invscale;
      rows[nrows].minx = rowMinX * invscale;
      rows[nrows].maxx = rowMaxX * invscale;
      rows[nrows].next = iter.next;
      nrows++;
      if (nrows >= maxRows)
        return nrows;
      // Set null break point
      breakEnd = rowStart;
      breakWidth = 0.0;
      breakMaxX = 0.0;
      // Indicate to skip the white space at the beginning of the row.
      rowStart = NULL;
      rowEnd = NULL;
      rowWidth = 0;
      rowMinX = rowMaxX = 0;
    } else {
      if (rowStart == NULL) {
        // Skip white space until the beginning of the line
        if (type == VG_CHAR || type == VG_CJK_CHAR) {
          // The current char is the row so far
          rowStartX = iter.x;
          rowStart = iter.str;
          rowEnd = iter.next;
          rowWidth = iter.nextx - rowStartX;
          rowMinX = q.x0 - rowStartX;
          rowMaxX = q.x1 - rowStartX;
          wordStart = iter.str;
          wordStartX = iter.x;
          wordMinX = q.x0 - rowStartX;
          // Set null break point
          breakEnd = rowStart;
          breakWidth = 0.0;
          breakMaxX = 0.0;
        }
      } else {
        float nextWidth = iter.nextx - rowStartX;

        // track last non-white space character
        if (type == VG_CHAR || type == VG_CJK_CHAR) {
          rowEnd = iter.next;
          rowWidth = iter.nextx - rowStartX;
          rowMaxX = q.x1 - rowStartX;
        }
        // track last end of a word
        if (((ptype == VG_CHAR || ptype == VG_CJK_CHAR) && type == VG_SPACE) ||
            type == VG_CJK_CHAR) {
          breakEnd = iter.str;
          breakWidth = rowWidth;
          breakMaxX = rowMaxX;
        }
        // track last beginning of a word
        if ((ptype == VG_SPACE && (type == VG_CHAR || type == VG_CJK_CHAR)) ||
            type == VG_CJK_CHAR) {
          wordStart = iter.str;
          wordStartX = iter.x;
          wordMinX = q.x0;
        }

        // Break to new line when a character is beyond break width.
        if ((type == VG_CHAR || type == VG_CJK_CHAR) &&
            nextWidth > breakRowWidth) {
          // The run length is too long, need to break to new line.
          if (breakEnd == rowStart) {
            // The current word is longer than the row length, just break it
            // from here.
            rows[nrows].start = rowStart;
            rows[nrows].end = iter.str;
            rows[nrows].width = rowWidth * invscale;
            rows[nrows].minx = rowMinX * invscale;
            rows[nrows].maxx = rowMaxX * invscale;
            rows[nrows].next = iter.str;
            nrows++;
            if (nrows >= maxRows)
              return nrows;
            rowStartX = iter.x;
            rowStart = iter.str;
            rowEnd = iter.next;
            rowWidth = iter.nextx - rowStartX;
            rowMinX = q.x0 - rowStartX;
            rowMaxX = q.x1 - rowStartX;
            wordStart = iter.str;
            wordStartX = iter.x;
            wordMinX = q.x0 - rowStartX;
          } else {
            // Break the line from the end of the last word, and start new line
            // from the beginning of the new.
            rows[nrows].start = rowStart;
            rows[nrows].end = breakEnd;
            rows[nrows].width = breakWidth * invscale;
            rows[nrows].minx = rowMinX * invscale;
            rows[nrows].maxx = breakMaxX * invscale;
            rows[nrows].next = wordStart;
            nrows++;
            if (nrows >= maxRows)
              return nrows;
            // Update row
            rowStartX = wordStartX;
            rowStart = wordStart;
            rowEnd = iter.next;
            rowWidth = iter.nextx - rowStartX;
            rowMinX = wordMinX - rowStartX;
            rowMaxX = q.x1 - rowStartX;
          }
          // Set null break point
          breakEnd = rowStart;
          breakWidth = 0.0;
          breakMaxX = 0.0;
        }
      }
    }

    pcodepoint = iter.codepoint;
    ptype = type;
  }

  // Break the line from the end of the last word, and start new line from the
  // beginning of the new.
  if (rowStart != NULL) {
    rows[nrows].start = rowStart;
    rows[nrows].end = rowEnd;
    rows[nrows].width = rowWidth * invscale;
    rows[nrows].minx = rowMinX * invscale;
    rows[nrows].maxx = rowMaxX * invscale;
    rows[nrows].next = end;
    nrows++;
  }

  return nrows;
}

float vgTextBounds(VGcontext *ctx, float x, float y, const char *string,
                   const char *end, float *bounds) {
  VGstate *state = vg__getState(ctx);
  float scale = vg__getFontScale(state) * ctx->devicePxRatio;
  float invscale = 1.0f / scale;
  float width;

  if (state->fontId == FONS_INVALID)
    return 0;

  fonsSetSize(ctx->fs, state->fontSize * scale);
  fonsSetSpacing(ctx->fs, state->letterSpacing * scale);
  fonsSetBlur(ctx->fs, state->fontBlur * scale);
  fonsSetAlign(ctx->fs, state->textAlign);
  fonsSetFont(ctx->fs, state->fontId);

  width = fonsTextBounds(ctx->fs, x * scale, y * scale, string, end, bounds);
  if (bounds != NULL) {
    // Use line bounds for height.
    fonsLineBounds(ctx->fs, y * scale, &bounds[1], &bounds[3]);
    bounds[0] *= invscale;
    bounds[1] *= invscale;
    bounds[2] *= invscale;
    bounds[3] *= invscale;
  }
  return width * invscale;
}

void vgTextBoxBounds(VGcontext *ctx, float x, float y, float breakRowWidth,
                     const char *string, const char *end, float *bounds) {
  VGstate *state = vg__getState(ctx);
  VGtextRow rows[2];
  float scale = vg__getFontScale(state) * ctx->devicePxRatio;
  float invscale = 1.0f / scale;
  int nrows = 0, i;
  int oldAlign = state->textAlign;
  int halign =
      state->textAlign & (VG_ALIGN_LEFT | VG_ALIGN_CENTER | VG_ALIGN_RIGHT);
  int valign = state->textAlign & (VG_ALIGN_TOP | VG_ALIGN_MIDDLE |
                                   VG_ALIGN_BOTTOM | VG_ALIGN_BASELINE);
  float lineh = 0, rminy = 0, rmaxy = 0;
  float minx, miny, maxx, maxy;

  if (state->fontId == FONS_INVALID) {
    if (bounds != NULL)
      bounds[0] = bounds[1] = bounds[2] = bounds[3] = 0.0f;
    return;
  }

  vgTextMetrics(ctx, NULL, NULL, &lineh);

  state->textAlign = VG_ALIGN_LEFT | valign;

  minx = maxx = x;
  miny = maxy = y;

  fonsSetSize(ctx->fs, state->fontSize * scale);
  fonsSetSpacing(ctx->fs, state->letterSpacing * scale);
  fonsSetBlur(ctx->fs, state->fontBlur * scale);
  fonsSetAlign(ctx->fs, state->textAlign);
  fonsSetFont(ctx->fs, state->fontId);
  fonsLineBounds(ctx->fs, 0, &rminy, &rmaxy);
  rminy *= invscale;
  rmaxy *= invscale;

  while ((nrows = vgTextBreakLines(ctx, string, end, breakRowWidth, rows, 2))) {
    for (i = 0; i < nrows; i++) {
      VGtextRow *row = &rows[i];
      float rminx, rmaxx, dx = 0;
      // Horizontal bounds
      if (halign & VG_ALIGN_LEFT)
        dx = 0;
      else if (halign & VG_ALIGN_CENTER)
        dx = breakRowWidth * 0.5f - row->width * 0.5f;
      else if (halign & VG_ALIGN_RIGHT)
        dx = breakRowWidth - row->width;
      rminx = x + row->minx + dx;
      rmaxx = x + row->maxx + dx;
      minx = vg__minf(minx, rminx);
      maxx = vg__maxf(maxx, rmaxx);
      // Vertical bounds.
      miny = vg__minf(miny, y + rminy);
      maxy = vg__maxf(maxy, y + rmaxy);

      y += lineh * state->lineHeight;
    }
    string = rows[nrows - 1].next;
  }

  state->textAlign = oldAlign;

  if (bounds != NULL) {
    bounds[0] = minx;
    bounds[1] = miny;
    bounds[2] = maxx;
    bounds[3] = maxy;
  }
}

void vgTextMetrics(VGcontext *ctx, float *ascender, float *descender,
                   float *lineh) {
  VGstate *state = vg__getState(ctx);
  float scale = vg__getFontScale(state) * ctx->devicePxRatio;
  float invscale = 1.0f / scale;

  if (state->fontId == FONS_INVALID)
    return;

  fonsSetSize(ctx->fs, state->fontSize * scale);
  fonsSetSpacing(ctx->fs, state->letterSpacing * scale);
  fonsSetBlur(ctx->fs, state->fontBlur * scale);
  fonsSetAlign(ctx->fs, state->textAlign);
  fonsSetFont(ctx->fs, state->fontId);

  fonsVertMetrics(ctx->fs, ascender, descender, lineh);
  if (ascender != NULL)
    *ascender *= invscale;
  if (descender != NULL)
    *descender *= invscale;
  if (lineh != NULL)
    *lineh *= invscale;
}
