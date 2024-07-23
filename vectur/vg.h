#ifndef VG_H
#define VG_H

#ifdef __cplusplus
extern "C" {
#endif

#define VG_PI 3.14159265358979323846264338327f

#ifdef _MSC_VER_
#pragma warning(push)
#pragma warning(disable : 4201)
#endif // _MSC_VER_

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
};

#ifdef __cplusplus
}
#endif

#endif // !VG_H
