#ifndef VG_GL_UTILS_H
#define VG_GL_UTILS_H

#include "vg.h"
#include <GL/glew.h>

struct VGLUframebuffer {
  VGcontext *ctx;
  GLuint fbo;
  GLuint rbo;
  GLuint texture;
  int image;
};
typedef struct VGLUframebuffer VGLUframebuffer;

// Helper function to create GL frame buffer to render to.
void vgluBindFramebuffer(VGLUframebuffer *fb);
VGLUframebuffer *vgluCreateFramebuffer(VGcontext *ctx, int w, int h,
                                       int imageFlags);
void vgluDeleteFramebuffer(VGLUframebuffer *fb);

#endif // VG_GL_UTILS_H

#ifdef VG_GL_IMPLEMENTATION

#if defined(VG_GL3) || defined(VG_GLES2) || defined(VG_GLES3)
// FBO is core in OpenGL 3>.
#define VG_FBO_VALID 1
#elif defined(VG_GL2)
// On OS X including glext defines FBO on GL2 too.
#ifdef __APPLE__
#include <OpenGL/glext.h>
#define VG_FBO_VALID 1
#endif
#endif

static GLint defaultFBO = -1;

VGLUframebuffer *vgluCreateFramebuffer(VGcontext *ctx, int w, int h,
                                       int imageFlags) {
#ifdef VG_FBO_VALID
  GLint defaultFBO;
  GLint defaultRBO;
  VGLUframebuffer *fb = NULL;

  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBO);
  glGetIntegerv(GL_RENDERBUFFER_BINDING, &defaultRBO);

  fb = (VGLUframebuffer *)malloc(sizeof(VGLUframebuffer));
  if (fb == NULL)
    goto error;
  memset(fb, 0, sizeof(VGLUframebuffer));

  fb->image = vgCreateImageRGBA(
      ctx, w, h, imageFlags | VG_IMAGE_FLIPY | VG_IMAGE_PREMULTIPLIED, NULL);

#if defined VG_GL2
  fb->texture = vglImageHandleGL2(ctx, fb->image);
#elif defined VG_GL3
  fb->texture = vglImageHandleGL3(ctx, fb->image);
#elif defined VG_GLES2
  fb->texture = vglImageHandleGLES2(ctx, fb->image);
#elif defined VG_GLES3
  fb->texture = vglImageHandleGLES3(ctx, fb->image);
#endif

  fb->ctx = ctx;

  // frame buffer object
  glGenFramebuffers(1, &fb->fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fb->fbo);

  // render buffer object
  glGenRenderbuffers(1, &fb->rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, fb->rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, w, h);

  // combine all
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         fb->texture, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, fb->rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
#ifdef GL_DEPTH24_STENCIL8
    // If GL_STENCIL_INDEX8 is not supported, try GL_DEPTH24_STENCIL8 as a
    // fallback. Some graphics cards require a depth buffer along with a
    // stencil.
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           fb->texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, fb->rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
#endif // GL_DEPTH24_STENCIL8
      goto error;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, defaultRBO);
  return fb;
error:
  glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, defaultRBO);
  vgluDeleteFramebuffer(fb);
  return NULL;
#else
  VG_NOTUSED(ctx);
  VG_NOTUSED(w);
  VG_NOTUSED(h);
  VG_NOTUSED(imageFlags);
  return NULL;
#endif
}

void vgluBindFramebuffer(VGLUframebuffer *fb) {
#ifdef VG_FBO_VALID
  if (defaultFBO == -1)
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, fb != NULL ? fb->fbo : defaultFBO);
#else
  VG_NOTUSED(fb);
#endif
}

void vgluDeleteFramebuffer(VGLUframebuffer *fb) {
#ifdef VG_FBO_VALID
  if (fb == NULL)
    return;
  if (fb->fbo != 0)
    glDeleteFramebuffers(1, &fb->fbo);
  if (fb->rbo != 0)
    glDeleteRenderbuffers(1, &fb->rbo);
  if (fb->image >= 0)
    vgDeleteImage(fb->ctx, fb->image);
  fb->ctx = NULL;
  fb->fbo = 0;
  fb->rbo = 0;
  fb->texture = 0;
  fb->image = -1;
  free(fb);
#else
  VG_NOTUSED(fb);
#endif
}

#endif // VG_GL_IMPLEMENTATION
