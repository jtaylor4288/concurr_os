#include "libc.h"

void main_pipe_server() {
  mkfifo( "hello_world" );
  int fd = open( "hello_world" );
  write( fd, "Hello, world!\n", 15 );
  close( fd );
  exit( 0 );
}
