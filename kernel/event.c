#include "kernel_event.h"
#include "event.h"

#include <stddef.h>
#include <string.h>
#include "PL050.h"

char xtoa( int x );
#define MAX_EVENT_BACKLOG 16

bool event_loop_open = false;
size_t eb_read = 0, eb_write = 0;
Event event_backlog[ MAX_EVENT_BACKLOG ];

uint8_t button_state = 0b000;

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

  uint8_t byte = PL050_getc( PS21 );
  uint8_t code = ( byte >> 3 ) & 0b00011111;

  if ( code == 0b00001 ) {
    uint8_t button_state_new = byte & 0b00000111;
    if ( button_state_new < button_state ) {
      event_backlog[ eb_write ].type = event_type_mouse_button_release;
      event_backlog[ eb_write ].mouse_release.button = button_state - button_state_new;
    } else {
      event_backlog[ eb_write ].type = event_type_mouse_button_press;
      event_backlog[ eb_write ].mouse_release.button = button_state_new - button_state;
    }
    PL050_getc( PS21 ); PL050_getc( PS21 ); // clear extra bytes
    eb_write = next_write;
  } else {
    char msg[ 11 ] = "?: ______\n";
    msg[ 3 ] = xtoa( ( byte >> 4 ) & 0xf );
    msg[ 4 ] = xtoa( ( byte >> 0 ) & 0xf );
    byte = PL050_getc( PS21 );
    msg[ 5 ] = xtoa( ( byte >> 4 ) & 0xf );
    msg[ 6 ] = xtoa( ( byte >> 0 ) & 0xf );
    byte = PL050_getc( PS21 );
    msg[ 7 ] = xtoa( ( byte >> 4 ) & 0xf );
    msg[ 8 ] = xtoa( ( byte >> 0 ) & 0xf );
    write( 1, msg, 10 );
  }
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
