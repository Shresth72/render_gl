#ifndef VG_GL_H
#define VG_GL_H

#ifdef __cplusplus
extern "C" {
#endif

// Create flags

enum VGcreateFlags {
  // Flag indicating if geometry based anti-aliasing is used (may not be needed
  // when using MSAA).
  VG_ANTIALIAS = 1 << 0,
  // Flag indicating if strokes should be drawn using stencil buffer. The
  // rendering will be a little
  // slower, but path overlaps (i.e. self-intersecting or sharp turns) will be
  // drawn just once.
  VG_STENCIL_STROKES = 1 << 1,
  // Flag indicating that additional debug checks are done.
  VG_DEBUG = 1 << 2,
};

#if defined VG_GL2_IMPLEMENTATION
#define VG_GL2 1
#define VG_GL_IMPLEMENTATION 1
#elif defined VG_GL3_IMPLEMENTATION
#define VG_GL3 1
#define VG_GL_IMPLEMENTATION 1
#define VG_GL_USE_UNIFORMBUFFER 1
#elif defined VG_GLES2_IMPLEMENTATION
#define VG_GLES2 1
#define VG_GL_IMPLEMENTATION 1
#elif defined VG_GLES3_IMPLEMENTATION
#define VG_GLES3 1
#define VG_GL_IMPLEMENTATION 1
#endif

#define VG_GL_USE_STATE_FILTER (1)

// Creates VG contexts for different OpenGL (ES) versions.
// Flags should be combination of the create flags above.

#if defined VG_GL2

VGcontext *vgCreateGL2(int flags);
void vgDeleteGL2(VGcontext *ctx);

int vglCreateImageFromHandleGL2(VGcontext *ctx, GLuint textureId, int w, int h,
                                int flags);
GLuint vglImageHandleGL2(VGcontext *ctx, int image);

#endif

#if defined VG_GL3

VGcontext *vgCreateGL3(int flags);
void vgDeleteGL3(VGcontext *ctx);

int vglCreateImageFromHandleGL3(VGcontext *ctx, GLuint textureId, int w, int h,
                                int flags);
GLuint vglImageHandleGL3(VGcontext *ctx, int image);

#endif

#if defined VG_GLES2

VGcontext *vgCreateGLES2(int flags);
void vgDeleteGLES2(VGcontext *ctx);

int vglCreateImageFromHandleGLES2(VGcontext *ctx, GLuint textureId, int w,
                                  int h, int flags);
GLuint vglImageHandleGLES2(VGcontext *ctx, int image);

#endif

#if defined VG_GLES3

VGcontext *vgCreateGLES3(int flags);
void vgDeleteGLES3(VGcontext *ctx);

int vglCreateImageFromHandleGLES3(VGcontext *ctx, GLuint textureId, int w,
                                  int h, int flags);
GLuint vglImageHandleGLES3(VGcontext *ctx, int image);

#endif

// These are additional flags on top of VGimageFlags.
enum VGimageFlagsGL {
  VG_IMAGE_NODELETE = 1 << 16, // Do not delete GL texture handle.
};

#ifdef __cplusplus
}
#endif

#endif /* VG_GL_H */

#ifdef VG_GL_IMPLEMENTATION

#include "vg.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum GLVGuniformLoc {
  GLVG_LOC_VIEWSIZE,
  GLVG_LOC_TEX,
  GLVG_LOC_FRAG,
  GLVG_MAX_LOCS
};

enum GLVGshaderType {
  NSVG_SHADER_FILLGRAD,
  NSVG_SHADER_FILLIMG,
  NSVG_SHADER_SIMPLE,
  NSVG_SHADER_IMG
};

#if VG_GL_USE_UNIFORMBUFFER
enum GLVGuniformBindings {
  GLVG_FRAG_BINDING = 0,
};
#endif

struct GLVGshader {
  GLuint prog;
  GLuint frag;
  GLuint vert;
  GLint loc[GLVG_MAX_LOCS];
};
typedef struct GLVGshader GLVGshader;

struct GLVGtexture {
  int id;
  GLuint tex;
  int width, height;
  int type;
  int flags;
};
typedef struct GLVGtexture GLVGtexture;

struct GLVGblend {
  GLenum srcRGB;
  GLenum dstRGB;
  GLenum srcAlpha;
  GLenum dstAlpha;
};
typedef struct GLVGblend GLVGblend;

enum GLVGcallType {
  GLVG_NONE = 0,
  GLVG_FILL,
  GLVG_CONVEXFILL,
  GLVG_STROKE,
  GLVG_TRIANGLES,
};

struct GLVGcall {
  int type;
  int image;
  int pathOffset;
  int pathCount;
  int triangleOffset;
  int triangleCount;
  int uniformOffset;
  GLVGblend blendFunc;
};
typedef struct GLVGcall GLVGcall;

struct GLVGpath {
  int fillOffset;
  int fillCount;
  int strokeOffset;
  int strokeCount;
};
typedef struct GLVGpath GLVGpath;

struct GLVGfragUniforms {
#if VG_GL_USE_UNIFORMBUFFER
  float scissorMat[12]; // matrices are actually 3 vec4s
  float paintMat[12];
  struct VGcolor innerCol;
  struct VGcolor outerCol;
  float scissorExt[2];
  float scissorScale[2];
  float extent[2];
  float radius;
  float feather;
  float strokeMult;
  float strokeThr;
  int texType;
  int type;
#else
// note: after modifying layout or size of uniform array,
// don't forget to also update the fragment shader source!
#define VG_GL_UNIFORMARRAY_SIZE 11
  union {
    struct {
      float scissorMat[12]; // matrices are actually 3 vec4s
      float paintMat[12];
      struct VGcolor innerCol;
      struct VGcolor outerCol;
      float scissorExt[2];
      float scissorScale[2];
      float extent[2];
      float radius;
      float feather;
      float strokeMult;
      float strokeThr;
      float texType;
      float type;
    };
    float uniformArray[VG_GL_UNIFORMARRAY_SIZE][4];
  };
#endif
};
typedef struct GLVGfragUniforms GLVGfragUniforms;

struct GLVGcontext {
  GLVGshader shader;
  GLVGtexture *textures;
  float view[2];
  int ntextures;
  int ctextures;
  int textureId;
  GLuint vertBuf;
#if defined VG_GL3
  GLuint vertArr;
#endif
#if VG_GL_USE_UNIFORMBUFFER
  GLuint fragBuf;
#endif
  int fragSize;
  int flags;

  // Per frame buffers
  GLVGcall *calls;
  int ccalls;
  int ncalls;
  GLVGpath *paths;
  int cpaths;
  int npaths;
  struct VGvertex *verts;
  int cverts;
  int nverts;
  unsigned char *uniforms;
  int cuniforms;
  int nuniforms;

// cached state
#if VG_GL_USE_STATE_FILTER
  GLuint boundTexture;
  GLuint stencilMask;
  GLenum stencilFunc;
  GLint stencilFuncRef;
  GLuint stencilFuncMask;
  GLVGblend blendFunc;
#endif

  int dummyTex;
};
typedef struct GLVGcontext GLVGcontext;

static int glvg__maxi(int a, int b) { return a > b ? a : b; }

#ifdef VG_GLES2
static unsigned int glvg__nearestPow2(unsigned int num) {
  unsigned n = num > 0 ? num - 1 : 0;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  n++;
  return n;
}
#endif

