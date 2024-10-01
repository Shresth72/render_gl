#pragma once
#include "../../utils/errors.h"

typedef struct {
  unsigned char *id;

  unsigned char *leader;
  unsigned char **playerIds;
  unsigned int playerCount;
} Room;
