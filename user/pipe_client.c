#include "libc.h"

void main_pipe_client() {
  int fd = open( "hello_world" );
  char buff[15];
  read( fd, buff, 15 );
  write( 1, buff, 15 );
  close( fd );
  unlink( "hello_world" );

  exit( 0 );
}