static void glvg__bindTexture(GLVGcontext *gl, GLuint tex) {
#if VG_GL_USE_STATE_FILTER
  if (gl->boundTexture != tex) {
    gl->boundTexture = tex;
    glBindTexture(GL_TEXTURE_2D, tex);
  }
#else
  glBindTexture(GL_TEXTURE_2D, tex);
#endif
}

static void glvg__stencilMask(GLVGcontext *gl, GLuint mask) {
#if VG_GL_USE_STATE_FILTER
  if (gl->stencilMask != mask) {
    gl->stencilMask = mask;
    glStencilMask(mask);
  }
#else
  glStencilMask(mask);
#endif
}

static void glvg__stencilFunc(GLVGcontext *gl, GLenum func, GLint ref,
                              GLuint mask) {
#if VG_GL_USE_STATE_FILTER
  if ((gl->stencilFunc != func) || (gl->stencilFuncRef != ref) ||
      (gl->stencilFuncMask != mask)) {

    gl->stencilFunc = func;
    gl->stencilFuncRef = ref;
    gl->stencilFuncMask = mask;
    glStencilFunc(func, ref, mask);
  }
#else
  glStencilFunc(func, ref, mask);
#endif
}
static void glvg__blendFuncSeparate(GLVGcontext *gl, const GLVGblend *blend) {
#if VG_GL_USE_STATE_FILTER
  if ((gl->blendFunc.srcRGB != blend->srcRGB) ||
      (gl->blendFunc.dstRGB != blend->dstRGB) ||
      (gl->blendFunc.srcAlpha != blend->srcAlpha) ||
      (gl->blendFunc.dstAlpha != blend->dstAlpha)) {

    gl->blendFunc = *blend;
    glBlendFuncSeparate(blend->srcRGB, blend->dstRGB, blend->srcAlpha,
                        blend->dstAlpha);
  }
#else
  glBlendFuncSeparate(blend->srcRGB, blend->dstRGB, blend->srcAlpha,
                      blend->dstAlpha);
#endif
}

static GLVGtexture *glvg__allocTexture(GLVGcontext *gl) {
  GLVGtexture *tex = NULL;
  int i;

  for (i = 0; i < gl->ntextures; i++) {
    if (gl->textures[i].id == 0) {
      tex = &gl->textures[i];
      break;
    }
  }
  if (tex == NULL) {
    if (gl->ntextures + 1 > gl->ctextures) {
      GLVGtexture *textures;
      int ctextures = glvg__maxi(gl->ntextures + 1, 4) +
                      gl->ctextures / 2; // 1.5x Overallocate
      textures =
          (GLVGtexture *)realloc(gl->textures, sizeof(GLVGtexture) * ctextures);
      if (textures == NULL)
        return NULL;
      gl->textures = textures;
      gl->ctextures = ctextures;
    }
    tex = &gl->textures[gl->ntextures++];
  }

  memset(tex, 0, sizeof(*tex));
  tex->id = ++gl->textureId;

  return tex;
}

static GLVGtexture *glvg__findTexture(GLVGcontext *gl, int id) {
  int i;
  for (i = 0; i < gl->ntextures; i++)
    if (gl->textures[i].id == id)
      return &gl->textures[i];
  return NULL;
}

static int glvg__deleteTexture(GLVGcontext *gl, int id) {
  int i;
  for (i = 0; i < gl->ntextures; i++) {
    if (gl->textures[i].id == id) {
      if (gl->textures[i].tex != 0 &&
          (gl->textures[i].flags & VG_IMAGE_NODELETE) == 0)
        glDeleteTextures(1, &gl->textures[i].tex);
      memset(&gl->textures[i], 0, sizeof(gl->textures[i]));
      return 1;
    }
  }
  return 0;
}

static void glvg__dumpShaderError(GLuint shader, const char *name,
                                  const char *type) {
  GLchar str[512 + 1];
  GLsizei len = 0;
  glGetShaderInfoLog(shader, 512, &len, str);
  if (len > 512)
    len = 512;
  str[len] = '\0';
  printf("Shader %s/%s error:\n%s\n", name, type, str);
}

static void glvg__dumpProgramError(GLuint prog, const char *name) {
  GLchar str[512 + 1];
  GLsizei len = 0;
  glGetProgramInfoLog(prog, 512, &len, str);
  if (len > 512)
    len = 512;
  str[len] = '\0';
  printf("Program %s error:\n%s\n", name, str);
}

static void glvg__checkError(GLVGcontext *gl, const char *str) {
  GLenum err;
  if ((gl->flags & VG_DEBUG) == 0)
    return;
  err = glGetError();
  if (err != GL_NO_ERROR) {
    printf("Error %08x after %s\n", err, str);
    return;
  }
}

static int glvg__createShader(GLVGshader *shader, const char *name,
                              const char *header, const char *opts,
                              const char *vshader, const char *fshader) {
  GLint status;
  GLuint prog, vert, frag;
  const char *str[3];
  str[0] = header;
  str[1] = opts != NULL ? opts : "";

  memset(shader, 0, sizeof(*shader));

  prog = glCreateProgram();
  vert = glCreateShader(GL_VERTEX_SHADER);
  frag = glCreateShader(GL_FRAGMENT_SHADER);
  str[2] = vshader;
  glShaderSource(vert, 3, str, 0);
  str[2] = fshader;
  glShaderSource(frag, 3, str, 0);

  glCompileShader(vert);
  glGetShaderiv(vert, GL_COMPILE_STATUS, &status);
  if (status != GL_TRUE) {
    glvg__dumpShaderError(vert, name, "vert");
    return 0;
  }

  glCompileShader(frag);
  glGetShaderiv(frag, GL_COMPILE_STATUS, &status);
  if (status != GL_TRUE) {
    glvg__dumpShaderError(frag, name, "frag");
    return 0;
  }

  glAttachShader(prog, vert);
  glAttachShader(prog, frag);

  glBindAttribLocation(prog, 0, "vertex");
  glBindAttribLocation(prog, 1, "tcoord");

  glLinkProgram(prog);
  glGetProgramiv(prog, GL_LINK_STATUS, &status);
  if (status != GL_TRUE) {
    glvg__dumpProgramError(prog, name);
    return 0;
  }

  shader->prog = prog;
  shader->vert = vert;
  shader->frag = frag;

  return 1;
}

static void glvg__deleteShader(GLVGshader *shader) {
  if (shader->prog != 0)
    glDeleteProgram(shader->prog);
  if (shader->vert != 0)
    glDeleteShader(shader->vert);
  if (shader->frag != 0)
    glDeleteShader(shader->frag);
}

static void glvg__getUniforms(GLVGshader *shader) {
  shader->loc[GLVG_LOC_VIEWSIZE] =
      glGetUniformLocation(shader->prog, "viewSize");
  shader->loc[GLVG_LOC_TEX] = glGetUniformLocation(shader->prog, "tex");

#if VG_GL_USE_UNIFORMBUFFER
  shader->loc[GLVG_LOC_FRAG] = glGetUniformBlockIndex(shader->prog, "frag");
#else
  shader->loc[GLVG_LOC_FRAG] = glGetUniformLocation(shader->prog, "frag");
#endif
}

static int glvg__renderCreateTexture(void *uptr, int type, int w, int h,
                                     int imageFlags, const unsigned char *data);

