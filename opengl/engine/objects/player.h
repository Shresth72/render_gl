#pragma once
#include "../../utils/errors.h"

typedef struct {
  unsigned char *roomId;
  unsigned char *id;

  unsigned int isLeader;
  unsigned char *card;

  unsigned int isLying;
  unsigned char statement;
} Player;
