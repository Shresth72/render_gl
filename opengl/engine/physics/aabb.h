#pragma once

typedef struct {
  float x;
  float y;
  float width;
  float height;
} AABB; // Axis-Aligned Bounding Box

int check_collision(AABB *a, AABB *b);
int check_aabb_collision(float x, float y, AABB *other);