static int glvg__renderCreate(void *uptr) {
  GLVGcontext *gl = (GLVGcontext *)uptr;
  int align = 4;

  // TODO: mediump float may not be enough for GLES2 in iOS.
  // see the following discussion: https://github.com/memononen/vg/issues/46
  static const char *shaderHeader =
#if defined VG_GL2
      "#define VG_GL2 1\n"
#elif defined VG_GL3
      "#version 150 core\n"
      "#define VG_GL3 1\n"
#elif defined VG_GLES2
      "#version 100\n"
      "#define VG_GL2 1\n"
#elif defined VG_GLES3
      "#version 300 es\n"
      "#define VG_GL3 1\n"
#endif

#if VG_GL_USE_UNIFORMBUFFER
      "#define USE_UNIFORMBUFFER 1\n"
#else
      "#define UNIFORMARRAY_SIZE 11\n"
#endif
      "\n";

  static const char *fillVertShader =
      "#ifdef VG_GL3\n"
      "	uniform vec2 viewSize;\n"
      "	in vec2 vertex;\n"
      "	in vec2 tcoord;\n"
      "	out vec2 ftcoord;\n"
      "	out vec2 fpos;\n"
      "#else\n"
      "	uniform vec2 viewSize;\n"
      "	attribute vec2 vertex;\n"
      "	attribute vec2 tcoord;\n"
      "	varying vec2 ftcoord;\n"
      "	varying vec2 fpos;\n"
      "#endif\n"
      "void main(void) {\n"
      "	ftcoord = tcoord;\n"
      "	fpos = vertex;\n"
      "	gl_Position = vec4(2.0*vertex.x/viewSize.x - 1.0, 1.0 - "
      "2.0*vertex.y/viewSize.y, 0, 1);\n"
      "}\n";

  static const char *fillFragShader =
      "#ifdef GL_ES\n"
      "#if defined(GL_FRAGMENT_PRECISION_HIGH) || defined(VG_GL3)\n"
      " precision highp float;\n"
      "#else\n"
      " precision mediump float;\n"
      "#endif\n"
      "#endif\n"
      "#ifdef VG_GL3\n"
      "#ifdef USE_UNIFORMBUFFER\n"
      "	layout(std140) uniform frag {\n"
      "		mat3 scissorMat;\n"
      "		mat3 paintMat;\n"
      "		vec4 innerCol;\n"
      "		vec4 outerCol;\n"
      "		vec2 scissorExt;\n"
      "		vec2 scissorScale;\n"
      "		vec2 extent;\n"
      "		float radius;\n"
      "		float feather;\n"
      "		float strokeMult;\n"
      "		float strokeThr;\n"
      "		int texType;\n"
      "		int type;\n"
      "	};\n"
      "#else\n" // VG_GL3 && !USE_UNIFORMBUFFER
      "	uniform vec4 frag[UNIFORMARRAY_SIZE];\n"
      "#endif\n"
      "	uniform sampler2D tex;\n"
      "	in vec2 ftcoord;\n"
      "	in vec2 fpos;\n"
      "	out vec4 outColor;\n"
      "#else\n" // !VG_GL3
      "	uniform vec4 frag[UNIFORMARRAY_SIZE];\n"
      "	uniform sampler2D tex;\n"
      "	varying vec2 ftcoord;\n"
      "	varying vec2 fpos;\n"
      "#endif\n"
      "#ifndef USE_UNIFORMBUFFER\n"
      "	#define scissorMat mat3(frag[0].xyz, frag[1].xyz, frag[2].xyz)\n"
      "	#define paintMat mat3(frag[3].xyz, frag[4].xyz, frag[5].xyz)\n"
      "	#define innerCol frag[6]\n"
      "	#define outerCol frag[7]\n"
      "	#define scissorExt frag[8].xy\n"
      "	#define scissorScale frag[8].zw\n"
      "	#define extent frag[9].xy\n"
      "	#define radius frag[9].z\n"
      "	#define feather frag[9].w\n"
      "	#define strokeMult frag[10].x\n"
      "	#define strokeThr frag[10].y\n"
      "	#define texType int(frag[10].z)\n"
      "	#define type int(frag[10].w)\n"
      "#endif\n"
      "\n"
      "float sdroundrect(vec2 pt, vec2 ext, float rad) {\n"
      "	vec2 ext2 = ext - vec2(rad,rad);\n"
      "	vec2 d = abs(pt) - ext2;\n"
      "	return min(max(d.x,d.y),0.0) + length(max(d,0.0)) - rad;\n"
      "}\n"
      "\n"
      "// Scissoring\n"
      "float scissorMask(vec2 p) {\n"
      "	vec2 sc = (abs((scissorMat * vec3(p,1.0)).xy) - scissorExt);\n"
      "	sc = vec2(0.5,0.5) - sc * scissorScale;\n"
      "	return clamp(sc.x,0.0,1.0) * clamp(sc.y,0.0,1.0);\n"
      "}\n"
      "#ifdef EDGE_AA\n"
      "// Stroke - from [0..1] to clipped pyramid, where the slope is 1px.\n"
      "float strokeMask() {\n"
      "	return min(1.0, (1.0-abs(ftcoord.x*2.0-1.0))*strokeMult) * min(1.0, "
      "ftcoord.y);\n"
      "}\n"
      "#endif\n"
      "\n"
      "void main(void) {\n"
      "   vec4 result;\n"
      "	float scissor = scissorMask(fpos);\n"
      "#ifdef EDGE_AA\n"
      "	float strokeAlpha = strokeMask();\n"
      "	if (strokeAlpha < strokeThr) discard;\n"
      "#else\n"
      "	float strokeAlpha = 1.0;\n"
      "#endif\n"
      "	if (type == 0) {			// Gradient\n"
      "		// Calculate gradient color using box gradient\n"
      "		vec2 pt = (paintMat * vec3(fpos,1.0)).xy;\n"
      "		float d = clamp((sdroundrect(pt, extent, radius) + "
      "feather*0.5) / feather, 0.0, 1.0);\n"
      "		vec4 color = mix(innerCol,outerCol,d);\n"
      "		// Combine alpha\n"
      "		color *= strokeAlpha * scissor;\n"
      "		result = color;\n"
      "	} else if (type == 1) {		// Image\n"
      "		// Calculate color fron texture\n"
      "		vec2 pt = (paintMat * vec3(fpos,1.0)).xy / extent;\n"
      "#ifdef VG_GL3\n"
      "		vec4 color = texture(tex, pt);\n"
      "#else\n"
      "		vec4 color = texture2D(tex, pt);\n"
      "#endif\n"
      "		if (texType == 1) color = vec4(color.xyz*color.w,color.w);"
      "		if (texType == 2) color = vec4(color.x);"
      "		// Apply color tint and alpha.\n"
      "		color *= innerCol;\n"
      "		// Combine alpha\n"
      "		color *= strokeAlpha * scissor;\n"
      "		result = color;\n"
      "	} else if (type == 2) {		// Stencil fill\n"
      "		result = vec4(1,1,1,1);\n"
      "	} else if (type == 3) {		// Textured tris\n"
      "#ifdef VG_GL3\n"
      "		vec4 color = texture(tex, ftcoord);\n"
      "#else\n"
      "		vec4 color = texture2D(tex, ftcoord);\n"
      "#endif\n"
      "		if (texType == 1) color = vec4(color.xyz*color.w,color.w);"
      "		if (texType == 2) color = vec4(color.x);"
      "		color *= scissor;\n"
      "		result = color * innerCol;\n"
      "	}\n"
      "#ifdef VG_GL3\n"
      "	outColor = result;\n"
      "#else\n"
      "	gl_FragColor = result;\n"
      "#endif\n"
      "}\n";

  glvg__checkError(gl, "init");

  if (gl->flags & VG_ANTIALIAS) {
    if (glvg__createShader(&gl->shader, "shader", shaderHeader,
                           "#define EDGE_AA 1\n", fillVertShader,
                           fillFragShader) == 0)
      return 0;
  } else {
    if (glvg__createShader(&gl->shader, "shader", shaderHeader, NULL,
                           fillVertShader, fillFragShader) == 0)
      return 0;
  }

  glvg__checkError(gl, "uniform locations");
  glvg__getUniforms(&gl->shader);

  // Create dynamic vertex array
#if defined VG_GL3
  glGenVertexArrays(1, &gl->vertArr);
#endif
  glGenBuffers(1, &gl->vertBuf);

#if VG_GL_USE_UNIFORMBUFFER
  // Create UBOs
  glUniformBlockBinding(gl->shader.prog, gl->shader.loc[GLVG_LOC_FRAG],
                        GLVG_FRAG_BINDING);
  glGenBuffers(1, &gl->fragBuf);
  glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &align);
