#include "libc.h"
#include "display.h"
#include "event.h"

void main_mouse() {
  if ( !open_event_loop() ) exit( 1 );
  display_t *display = get_display();

  size_t  x = DISPLAY_WIDTH  / 2;
  size_t  y = DISPLAY_HEIGHT / 2;
  Event event;

  uint16_t colour = 0b0111111111111111;

  while ( 1 ) {
    while ( poll_event( &event ) ) {
      if ( event.type == event_type_mouse_button_press ) {
        switch ( event.mouse_press.button ) {
          case ButtonLeft:
            colour = 0b0100001000011111;
            break;
          case ButtonMiddle:
            colour = 0b0100001111110000;
            break;
          case ButtonRight:
            colour = 0b0111111000010000;
            break;
        }
      }
    }

    for ( size_t j = 0; j < DISPLAY_HEIGHT; ++j ) {
      for ( size_t i = 0; i < DISPLAY_WIDTH; ++i ) {
        display->buffer[ j ][ i ] = colour;
      }
    }
    display->buffer[ y ][ x ] = 0b0000000000000000;

    display = flip_display();
  }
}
