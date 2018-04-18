#include "libc.h"

char xtoa( int x ) {
  if ( 0 <= x && x < 10 ) {
    return '0' + x;
  }
  if ( 10 <= x && x < 16 ) {
    return 'a' + x - 10;
  }
  return '?';
}

void main_phil() {
  int i;
  for ( i = 0; i < 16; ++i ) {
    if ( fork() == 0 ) {
      break;
    }
  }

  char c = xtoa( i );
  write( 1, &c, 1 );
  exit( 0 );
}