#endif
  gl->fragSize =
      sizeof(GLVGfragUniforms) + align - sizeof(GLVGfragUniforms) % align;

  // Some platforms does not allow to have samples to unset textures.
  // Create empty one which is bound when there's no texture specified.
  gl->dummyTex = glvg__renderCreateTexture(gl, VG_TEXTURE_ALPHA, 1, 1, 0, NULL);

  glvg__checkError(gl, "create done");

  glFinish();

  return 1;
}

static int glvg__renderCreateTexture(void *uptr, int type, int w, int h,
                                     int imageFlags,
                                     const unsigned char *data) {
  GLVGcontext *gl = (GLVGcontext *)uptr;
  GLVGtexture *tex = glvg__allocTexture(gl);

  if (tex == NULL)
    return 0;

#ifdef VG_GLES2
  // Check for non-power of 2.
  if (glvg__nearestPow2(w) != (unsigned int)w ||
      glvg__nearestPow2(h) != (unsigned int)h) {
    // No repeat
    if ((imageFlags & VG_IMAGE_REPEATX) != 0 ||
        (imageFlags & VG_IMAGE_REPEATY) != 0) {
      printf("Repeat X/Y is not supported for non power-of-two textures (%d x "
             "%d)\n",
             w, h);
      imageFlags &= ~(VG_IMAGE_REPEATX | VG_IMAGE_REPEATY);
    }
    // No mips.
    if (imageFlags & VG_IMAGE_GENERATE_MIPMAPS) {
      printf(
          "Mip-maps is not support for non power-of-two textures (%d x %d)\n",
          w, h);
      imageFlags &= ~VG_IMAGE_GENERATE_MIPMAPS;
    }
  }
#endif

  glGenTextures(1, &tex->tex);
  tex->width = w;
  tex->height = h;
  tex->type = type;
  tex->flags = imageFlags;
  glvg__bindTexture(gl, tex->tex);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#ifndef VG_GLES2
  glPixelStorei(GL_UNPACK_ROW_LENGTH, tex->width);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
#endif

#if defined(VG_GL2)
  // GL 1.4 and later has support for generating mipmaps using a tex parameter.
  if (imageFlags & VG_IMAGE_GENERATE_MIPMAPS) {
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
  }
#endif

  if (type == VG_TEXTURE_RGBA)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 data);
  else
#if defined(VG_GLES2) || defined(VG_GL2)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, w, h, 0, GL_LUMINANCE,
                 GL_UNSIGNED_BYTE, data);
#elif defined(VG_GLES3)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, w, h, 0, GL_RED, GL_UNSIGNED_BYTE,
                 data);
#else
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE,
                 data);
#endif

  if (imageFlags & VG_IMAGE_GENERATE_MIPMAPS) {
    if (imageFlags & VG_IMAGE_NEAREST) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                      GL_NEAREST_MIPMAP_NEAREST);
    } else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                      GL_LINEAR_MIPMAP_LINEAR);
    }
  } else {
    if (imageFlags & VG_IMAGE_NEAREST) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    } else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
  }

  if (imageFlags & VG_IMAGE_NEAREST) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  if (imageFlags & VG_IMAGE_REPEATX)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  else
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

  if (imageFlags & VG_IMAGE_REPEATY)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  else
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
#ifndef VG_GLES2
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
#endif

  // The new way to build mipmaps on GLES and GL3
#if !defined(VG_GL2)
  if (imageFlags & VG_IMAGE_GENERATE_MIPMAPS) {
    glGenerateMipmap(GL_TEXTURE_2D);
  }
#endif

  glvg__checkError(gl, "create tex");
  glvg__bindTexture(gl, 0);

  return tex->id;
}

static int glvg__renderDeleteTexture(void *uptr, int image) {
  GLVGcontext *gl = (GLVGcontext *)uptr;
  return glvg__deleteTexture(gl, image);
}

static int glvg__renderUpdateTexture(void *uptr, int image, int x, int y, int w,
                                     int h, const unsigned char *data) {
  GLVGcontext *gl = (GLVGcontext *)uptr;
  GLVGtexture *tex = glvg__findTexture(gl, image);

  if (tex == NULL)
    return 0;
  glvg__bindTexture(gl, tex->tex);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

#ifndef VG_GLES2
  glPixelStorei(GL_UNPACK_ROW_LENGTH, tex->width);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, x);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, y);
#else
  // No support for all of skip, need to update a whole row at a time.
  if (tex->type == VG_TEXTURE_RGBA)
    data += y * tex->width * 4;
  else
    data += y * tex->width;
  x = 0;
  w = tex->width;
#endif

  if (tex->type == VG_TEXTURE_RGBA)
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE,
                    data);
  else
#if defined(VG_GLES2) || defined(VG_GL2)
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_LUMINANCE,
                    GL_UNSIGNED_BYTE, data);
#else
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RED, GL_UNSIGNED_BYTE,
                    data);
#endif

  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
#ifndef VG_GLES2
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
#endif

  glvg__bindTexture(gl, 0);

  return 1;
}

static int glvg__renderGetTextureSize(void *uptr, int image, int *w, int *h) {
  GLVGcontext *gl = (GLVGcontext *)uptr;
  GLVGtexture *tex = glvg__findTexture(gl, image);
  if (tex == NULL)
    return 0;
  *w = tex->width;
  *h = tex->height;
  return 1;
}

static void glvg__xformToMat3x4(float *m3, float *t) {
  m3[0] = t[0];
  m3[1] = t[1];
  m3[2] = 0.0f;
  m3[3] = 0.0f;
  m3[4] = t[2];
  m3[5] = t[3];
  m3[6] = 0.0f;
  m3[7] = 0.0f;
  m3[8] = t[4];
  m3[9] = t[5];
  m3[10] = 1.0f;
  m3[11] = 0.0f;
}

static VGcolor glvg__premulColor(VGcolor c) {
  c.r *= c.a;
  c.g *= c.a;
  c.b *= c.a;
  return c;
}

