#include "uniform_map.h"

unsigned int hash(const char *str) {
  unsigned int hash = 0;
  while (*str) {
    hash = (hash << 5) + *str++;
  }
  return hash % HASHMAP_SIZE;
}

UniformHashMap *uniform_create_hashmap() {
  UniformHashMap *map = malloc(sizeof(UniformHashMap));
  map->table = malloc(sizeof(UniformNode *) * HASHMAP_SIZE);
  for (int i = 0; i < HASHMAP_SIZE; i++) {
    map->table[i] = NULL;
  }
  return map;
}

void uniform_hashmap_add(UniformHashMap *map, const char *name, int location) {
  unsigned int index = hash(name);
  UniformNode *newNode = malloc(sizeof(UniformNode));
  newNode->name = strdup(name);
  newNode->location = location;
  newNode->next = map->table[index];
  map->table[index] = newNode;
}

int uniform_hashmap_get(UniformHashMap *map, const char *name) {
  unsigned int index = hash(name);
  UniformNode *node = map->table[index];
  while (node) {
    if (strcmp(node->name, name) == 0) {
      return node->location;
    }
    node = node->next;
  }
  return -1;
}

void uniform_hashmap_destroy(UniformHashMap *map) {
  for (int i = 0; i < HASHMAP_SIZE; i++) {
    UniformNode *node = map->table[i];
    while (node) {
      UniformNode *temp = node;
      node = node->next;
      free(temp->name);
      free(temp);
    }
  }
  free(map->table);
  free(map);
}
