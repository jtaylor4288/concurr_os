#include "libc.h"

void main_pipe_server() {
  if ( mkfifo( "hello_world" ) != 0 ){
    write( 1, "aaa", 3 );
  }
  int fd = open( "hello_world" );
  write( fd, "Hello, world!\n", 15 );
  close( fd );
  if ( unlink( "hello_world" ) != 0 ){
    write( 1, "bbb", 3 );
  }

  exit( 0 );
}
