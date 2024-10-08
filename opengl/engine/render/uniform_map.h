#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASHMAP_SIZE 128

typedef struct UniformNode {
  char *name;
  int location;
  struct UniformNode *next;
} UniformNode;

typedef struct {
  UniformNode **table;
} UniformHashMap;

unsigned int uniform_hash(const char *str);
UniformHashMap *uniform_create_hashmap();
void uniform_hashmap_add(UniformHashMap *map, const char *name, int location);
int uniform_hashmap_get(UniformHashMap *map, const char *name);
void uniform_hashmap_destroy(UniformHashMap *map);
