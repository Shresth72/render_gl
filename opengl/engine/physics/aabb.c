#include "aabb.h"

int check_collision(AABB *a, AABB *b) {
  return (a->x < b->x + b->width && a->x + a->width > b->x &&
          a->y < b->y + b->height && a->y + a->height > b->y);
}

int check_aabb_collision(float x, float y, AABB *other) {
  return (x + 0.125f > other->x - other->width && // Right side of tank
          x - 0.125f < other->x + other->width && // Left side of tank
          y + 0.18f > other->y - other->height && // Top side of tank
          y - 0.18f < other->y + other->height);  // Bottom side of tank
}
