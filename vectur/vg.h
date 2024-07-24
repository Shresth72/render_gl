#ifndef VG_H
#define VG_H

#ifdef __cplusplus
extern "C" {
#endif

#define VG_PI 3.14159265358979323846264338327f

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif // !_MSC_VER

//------------------------------------------//
//---------- VG ENUMS AND STRUCTS ----------//
//------------------------------------------//

typedef struct VGcontext VGcontext;

struct VGcolor {
  union {
    float rgba[4];
    struct {
      float r, g, b, a;
    };
  };
};
typedef struct VGcolor VGcolor;

struct VGpaint {
  float xform[6];
  float extent[2];
  float radius;
  float feather;
  VGcolor innerColor;
  VGcolor outerColor;
  int image;
};
typedef struct VGpaint VGpaint;

enum VGwinding {
  VG_CCW = 1, // Winding for solid shapes
  VG_CW = 2,  // Winding for holes
};

enum VGsolidity {
  VG_SOLID = 1, // CCW
  VG_HOLE = 2,  // CW
};

enum VGlineCap {
  VG_BUTT,
  VG_ROUND,
  VG_SQUARE,
  VG_BEVEL,
  VG_MITER,
};

enum VGalign {
  // Horizontal
  VG_ALIGN_LEFT = 1 << 0, // default
  VG_ALIGN_CENTER = 1 << 1,
  VG_ALIGN_RIGHT = 1 << 2,
  // Vertical
  VG_ALIGN_TOP = 1 << 3,
  VG_ALIGN_MIDDLE = 1 << 4,
  VG_ALIGN_BOTTOM = 1 << 5,
  VG_ALIGN_BASELINE = 1 << 6, // default
};

enum VGblendFactor {
  VG_ZERO = 1 << 0,
  VG_ONE = 1 << 1,
  VG_SRC_COLOR = 1 << 2,
  VG_ONE_MINUS_SRC_COLOR = 1 << 3,
  VG_DST_COLOR = 1 << 4,
  VG_ONE_MINUS_DST_COLOR = 1 << 5,
  VG_SRC_ALPHA = 1 << 6,
  VG_ONE_MINUS_SRC_ALPHA = 1 << 7,
  VG_DST_ALPHA = 1 << 8,
  VG_ONE_MINUS_DST_ALPHA = 1 << 9,
  VG_SRC_ALPHA_SATURATE = 1 << 10,
};

enum VGcompositeOperation {
  VG_SOURCE_OVER,
  VG_SOURCE_IN,
  VG_SOURCE_OUT,
  VG_ATOP,
  VG_DESTINATION_OVER,
  VG_DESTINATION_IN,
  VG_DESTINATION_OUT,
  VG_DESTINATION_ATOP,
  VG_LIGHTER,
  VG_COPY,
  VG_XOR,
};

struct VGcompositeOperationState {
  int srcRGB;
  int dstRGB;
  int srcAlpha;
  int dstAlpha;
};
typedef struct VGcompositeOperationState VGcompositeOperationState;

struct VGglyphPosition {
  const char *str;
  float x;
  float minx, maxx; // bounds
};
typedef struct VGglyphPosition VGglyphPosition;

struct VGtextRow {
  const char *start;
  const char *end;
  const char *next;
  float width;
  float minx, maxx;
};
typedef struct VGtextRow VGtextRow;

enum VGimageFlags {
  VG_IMAGE_GENERATE_MIPMAPS =
      1 << 0,                // Generate mipmaps during creation of the image.
  VG_IMAGE_REPEATX = 1 << 1, // Repeat image in X direction.
  VG_IMAGE_REPEATY = 1 << 2, // Repeat image in Y direction.
  VG_IMAGE_FLIPY =
      1 << 3, // Flips (inverses) image in Y direction when rendered.
  VG_IMAGE_PREMULTIPLIED = 1 << 4, // Image data has premultiplied alpha.
  VG_IMAGE_NEAREST = 1 << 5, // Image interpolation is Nearest instead Linear
};

//------------------------------------------//
//------------- VG DRAWING API -------------//
//------------------------------------------//

//
// Frame Operations
//

void vgBeginFrame(VGcontext *ctx, float windowWidth, float windowHeight,
                  float devicePixelRatio);

void vgCancelFrame(VGcontext *ctx);

void vgEndFrame(VGcontext *ctx);

//
// Composite Operations
//

// Sets the composite operation. The op parameter is of type
// vgGlobalCompositeOperation
void vgGlobalCompositeOperation(VGcontext *ctx, int op);

// Sets the composite operation with custom pixel arithmetic.
void vgGlobalCompositeBlendFunc(VGcontext *ctx, int sfactor, int dfactor);

// Sets the composite operation with custom pixel arithmetic for RGB and alpha
// components separately.
void vgGlobalCompositeBlendFuncSeperate(VGcontext *ctx, int srcRGB, int dstRGB,
                                        int srcAlpha, int dstAlpha);

//
// Color Utils
//

VGcolor vgRGB(unsigned char r, unsigned char g, unsigned char b);

VGcolor vgRGBf(float r, float g, float b);

VGcolor vgRGBA(unsigned char r, unsigned char g, unsigned char b,
               unsigned char a);

VGcolor vgRGBAf(float r, float g, float b, float a);

VGcolor vgLerpRGBA(VGcolor c0, VGcolor c1, float u);

VGcolor vgTransRGBA(VGcolor c0, unsigned char a);

VGcolor vgTransRGBAf(VGcolor c0, float a);

VGcolor vgHSL(float h, float s, float l);

VGcolor vgHSLA(float h, float s, float l, unsigned char a);

//
// State Handling
//
// State represents how paths will be rendered.
// Contains transform, fill, stroke styles, text, font styles and clip

// Pushes and saves the current render state into a state stack.
// A matching vgRestore() must be used to restore the state.
void vgSave(VGcontext *ctx);

// Pops and restores current render state.
void vgRestore(VGcontext *ctx);

// Resets current render state to default values. Does not affect the render
// state stack.
void vgReset(VGcontext *ctx);

//
// Render Style
//
// Fill and Stroker Render Style - Solid, Gradient, Pattern
// Gradient - vgLinearGradient(), vgBoxGradient(), vgRadialGradient(),
// vgImagePattern()

// Sets whether to draw antialias for vgStroke() and vgFill(). It's enabled by
// default.
void vgShapeAntiAlias(VGcontext *ctx, int enabled);

// Sets current stroke style to a solid color.
void vgStrokeColor(VGcontext *ctx, VGcolor color);

// Sets current stroke style to a paint, which can be a one of the gradients or
// a pattern.
void vgStrokePaint(VGcontext *ctx, VGpaint paint);

// Sets current fill style to a solid color.
void vgFillColor(VGcontext *ctx, VGcolor color);

// Sets current fill style to a paint, which can be a one of the gradients or a
// pattern.
void vgFillPaint(VGcontext *ctx, VGpaint paint);

// Sets the miter limit of the stroke style.
// Miter limit controls when a sharp corner is beveled.
void vgMiterLimit(VGcontext *ctx, float limit);

// Sets the stroke width of the stroke style.
void vgStrokeWidth(VGcontext *ctx, float size);

// Sets how the end of the line (cap) is drawn,
// Can be one of: vg_BUTT (default), VG_ROUND, VG_SQUARE.
void vgLineCap(VGcontext *ctx, int cap);

// Sets how sharp path corners are drawn.
// Can be one of vg_MITER (default), VG_ROUND, VG_BEVEL.
void vgLineJoin(VGcontext *ctx, int join);

// Sets the transparency applied to all rendered shapes.
// Already transparent paths will get proportionally more transparent as well.
void vgGlobalAlpha(VGcontext *ctx, float alpha);

//
// Transforms
//
// Paths, Gradients, Pattern and Clips can be transformed
// using Transformation matrix
// TODO: use something else
// [sx kx tx]
// [ky sy ty]
// [ 0  0  1]
// -> sx,sy define scaling, kx,ky skewing, and tx,ty translation

// Resets current transform to a identity matrix.
void vgResetTransform(VGcontext *ctx);

// Premultiplies current coordinate system by specified matrix.
// The parameters are interpreted as matrix as follows:
//   [a c e]
//   [b d f]
//   [0 0 1]
void vgTransform(VGcontext *ctx, float a, float b, float c, float d, float e,
                 float f);

// Translates current coordinate system.
void vgTranslate(VGcontext *ctx, float x, float y);

// Rotates current coordinate system. Angle is specified in radians.
void vgRotate(VGcontext *ctx, float angle);

// Skews the current coordinate system along X axis. Angle is specified in
// radians.
void vgSkewX(VGcontext *ctx, float angle);

// Skews the current coordinate system along Y axis. Angle is specified in
// radians.
void vgSkewY(VGcontext *ctx, float angle);

// Scales the current coordinate system.
void vgScale(VGcontext *ctx, float x, float y);

// Stores the top part (a-f) of the current transformation matrix in to the
// specified buffer.
//   [a c e]
//   [b d f]
//   [0 0 1]
// There should be space for 6 floats in the return buffer for the values a-f.
void vgCurrentTransform(VGcontext *ctx, float *xform);

// The following functions can be used to make calculations on 2x3
// transformation matrices. A 2x3 matrix is represented as float[6].

// Sets the transform to identity matrix.
void vgTransformIdentity(float *dst);

// Sets the transform to translation matrix matrix.
void vgTransformTranslate(float *dst, float tx, float ty);

// Sets the transform to scale matrix.
void vgTransformScale(float *dst, float sx, float sy);

// Sets the transform to rotate matrix. Angle is specified in radians.
void vgTransformRotate(float *dst, float a);

// Sets the transform to skew-x matrix. Angle is specified in radians.
void vgTransformSkewX(float *dst, float a);

// Sets the transform to skew-y matrix. Angle is specified in radians.
void vgTransformSkewY(float *dst, float a);

// Sets the transform to the result of multiplication of two transforms, of A =
// A*B.
void vgTransformMultiply(float *dst, const float *src);

// Sets the transform to the result of multiplication of two transforms, of A =
// B*A.
void vgTransformPremultiply(float *dst, const float *src);

// Sets the destination to inverse of specified transform.
// Returns 1 if the inverse could be calculated, else 0.
int vgTransformInverse(float *dst, const float *src);

// Transform a point by given transform.
void vgTransformPoint(float *dstx, float *dsty, const float *xform, float srcx,
                      float srcy);

// Converts degrees to radians and vice versa.
float vgDegToRad(float deg);
float vgRadToDeg(float rad);

//
// Images
//
// Load and Renders jpg, png, psd, tga, pic anf gif
// Upload image using stb_image

// Creates image by loading it from the disk from specified file name.
// Returns handle to the image.
int vgCreateImage(VGcontext *ctx, const char *filename, int imageFlags);

// Creates image by loading it from the specified chunk of memory.
// Returns handle to the image.
int vgCreateImageMem(VGcontext *ctx, int imageFlags, unsigned char *data,
                     int ndata);

// Creates image from specified image data.
// Returns handle to the image.
int vgCreateImageRGBA(VGcontext *ctx, int w, int h, int imageFlags,
                      const unsigned char *data);

// Updates image data specified by image handle.
void vgUpdateImage(VGcontext *ctx, int image, const unsigned char *data);

// Returns the dimensions of a created image.
void vgImageSize(VGcontext *ctx, int image, int *w, int *h);

// Deletes created image.
void vgDeleteImage(VGcontext *ctx, int image);

//
// Paints
//
// Linear, Box and Radial gradients
// Gradient and Gmage pattern.
// These can be used as paints for strokes and fills

// Creates and returns a linear gradient. Parameters (sx,sy)-(ex,ey) specify the
// start and end coordinates of the linear gradient, icol specifies the start
// color and ocol the end color. The gradient is transformed by the current
// transform when it is passed to vgFillPaint() or vgStrokePaint().
VGpaint vgLinearGradient(VGcontext *ctx, float sx, float sy, float ex, float ey,
                         VGcolor icol, VGcolor ocol);

// Creates and returns a box gradient. Box gradient is a feathered rounded
// rectangle, it is useful for rendering drop shadows or highlights for boxes.
// Parameters (x,y) define the top-left corner of the rectangle, (w,h) define
// the size of the rectangle, r defines the corner radius, and f feather.
// Feather defines how blurry the border of the rectangle is. Parameter icol
// specifies the inner color and ocol the outer color of the gradient. The
// gradient is transformed by the current transform when it is passed to
// vgFillPaint() or vgStrokePaint().
VGpaint vgBoxGradient(VGcontext *ctx, float x, float y, float w, float h,
                      float r, float f, VGcolor icol, VGcolor ocol);

// Creates and returns a radial gradient. Parameters (cx,cy) specify the center,
// inr and outr specify the inner and outer radius of the gradient, icol
// specifies the start color and ocol the end color. The gradient is transformed
// by the current transform when it is passed to vgFillPaint() or
// vgStrokePaint().
VGpaint vgRadialGradient(VGcontext *ctx, float cx, float cy, float inr,
                         float outr, VGcolor icol, VGcolor ocol);

// Creates and returns an image pattern. Parameters (ox,oy) specify the left-top
// location of the image pattern, (ex,ey) the size of one image, angle rotation
// around the top-left corner, image is handle to the image to render. The
// gradient is transformed by the current transform when it is passed to
// vgFillPaint() or vgStrokePaint().
VGpaint vgImagePattern(VGcontext *ctx, float ox, float oy, float ex, float ey,
                       float angle, int image, float alpha);

//
// Scissor Clip
//
// Scissoring allows you to clip the rendering into a rectangle. This is useful
// for various user interface cases like rendering a text edit or a timeline.

// Sets the current scissor rectangle.
// The scissor rectangle is transformed by the current transform.
void vgScissor(VGcontext *ctx, float x, float y, float w, float h);

// Intersects current scissor rectangle with the specified rectangle.
// The scissor rectangle is transformed by the current transform.
// Note: in case the rotation of previous scissor rect differs from
// the current one, the intersection will be done between the specified
// rectangle and the previous scissor rectangle transformed in the current
// transform space. The resulting shape is always rectangle.
void vgIntersectScissor(VGcontext *ctx, float x, float y, float w, float h);

// Reset and disables scissoring.
void vgResetScissor(VGcontext *ctx);

//
// Paths
//
// For shapes and vectors
// Begin Path - vgBeginPath()
// Define winding for solid or hole fill - vgPathWinding()
// Fill Path - vgFill()
// Stroke Style - vgStroke()

// Clears the current path and sub-paths.
void vgBeginPath(VGcontext *ctx);

// Starts new sub-path with specified point as first point.
void vgMoveTo(VGcontext *ctx, float x, float y);

// Adds line segment from the last point in the path to the specified point.
void vgLineTo(VGcontext *ctx, float x, float y);

// Adds cubic bezier segment from last point in the path via two control points
// to the specified point.
void vgBezierTo(VGcontext *ctx, float c1x, float c1y, float c2x, float c2y,
                float x, float y);

// Adds quadratic bezier segment from last point in the path via a control point
// to the specified point.
void vgQuadTo(VGcontext *ctx, float cx, float cy, float x, float y);

// Adds an arc segment at the corner defined by the last path point, and two
// specified points.
void vgArcTo(VGcontext *ctx, float x1, float y1, float x2, float y2,
             float radius);

// Closes current sub-path with a line segment.
void vgClosePath(VGcontext *ctx);

// Sets the current sub-path winding, see VGwinding and VGsolidity.
void vgPathWinding(VGcontext *ctx, int dir);

// Creates new circle arc shaped sub-path. The arc center is at cx,cy, the arc
// radius is r, and the arc is drawn from angle a0 to a1, and swept in direction
// dir (VG_CCW, or VG_CW). Angles are specified in radians.
void vgArc(VGcontext *ctx, float cx, float cy, float r, float a0, float a1,
           int dir);

// Creates new rectangle shaped sub-path.
void vgRect(VGcontext *ctx, float x, float y, float w, float h);

// Creates new rounded rectangle shaped sub-path.
void vgRoundedRect(VGcontext *ctx, float x, float y, float w, float h, float r);

// Creates new rounded rectangle shaped sub-path with varying radii for each
// corner.
void vgRoundedRectVarying(VGcontext *ctx, float x, float y, float w, float h,
                          float radTopLeft, float radTopRight,
                          float radBottomRight, float radBottomLeft);

// Creates new ellipse shaped sub-path.
void vgEllipse(VGcontext *ctx, float cx, float cy, float rx, float ry);

// Creates new circle shaped sub-path.
void vgCircle(VGcontext *ctx, float cx, float cy, float r);

// Fills the current path with current fill style.
void vgFill(VGcontext *ctx);

// Fills the current path with current stroke style.
void vgStroke(VGcontext *ctx);

//
// Text
//
// Load and Render .ttf files

// Creates font by loading it from the disk from specified file name.
// Returns handle to the font.
int vgCreateFont(VGcontext *ctx, const char *name, const char *filename);

// fontIndex specifies which font face to load from a .ttf/.ttc file.
int vgCreateFontAtIndex(VGcontext *ctx, const char *name, const char *filename,
                        const int fontIndex);

// Creates font by loading it from the specified memory chunk.
// Returns handle to the font.
int vgCreateFontMem(VGcontext *ctx, const char *name, unsigned char *data,
                    int ndata, int freeData);

// fontIndex specifies which font face to load from a .ttf/.ttc file.
int vgCreateFontMemAtIndex(VGcontext *ctx, const char *name,
                           unsigned char *data, int ndata, int freeData,
                           const int fontIndex);

// Finds a loaded font of specified name, and returns handle to it, or -1 if the
// font is not found.
int vgFindFont(VGcontext *ctx, const char *name);

// Adds a fallback font by handle.
int vgAddFallbackFontId(VGcontext *ctx, int baseFont, int fallbackFont);

// Adds a fallback font by name.
int vgAddFallbackFont(VGcontext *ctx, const char *baseFont,
                      const char *fallbackFont);

// Resets fallback fonts by handle.
void vgResetFallbackFontsId(VGcontext *ctx, int baseFont);

// Resets fallback fonts by name.
void vgResetFallbackFonts(VGcontext *ctx, const char *baseFont);

// Sets the font size of current text style.
void vgFontSize(VGcontext *ctx, float size);

// Sets the blur of current text style.
void vgFontBlur(VGcontext *ctx, float blur);

// Sets the letter spacing of current text style.
void vgTextLetterSpacing(VGcontext *ctx, float spacing);

// Sets the proportional line height of current text style. The line height is
// specified as multiple of font size.
void vgTextLineHeight(VGcontext *ctx, float lineHeight);

// Sets the text align of current text style, see VGalign for options.
void vgTextAlign(VGcontext *ctx, int align);

// Sets the font face based on specified id of current text style.
void vgFontFaceId(VGcontext *ctx, int font);

// Sets the font face based on specified name of current text style.
void vgFontFace(VGcontext *ctx, const char *font);

// Draws text string at specified location. If end is specified only the
// sub-string up to the end is drawn.
float vgText(VGcontext *ctx, float x, float y, const char *string,
             const char *end);

// Draws multi-line text string at specified location wrapped at the specified
// width. If end is specified only the sub-string up to the end is drawn. White
// space is stripped at the beginning of the rows, the text is split at word
// boundaries or when new-line characters are encountered. Words longer than the
// max width are slit at nearest character (i.e. no hyphenation).
void vgTextBox(VGcontext *ctx, float x, float y, float breakRowWidth,
               const char *string, const char *end);

// Measures the specified text string. Parameter bounds should be a pointer to
// float[4], if the bounding box of the text should be returned. The bounds
// value are [xmin,ymin, xmax,ymax] Returns the horizontal advance of the
// measured text (i.e. where the next character should drawn). Measured values
// are returned in local coordinate space.
float vgTextBounds(VGcontext *ctx, float x, float y, const char *string,
                   const char *end, float *bounds);

// Measures the specified multi-text string. Parameter bounds should be a
// pointer to float[4], if the bounding box of the text should be returned. The
// bounds value are [xmin,ymin, xmax,ymax] Measured values are returned in local
// coordinate space.
void vgTextBoxBounds(VGcontext *ctx, float x, float y, float breakRowWidth,
                     const char *string, const char *end, float *bounds);

// Calculates the glyph x positions of the specified text. If end is specified
// only the sub-string will be used. Measured values are returned in local
// coordinate space.
int vgTextGlyphPositions(VGcontext *ctx, float x, float y, const char *string,
                         const char *end, VGglyphPosition *positions,
                         int maxPositions);

// Returns the vertical metrics based on the current text style.
// Measured values are returned in local coordinate space.
void vgTextMetrics(VGcontext *ctx, float *ascender, float *descender,
                   float *lineh);

// Breaks the specified text into lines. If end is specified only the sub-string
// will be used. White space is stripped at the beginning of the rows, the text
// is split at word boundaries or when new-line characters are encountered.
// Words longer than the max width are slit at nearest character (i.e. no
// hyphenation).
int vgTextBreakLines(VGcontext *ctx, const char *string, const char *end,
                     float breakRowWidth, VGtextRow *rows, int maxRows);

//------------------------------------------//
//------------- VG INTERNAL API ------------//
//------------------------------------------//

enum VGtexture {
  VG_TEXTURE_ALPHA = 0x01,
  VG_TEXTURE_RGBA = 0x02,
};

struct VGscissor {
  float xform[6];
  float extent[2];
};
typedef struct VGscissor VGscissor;

struct VGvertex {
  float x, y, u, v;
};
typedef struct VGvertex VGvertex;

struct VGpath {
  int first;
  int count;
  unsigned char closed;
  int nbevel;
  VGvertex *fill;
  int nfill;
  VGvertex *stroke;
  int nstroke;
  int winding;
  int convex;
};
typedef struct VGpath VGpath;

struct VGparams {
  void *userPtr;
  int edgeAntiAlias;
  int (*renderCreate)(void *uptr);
  int (*renderCreateTexture)(void *uptr, int type, int w, int h, int imageFlags,
                             const unsigned char *data);
  int (*renderDeleteTexture)(void *uptr, int image);
  int (*renderUpdateTexture)(void *uptr, int image, int x, int y, int w, int h,
                             const unsigned char *data);
  int (*renderGetTextureSize)(void *uptr, int image, int *w, int *h);
  void (*renderViewport)(void *uptr, float width, float height,
                         float devicePixelRatio);
  void (*renderCancel)(void *uptr);
  void (*renderFlush)(void *uptr);
  void (*renderFill)(void *uptr, VGpaint *paint,
                     VGcompositeOperationState compositeOperation,
                     VGscissor *scissor, float fringe, const float *bounds,
                     const VGpath *paths, int npaths);
  void (*renderStroke)(void *uptr, VGpaint *paint,
                       VGcompositeOperationState compositeOperation,
                       VGscissor *scissor, float fringe, float strokeWidth,
                       const VGpath *paths, int npaths);
  void (*renderTriangles)(void *uptr, VGpaint *paint,
                          VGcompositeOperationState compositeOperation,
                          VGscissor *scissor, const VGvertex *verts, int nverts,
                          float fringe);
  void (*renderDelete)(void *uptr);
};
typedef struct VGparams VGparams;

// constructor and destructor called by render backend
VGcontext *vgCreateInternal(VGparams *params);
void vgDeleteInternal(VGcontext *ctx);

VGparams *vgInternalParams(VGcontext *ctx);

// Debug function to dump cached path data
void vgDebugDumpPathCache(VGcontext *ctx);

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#define VG_NOTUSED(v)                                                          \
  for (;;) {                                                                   \
    (void)(1 ? (void)0 : ((void)(v)));                                         \
    break;                                                                     \
  }

#ifdef __cplusplus
}
#endif

#endif // !VG_H