static int glvg__convertPaint(GLVGcontext *gl, GLVGfragUniforms *frag,
                              VGpaint *paint, VGscissor *scissor, float width,
                              float fringe, float strokeThr) {
  GLVGtexture *tex = NULL;
  float invxform[6];

  memset(frag, 0, sizeof(*frag));

  frag->innerCol = glvg__premulColor(paint->innerColor);
  frag->outerCol = glvg__premulColor(paint->outerColor);

  if (scissor->extent[0] < -0.5f || scissor->extent[1] < -0.5f) {
    memset(frag->scissorMat, 0, sizeof(frag->scissorMat));
    frag->scissorExt[0] = 1.0f;
    frag->scissorExt[1] = 1.0f;
    frag->scissorScale[0] = 1.0f;
    frag->scissorScale[1] = 1.0f;
  } else {
    vgTransformInverse(invxform, scissor->xform);
    glvg__xformToMat3x4(frag->scissorMat, invxform);
    frag->scissorExt[0] = scissor->extent[0];
    frag->scissorExt[1] = scissor->extent[1];
    frag->scissorScale[0] = sqrtf(scissor->xform[0] * scissor->xform[0] +
                                  scissor->xform[2] * scissor->xform[2]) /
                            fringe;
    frag->scissorScale[1] = sqrtf(scissor->xform[1] * scissor->xform[1] +
                                  scissor->xform[3] * scissor->xform[3]) /
                            fringe;
  }

  memcpy(frag->extent, paint->extent, sizeof(frag->extent));
  frag->strokeMult = (width * 0.5f + fringe * 0.5f) / fringe;
  frag->strokeThr = strokeThr;

  if (paint->image != 0) {
    tex = glvg__findTexture(gl, paint->image);
    if (tex == NULL)
      return 0;
    if ((tex->flags & VG_IMAGE_FLIPY) != 0) {
      float m1[6], m2[6];
      vgTransformTranslate(m1, 0.0f, frag->extent[1] * 0.5f);
      vgTransformMultiply(m1, paint->xform);
      vgTransformScale(m2, 1.0f, -1.0f);
      vgTransformMultiply(m2, m1);
      vgTransformTranslate(m1, 0.0f, -frag->extent[1] * 0.5f);
      vgTransformMultiply(m1, m2);
      vgTransformInverse(invxform, m1);
    } else {
      vgTransformInverse(invxform, paint->xform);
    }
    frag->type = NSVG_SHADER_FILLIMG;

#if VG_GL_USE_UNIFORMBUFFER
    if (tex->type == VG_TEXTURE_RGBA)
      frag->texType = (tex->flags & VG_IMAGE_PREMULTIPLIED) ? 0 : 1;
    else
      frag->texType = 2;
#else
    if (tex->type == VG_TEXTURE_RGBA)
      frag->texType = (tex->flags & VG_IMAGE_PREMULTIPLIED) ? 0.0f : 1.0f;
    else
      frag->texType = 2.0f;
#endif
    //		printf("frag->texType = %d\n", frag->texType);
  } else {
    frag->type = NSVG_SHADER_FILLGRAD;
    frag->radius = paint->radius;
    frag->feather = paint->feather;
    vgTransformInverse(invxform, paint->xform);
  }

  glvg__xformToMat3x4(frag->paintMat, invxform);

  return 1;
}

static GLVGfragUniforms *vg__fragUniformPtr(GLVGcontext *gl, int i);

static void glvg__setUniforms(GLVGcontext *gl, int uniformOffset, int image) {
  GLVGtexture *tex = NULL;
#if VG_GL_USE_UNIFORMBUFFER
  glBindBufferRange(GL_UNIFORM_BUFFER, GLVG_FRAG_BINDING, gl->fragBuf,
                    uniformOffset, sizeof(GLVGfragUniforms));
#else
  GLVGfragUniforms *frag = vg__fragUniformPtr(gl, uniformOffset);
  glUniform4fv(gl->shader.loc[GLVG_LOC_FRAG], VG_GL_UNIFORMARRAY_SIZE,
               &(frag->uniformArray[0][0]));
#endif

  if (image != 0) {
    tex = glvg__findTexture(gl, image);
  }
  // If no image is set, use empty texture
  if (tex == NULL) {
    tex = glvg__findTexture(gl, gl->dummyTex);
  }
  glvg__bindTexture(gl, tex != NULL ? tex->tex : 0);
  glvg__checkError(gl, "tex paint tex");
}

static void glvg__renderViewport(void *uptr, float width, float height,
                                 float devicePixelRatio) {
  VG_NOTUSED(devicePixelRatio);
  GLVGcontext *gl = (GLVGcontext *)uptr;
  gl->view[0] = width;
  gl->view[1] = height;
}

static void glvg__fill(GLVGcontext *gl, GLVGcall *call) {
  GLVGpath *paths = &gl->paths[call->pathOffset];
  int i, npaths = call->pathCount;

  // Draw shapes
  glEnable(GL_STENCIL_TEST);
  glvg__stencilMask(gl, 0xff);
  glvg__stencilFunc(gl, GL_ALWAYS, 0, 0xff);
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

  // set bindpoint for solid loc
  glvg__setUniforms(gl, call->uniformOffset, 0);
  glvg__checkError(gl, "fill simple");

  glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
  glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR_WRAP);
  glDisable(GL_CULL_FACE);
  for (i = 0; i < npaths; i++)
    glDrawArrays(GL_TRIANGLE_FAN, paths[i].fillOffset, paths[i].fillCount);
  glEnable(GL_CULL_FACE);

  // Draw anti-aliased pixels
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

  glvg__setUniforms(gl, call->uniformOffset + gl->fragSize, call->image);
  glvg__checkError(gl, "fill fill");

  if (gl->flags & VG_ANTIALIAS) {
    glvg__stencilFunc(gl, GL_EQUAL, 0x00, 0xff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    // Draw fringes
    for (i = 0; i < npaths; i++)
      glDrawArrays(GL_TRIANGLE_STRIP, paths[i].strokeOffset,
                   paths[i].strokeCount);
  }

  // Draw fill
  glvg__stencilFunc(gl, GL_NOTEQUAL, 0x0, 0xff);
  glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
  glDrawArrays(GL_TRIANGLE_STRIP, call->triangleOffset, call->triangleCount);

  glDisable(GL_STENCIL_TEST);
}

static void glvg__convexFill(GLVGcontext *gl, GLVGcall *call) {
  GLVGpath *paths = &gl->paths[call->pathOffset];
  int i, npaths = call->pathCount;

  glvg__setUniforms(gl, call->uniformOffset, call->image);
  glvg__checkError(gl, "convex fill");

  for (i = 0; i < npaths; i++) {
    glDrawArrays(GL_TRIANGLE_FAN, paths[i].fillOffset, paths[i].fillCount);
    // Draw fringes
    if (paths[i].strokeCount > 0) {
      glDrawArrays(GL_TRIANGLE_STRIP, paths[i].strokeOffset,
                   paths[i].strokeCount);
    }
  }
}

