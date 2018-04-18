#include "libc.h"

void main_pipe_server() {
  mkfifo( "hello_world" );
  int fd = open( "hello_world" );

  char msg[15] = "Hello, world!\n";
  size_t i = 0;
  while ( 1 ) {
    i += write( fd, &msg[i], ( 14 - i ) );
    i %= 14;
    yield();
  }

  close( fd );
  exit( 0 );
}
