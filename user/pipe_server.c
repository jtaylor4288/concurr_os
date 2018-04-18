#include "libc.h"

main_pipe_server() {
  assert( mkfifo( "hello_world" ) == 0 );
  int fd = open( "hello_world" );
  write( fd, "Hello, world!\n", 15 );
  close( fd );
  assert( unlink( "hello_world" ) == 0 );
}
