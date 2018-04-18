#include "libc.h"

void main_pipe_client() {
  int fd = open( "hello_world" );

  char buff[5];
  while ( 1 ) {
    size_t n = read( fd, buff, 5 );
    write( 1, buff, n );
    yield();
  }

  close( fd );
  unlink( "hello_world" );

  exit( 0 );
}
