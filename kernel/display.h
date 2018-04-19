#ifndef __DISPLAY_H
#define __DISPLAY_H

#include   "GIC.h"
#include "PL050.h"
#include "PL111.h"
#include   "SYS.h"

#define DISPLAY_WIDTH  800
#define DISPLAY_HEIGHT 600

typedef struct {
  uint16_t buffer[DISPLAY_HEIGHT][DISPLAY_WIDTH];
} display_t;

void init_display();

display_t* get_display();
display_t* flip_display();

#endif
