#include "kernel_event.h"
#include "event.h"

#include <stddef.h>
#include <string.h>
#include "PL050.h"

#define MAX_EVENT_BACKLOG 16

bool event_loop_open = false;
size_t eb_read = 0, eb_write = 0;
Event event_backlog[ MAX_EVENT_BACKLOG ];

// unimplemented
void on_kernel_event_keyboard() {
  // clears bytes
  for ( size_t i = 0; i < 3; ++i ) {
    PL050_getc( PS20 );
  }
}

void on_kernel_event_mouse() {
  if ( !event_loop_open ) return;

  size_t next_write = ( eb_write + 1 ) % MAX_EVENT_BACKLOG;
  if ( next_write == eb_read ) return;

  uint8_t type = PL050_getc( PS21 );
  switch ( type ) {

    // Mouse move
    case 0x00: {
      event_backlog[eb_write].type = event_type_mouse_move;
      uint8_t x;
      x = PL050_getc( PS21 );
      event_backlog[eb_write].mouse_move.dx  = *(int8_t*)(&x) << 8;
      x = PL050_getc( PS21 );
      event_backlog[eb_write].mouse_move.dx |= *(int8_t*)(&x);
      x = PL050_getc( PS21 );
      event_backlog[eb_write].mouse_move.dy  = *(int8_t*)(&x) << 8;
      x = PL050_getc( PS21 );
      event_backlog[eb_write].mouse_move.dy |= *(int8_t*)(&x);
    }

    // Mouse button released
    case 0x08: {
      event_backlog[eb_write].type = event_type_mouse_button_release;
      PL050_getc( PS21 ); // clear remaining bytes
      PL050_getc( PS21 );
      break;
    }

    // Mouse button pressed ( left )
    case 0x09: {
      event_backlog[eb_write].type = event_type_mouse_button_press;
      event_backlog[eb_write].mouse_button_press.button = ButtonLeft;
      PL050_getc( PS21 ); // clear remaining bytes
      PL050_getc( PS21 );
      break;
    }

    // Mouse button pressed ( right )
    case 0x0A: {
      event_backlog[eb_write].type = event_type_mouse_button_press;
      event_backlog[eb_write].mouse_button_press.button = ButtonRight;
      PL050_getc( PS21 ); // clear remaining bytes
      PL050_getc( PS21 );
      break;
    }

    // Mouse button pressed ( middle )
    case 0x0C: {
      event_backlog[eb_write].type = event_type_mouse_button_press;
      event_backlog[eb_write].mouse_button_press.button = ButtonMiddle;
      PL050_getc( PS21 ); // clear remaining bytes
      PL050_getc( PS21 );
      break;
    }

    default: {
      PL050_getc( PS21 ); // clear remaining bytes
      PL050_getc( PS21 );
      return;
    }
  }

  eb_write = next_write;
}


bool open_event_loop() {
  if ( event_loop_open ) return false;

  eb_read = 0;
  eb_write = 0;
  event_loop_open = true;
  return true;
}

void close_event_loop() {
  event_loop_open = false;
}

bool poll_event( Event *event ) {
  if ( !event_loop_open ) return false;
  if ( eb_read == eb_write ) return false;

  memcpy( event, &event_backlog[eb_read], sizeof( Event ) );
  eb_read = ( eb_read + 1 ) % MAX_EVENT_BACKLOG;
  return true;
}