static void glvg__stroke(GLVGcontext *gl, GLVGcall *call) {
  GLVGpath *paths = &gl->paths[call->pathOffset];
  int npaths = call->pathCount, i;

  if (gl->flags & VG_STENCIL_STROKES) {

    glEnable(GL_STENCIL_TEST);
    glvg__stencilMask(gl, 0xff);

    // Fill the stroke base without overlap
    glvg__stencilFunc(gl, GL_EQUAL, 0x0, 0xff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    glvg__setUniforms(gl, call->uniformOffset + gl->fragSize, call->image);
    glvg__checkError(gl, "stroke fill 0");
    for (i = 0; i < npaths; i++)
      glDrawArrays(GL_TRIANGLE_STRIP, paths[i].strokeOffset,
                   paths[i].strokeCount);

    // Draw anti-aliased pixels.
    glvg__setUniforms(gl, call->uniformOffset, call->image);
    glvg__stencilFunc(gl, GL_EQUAL, 0x00, 0xff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    for (i = 0; i < npaths; i++)
      glDrawArrays(GL_TRIANGLE_STRIP, paths[i].strokeOffset,
                   paths[i].strokeCount);

    // Clear stencil buffer.
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glvg__stencilFunc(gl, GL_ALWAYS, 0x0, 0xff);
    glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
    glvg__checkError(gl, "stroke fill 1");
    for (i = 0; i < npaths; i++)
      glDrawArrays(GL_TRIANGLE_STRIP, paths[i].strokeOffset,
                   paths[i].strokeCount);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    glDisable(GL_STENCIL_TEST);

    //		glvg__convertPaint(gl, vg__fragUniformPtr(gl,
    // call->uniformOffset + gl->fragSize), paint, scissor, strokeWidth,
    // fringe, 1.0f - 0.5f/255.0f);

  } else {
    glvg__setUniforms(gl, call->uniformOffset, call->image);
    glvg__checkError(gl, "stroke fill");
    // Draw Strokes
    for (i = 0; i < npaths; i++)
      glDrawArrays(GL_TRIANGLE_STRIP, paths[i].strokeOffset,
                   paths[i].strokeCount);
  }
}

static void glvg__triangles(GLVGcontext *gl, GLVGcall *call) {
  glvg__setUniforms(gl, call->uniformOffset, call->image);
  glvg__checkError(gl, "triangles fill");

  glDrawArrays(GL_TRIANGLES, call->triangleOffset, call->triangleCount);
}

static void glvg__renderCancel(void *uptr) {
  GLVGcontext *gl = (GLVGcontext *)uptr;
  gl->nverts = 0;
  gl->npaths = 0;
  gl->ncalls = 0;
  gl->nuniforms = 0;
}

static GLenum glvg_convertBlendFuncFactor(int factor) {
  if (factor == VG_ZERO)
    return GL_ZERO;
  if (factor == VG_ONE)
    return GL_ONE;
  if (factor == VG_SRC_COLOR)
    return GL_SRC_COLOR;
  if (factor == VG_ONE_MINUS_SRC_COLOR)
    return GL_ONE_MINUS_SRC_COLOR;
  if (factor == VG_DST_COLOR)
    return GL_DST_COLOR;
  if (factor == VG_ONE_MINUS_DST_COLOR)
    return GL_ONE_MINUS_DST_COLOR;
  if (factor == VG_SRC_ALPHA)
    return GL_SRC_ALPHA;
  if (factor == VG_ONE_MINUS_SRC_ALPHA)
    return GL_ONE_MINUS_SRC_ALPHA;
  if (factor == VG_DST_ALPHA)
    return GL_DST_ALPHA;
  if (factor == VG_ONE_MINUS_DST_ALPHA)
    return GL_ONE_MINUS_DST_ALPHA;
  if (factor == VG_SRC_ALPHA_SATURATE)
    return GL_SRC_ALPHA_SATURATE;
  return GL_INVALID_ENUM;
}

static GLVGblend glvg__blendCompositeOperation(VGcompositeOperationState op) {
  GLVGblend blend;
  blend.srcRGB = glvg_convertBlendFuncFactor(op.srcRGB);
  blend.dstRGB = glvg_convertBlendFuncFactor(op.dstRGB);
  blend.srcAlpha = glvg_convertBlendFuncFactor(op.srcAlpha);
  blend.dstAlpha = glvg_convertBlendFuncFactor(op.dstAlpha);
  if (blend.srcRGB == GL_INVALID_ENUM || blend.dstRGB == GL_INVALID_ENUM ||
      blend.srcAlpha == GL_INVALID_ENUM || blend.dstAlpha == GL_INVALID_ENUM) {
    blend.srcRGB = GL_ONE;
    blend.dstRGB = GL_ONE_MINUS_SRC_ALPHA;
    blend.srcAlpha = GL_ONE;
    blend.dstAlpha = GL_ONE_MINUS_SRC_ALPHA;
  }
  return blend;
}

static void glvg__renderFlush(void *uptr) {
  GLVGcontext *gl = (GLVGcontext *)uptr;
  int i;

  if (gl->ncalls > 0) {

    // Setup require GL state.
    glUseProgram(gl->shader.prog);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glStencilMask(0xffffffff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilFunc(GL_ALWAYS, 0, 0xffffffff);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
#if VG_GL_USE_STATE_FILTER
    gl->boundTexture = 0;
    gl->stencilMask = 0xffffffff;
    gl->stencilFunc = GL_ALWAYS;
    gl->stencilFuncRef = 0;
    gl->stencilFuncMask = 0xffffffff;
    gl->blendFunc.srcRGB = GL_INVALID_ENUM;
    gl->blendFunc.srcAlpha = GL_INVALID_ENUM;
    gl->blendFunc.dstRGB = GL_INVALID_ENUM;
    gl->blendFunc.dstAlpha = GL_INVALID_ENUM;
#endif

#if VG_GL_USE_UNIFORMBUFFER
    // Upload ubo for frag shaders
    glBindBuffer(GL_UNIFORM_BUFFER, gl->fragBuf);
    glBufferData(GL_UNIFORM_BUFFER, gl->nuniforms * gl->fragSize, gl->uniforms,
                 GL_STREAM_DRAW);
#endif

    // Upload vertex data
#if defined VG_GL3
    glBindVertexArray(gl->vertArr);
#endif
    glBindBuffer(GL_ARRAY_BUFFER, gl->vertBuf);
    glBufferData(GL_ARRAY_BUFFER, gl->nverts * sizeof(VGvertex), gl->verts,
                 GL_STREAM_DRAW);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VGvertex),
                          (const GLvoid *)(size_t)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VGvertex),
                          (const GLvoid *)(0 + 2 * sizeof(float)));

    // Set view and texture just once per frame.
    glUniform1i(gl->shader.loc[GLVG_LOC_TEX], 0);
    glUniform2fv(gl->shader.loc[GLVG_LOC_VIEWSIZE], 1, gl->view);

#if VG_GL_USE_UNIFORMBUFFER
    glBindBuffer(GL_UNIFORM_BUFFER, gl->fragBuf);
#endif

    for (i = 0; i < gl->ncalls; i++) {
      GLVGcall *call = &gl->calls[i];
      glvg__blendFuncSeparate(gl, &call->blendFunc);
      if (call->type == GLVG_FILL)
        glvg__fill(gl, call);
      else if (call->type == GLVG_CONVEXFILL)
        glvg__convexFill(gl, call);
      else if (call->type == GLVG_STROKE)
        glvg__stroke(gl, call);
      else if (call->type == GLVG_TRIANGLES)
        glvg__triangles(gl, call);
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
#if defined VG_GL3
    glBindVertexArray(0);
#endif
    glDisable(GL_CULL_FACE);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
    glvg__bindTexture(gl, 0);
  }

  // Reset calls
  gl->nverts = 0;
  gl->npaths = 0;
  gl->ncalls = 0;
  gl->nuniforms = 0;
}

static int glvg__maxVertCount(const VGpath *paths, int npaths) {
  int i, count = 0;
  for (i = 0; i < npaths; i++) {
    count += paths[i].nfill;
    count += paths[i].nstroke;
  }
  return count;
}

static GLVGcall *glvg__allocCall(GLVGcontext *gl) {
  GLVGcall *ret = NULL;
  if (gl->ncalls + 1 > gl->ccalls) {
    GLVGcall *calls;
    int ccalls =
        glvg__maxi(gl->ncalls + 1, 128) + gl->ccalls / 2; // 1.5x Overallocate
    calls = (GLVGcall *)realloc(gl->calls, sizeof(GLVGcall) * ccalls);
    if (calls == NULL)
      return NULL;
    gl->calls = calls;
    gl->ccalls = ccalls;
  }
  ret = &gl->calls[gl->ncalls++];
  memset(ret, 0, sizeof(GLVGcall));
  return ret;
}

static int glvg__allocPaths(GLVGcontext *gl, int n) {
  int ret = 0;
  if (gl->npaths + n > gl->cpaths) {
    GLVGpath *paths;
    int cpaths =
        glvg__maxi(gl->npaths + n, 128) + gl->cpaths / 2; // 1.5x Overallocate
    paths = (GLVGpath *)realloc(gl->paths, sizeof(GLVGpath) * cpaths);
    if (paths == NULL)
      return -1;
    gl->paths = paths;
    gl->cpaths = cpaths;
  }
  ret = gl->npaths;
  gl->npaths += n;
  return ret;
}

static int glvg__allocVerts(GLVGcontext *gl, int n) {
  int ret = 0;
  if (gl->nverts + n > gl->cverts) {
    VGvertex *verts;
    int cverts =
        glvg__maxi(gl->nverts + n, 4096) + gl->cverts / 2; // 1.5x Overallocate
    verts = (VGvertex *)realloc(gl->verts, sizeof(VGvertex) * cverts);
    if (verts == NULL)
      return -1;
    gl->verts = verts;
    gl->cverts = cverts;
  }
  ret = gl->nverts;
  gl->nverts += n;
  return ret;
}

static int glvg__allocFragUniforms(GLVGcontext *gl, int n) {
  int ret = 0, structSize = gl->fragSize;
  if (gl->nuniforms + n > gl->cuniforms) {
    unsigned char *uniforms;
    int cuniforms = glvg__maxi(gl->nuniforms + n, 128) +
                    gl->cuniforms / 2; // 1.5x Overallocate
    uniforms = (unsigned char *)realloc(gl->uniforms, structSize * cuniforms);
    if (uniforms == NULL)
      return -1;
    gl->uniforms = uniforms;
    gl->cuniforms = cuniforms;
  }
  ret = gl->nuniforms * structSize;
  gl->nuniforms += n;
  return ret;
}

static GLVGfragUniforms *vg__fragUniformPtr(GLVGcontext *gl, int i) {
  return (GLVGfragUniforms *)&gl->uniforms[i];
}

static void glvg__vset(VGvertex *vtx, float x, float y, float u, float v) {
  vtx->x = x;
  vtx->y = y;
  vtx->u = u;
  vtx->v = v;
}

static void glvg__renderFill(void *uptr, VGpaint *paint,
                             VGcompositeOperationState compositeOperation,
                             VGscissor *scissor, float fringe,
                             const float *bounds, const VGpath *paths,
                             int npaths) {
  GLVGcontext *gl = (GLVGcontext *)uptr;
  GLVGcall *call = glvg__allocCall(gl);
  VGvertex *quad;
  GLVGfragUniforms *frag;
  int i, maxverts, offset;

  if (call == NULL)
    return;

  call->type = GLVG_FILL;
  call->triangleCount = 4;
  call->pathOffset = glvg__allocPaths(gl, npaths);
  if (call->pathOffset == -1)
    goto error;
  call->pathCount = npaths;
  call->image = paint->image;
  call->blendFunc = glvg__blendCompositeOperation(compositeOperation);

  if (npaths == 1 && paths[0].convex) {
    call->type = GLVG_CONVEXFILL;
    call->triangleCount =
        0; // Bounding box fill quad not needed for convex fill
  }

  // Allocate vertices for all the paths.
  maxverts = glvg__maxVertCount(paths, npaths) + call->triangleCount;
  offset = glvg__allocVerts(gl, maxverts);
  if (offset == -1)
    goto error;

  for (i = 0; i < npaths; i++) {
    GLVGpath *copy = &gl->paths[call->pathOffset + i];
    const VGpath *path = &paths[i];
    memset(copy, 0, sizeof(GLVGpath));
    if (path->nfill > 0) {
      copy->fillOffset = offset;
      copy->fillCount = path->nfill;
      memcpy(&gl->verts[offset], path->fill, sizeof(VGvertex) * path->nfill);
      offset += path->nfill;
    }
    if (path->nstroke > 0) {
      copy->strokeOffset = offset;
      copy->strokeCount = path->nstroke;
      memcpy(&gl->verts[offset], path->stroke,
             sizeof(VGvertex) * path->nstroke);
      offset += path->nstroke;
    }
  }

  // Setup uniforms for draw calls
  if (call->type == GLVG_FILL) {
    // Quad
    call->triangleOffset = offset;
    quad = &gl->verts[call->triangleOffset];
    glvg__vset(&quad[0], bounds[2], bounds[3], 0.5f, 1.0f);
    glvg__vset(&quad[1], bounds[2], bounds[1], 0.5f, 1.0f);
    glvg__vset(&quad[2], bounds[0], bounds[3], 0.5f, 1.0f);
    glvg__vset(&quad[3], bounds[0], bounds[1], 0.5f, 1.0f);

    call->uniformOffset = glvg__allocFragUniforms(gl, 2);
    if (call->uniformOffset == -1)
      goto error;
    // Simple shader for stencil
    frag = vg__fragUniformPtr(gl, call->uniformOffset);
    memset(frag, 0, sizeof(*frag));
    frag->strokeThr = -1.0f;
    frag->type = NSVG_SHADER_SIMPLE;
    // Fill shader
    glvg__convertPaint(
        gl, vg__fragUniformPtr(gl, call->uniformOffset + gl->fragSize), paint,
        scissor, fringe, fringe, -1.0f);
  } else {
    call->uniformOffset = glvg__allocFragUniforms(gl, 1);
    if (call->uniformOffset == -1)
      goto error;
    // Fill shader
    glvg__convertPaint(gl, vg__fragUniformPtr(gl, call->uniformOffset), paint,
                       scissor, fringe, fringe, -1.0f);
  }

  return;

error:
  // We get here if call alloc was ok, but something else is not.
  // Roll back the last call to prevent drawing it.
  if (gl->ncalls > 0)
    gl->ncalls--;
}

static void glvg__renderStroke(void *uptr, VGpaint *paint,
                               VGcompositeOperationState compositeOperation,
                               VGscissor *scissor, float fringe,
                               float strokeWidth, const VGpath *paths,
                               int npaths) {
  GLVGcontext *gl = (GLVGcontext *)uptr;
  GLVGcall *call = glvg__allocCall(gl);
  int i, maxverts, offset;

  if (call == NULL)
    return;

  call->type = GLVG_STROKE;
  call->pathOffset = glvg__allocPaths(gl, npaths);
  if (call->pathOffset == -1)
    goto error;
  call->pathCount = npaths;
  call->image = paint->image;
  call->blendFunc = glvg__blendCompositeOperation(compositeOperation);

  // Allocate vertices for all the paths.
  maxverts = glvg__maxVertCount(paths, npaths);
  offset = glvg__allocVerts(gl, maxverts);
  if (offset == -1)
    goto error;

  for (i = 0; i < npaths; i++) {
    GLVGpath *copy = &gl->paths[call->pathOffset + i];
    const VGpath *path = &paths[i];
    memset(copy, 0, sizeof(GLVGpath));
    if (path->nstroke) {
      copy->strokeOffset = offset;
      copy->strokeCount = path->nstroke;
      memcpy(&gl->verts[offset], path->stroke,
             sizeof(VGvertex) * path->nstroke);
      offset += path->nstroke;
    }
  }

  if (gl->flags & VG_STENCIL_STROKES) {
    // Fill shader
    call->uniformOffset = glvg__allocFragUniforms(gl, 2);
    if (call->uniformOffset == -1)
      goto error;

    glvg__convertPaint(gl, vg__fragUniformPtr(gl, call->uniformOffset), paint,
                       scissor, strokeWidth, fringe, -1.0f);
    glvg__convertPaint(
        gl, vg__fragUniformPtr(gl, call->uniformOffset + gl->fragSize), paint,
        scissor, strokeWidth, fringe, 1.0f - 0.5f / 255.0f);

  } else {
    // Fill shader
    call->uniformOffset = glvg__allocFragUniforms(gl, 1);
    if (call->uniformOffset == -1)
      goto error;
    glvg__convertPaint(gl, vg__fragUniformPtr(gl, call->uniformOffset), paint,
                       scissor, strokeWidth, fringe, -1.0f);
  }

  return;

error:
  // We get here if call alloc was ok, but something else is not.
  // Roll back the last call to prevent drawing it.
  if (gl->ncalls > 0)
    gl->ncalls--;
}

static void glvg__renderTriangles(void *uptr, VGpaint *paint,
                                  VGcompositeOperationState compositeOperation,
                                  VGscissor *scissor, const VGvertex *verts,
                                  int nverts, float fringe) {
  GLVGcontext *gl = (GLVGcontext *)uptr;
  GLVGcall *call = glvg__allocCall(gl);
  GLVGfragUniforms *frag;

  if (call == NULL)
    return;

  call->type = GLVG_TRIANGLES;
  call->image = paint->image;
  call->blendFunc = glvg__blendCompositeOperation(compositeOperation);

  // Allocate vertices for all the paths.
  call->triangleOffset = glvg__allocVerts(gl, nverts);
  if (call->triangleOffset == -1)
    goto error;
  call->triangleCount = nverts;

  memcpy(&gl->verts[call->triangleOffset], verts, sizeof(VGvertex) * nverts);

  // Fill shader
  call->uniformOffset = glvg__allocFragUniforms(gl, 1);
  if (call->uniformOffset == -1)
    goto error;
  frag = vg__fragUniformPtr(gl, call->uniformOffset);
  glvg__convertPaint(gl, frag, paint, scissor, 1.0f, fringe, -1.0f);
  frag->type = NSVG_SHADER_IMG;

  return;

error:
  // We get here if call alloc was ok, but something else is not.
  // Roll back the last call to prevent drawing it.
  if (gl->ncalls > 0)
    gl->ncalls--;
}

static void glvg__renderDelete(void *uptr) {
  GLVGcontext *gl = (GLVGcontext *)uptr;
  int i;
  if (gl == NULL)
    return;

  glvg__deleteShader(&gl->shader);

#if VG_GL3
#if VG_GL_USE_UNIFORMBUFFER
  if (gl->fragBuf != 0)
    glDeleteBuffers(1, &gl->fragBuf);
#endif
  if (gl->vertArr != 0)
    glDeleteVertexArrays(1, &gl->vertArr);
#endif
  if (gl->vertBuf != 0)
    glDeleteBuffers(1, &gl->vertBuf);

  for (i = 0; i < gl->ntextures; i++) {
    if (gl->textures[i].tex != 0 &&
        (gl->textures[i].flags & VG_IMAGE_NODELETE) == 0)
      glDeleteTextures(1, &gl->textures[i].tex);
  }
  free(gl->textures);

  free(gl->paths);
  free(gl->verts);
  free(gl->uniforms);
  free(gl->calls);

  free(gl);
}

#if defined VG_GL2
VGcontext *vgCreateGL2(int flags)
#elif defined VG_GL3
VGcontext *vgCreateGL3(int flags)
#elif defined VG_GLES2
VGcontext *vgCreateGLES2(int flags)
#elif defined VG_GLES3
VGcontext *vgCreateGLES3(int flags)
#endif
{
  VGparams params;
  VGcontext *ctx = NULL;
  GLVGcontext *gl = (GLVGcontext *)malloc(sizeof(GLVGcontext));
  if (gl == NULL)
    goto error;
  memset(gl, 0, sizeof(GLVGcontext));

  memset(&params, 0, sizeof(params));
  params.renderCreate = glvg__renderCreate;
  params.renderCreateTexture = glvg__renderCreateTexture;
  params.renderDeleteTexture = glvg__renderDeleteTexture;
  params.renderUpdateTexture = glvg__renderUpdateTexture;
  params.renderGetTextureSize = glvg__renderGetTextureSize;
  params.renderViewport = glvg__renderViewport;
  params.renderCancel = glvg__renderCancel;
  params.renderFlush = glvg__renderFlush;
  params.renderFill = glvg__renderFill;
  params.renderStroke = glvg__renderStroke;
  params.renderTriangles = glvg__renderTriangles;
  params.renderDelete = glvg__renderDelete;
  params.userPtr = gl;
  params.edgeAntiAlias = flags & VG_ANTIALIAS ? 1 : 0;

  gl->flags = flags;

  ctx = vgCreateInternal(&params);
  if (ctx == NULL)
    goto error;

  return ctx;

error:
  // 'gl' is freed by vgDeleteInternal.
  if (ctx != NULL)
    vgDeleteInternal(ctx);
  return NULL;
}

#if defined VG_GL2
void vgDeleteGL2(VGcontext *ctx)
#elif defined VG_GL3
void vgDeleteGL3(VGcontext *ctx)
#elif defined VG_GLES2
void vgDeleteGLES2(VGcontext *ctx)
#elif defined VG_GLES3
void vgDeleteGLES3(VGcontext *ctx)
#endif
{
  vgDeleteInternal(ctx);
}

#if defined VG_GL2
int vglCreateImageFromHandleGL2(VGcontext *ctx, GLuint textureId, int w, int h,
                                int imageFlags)
#elif defined VG_GL3
int vglCreateImageFromHandleGL3(VGcontext *ctx, GLuint textureId, int w, int h,
                                int imageFlags)
#elif defined VG_GLES2
int vglCreateImageFromHandleGLES2(VGcontext *ctx, GLuint textureId, int w,
                                  int h, int imageFlags)
#elif defined VG_GLES3
int vglCreateImageFromHandleGLES3(VGcontext *ctx, GLuint textureId, int w,
                                  int h, int imageFlags)
#endif
{
  GLVGcontext *gl = (GLVGcontext *)vgInternalParams(ctx)->userPtr;
  GLVGtexture *tex = glvg__allocTexture(gl);

  if (tex == NULL)
    return 0;

  tex->type = VG_TEXTURE_RGBA;
  tex->tex = textureId;
  tex->flags = imageFlags;
  tex->width = w;
  tex->height = h;

  return tex->id;
}

#if defined VG_GL2
GLuint vglImageHandleGL2(VGcontext *ctx, int image)
#elif defined VG_GL3
GLuint vglImageHandleGL3(VGcontext *ctx, int image)
#elif defined VG_GLES2
GLuint vglImageHandleGLES2(VGcontext *ctx, int image)
#elif defined VG_GLES3
GLuint vglImageHandleGLES3(VGcontext *ctx, int image)
#endif
{
  GLVGcontext *gl = (GLVGcontext *)vgInternalParams(ctx)->userPtr;
  GLVGtexture *tex = glvg__findTexture(gl, image);
  return tex->tex;
}

#endif /* VG_GL_IMPLEMENTATION */
