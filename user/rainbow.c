#include "libc.h"
#include "display.h"

uint16_t colours[7] = {
  0b0111110000000000,
  0b0111110111100000,
  0b0111111111100000,
  0b0000001111100000,
  0b0000000111111111,
  0b0011110000011111,
  0b0111110000011111
};

void main_rainbow() {
  display_t *display = get_display();

  size_t i = 0;
  while ( 1 ) {
    for ( size_t y = 0; y < DISPLAY_HEIGHT; ++y ) {
      for ( size_t x = 0; x < DISPLAY_WIDTH; ++x ) {
        display->buffer[ y ][ x ] = colours[i];
      }
    }
    i = ( i + 1 ) % 7;
    display = flip_display();
    yield();
  }
}
