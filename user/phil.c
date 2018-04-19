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

#define PHIL_COUNT 16

typedef enum { none, dirty, clean } state;

void main_phil() {
  int i;
  // Make 15 copies, 16 processes in total
  for ( i = 0; i < PHIL_COUNT-1; ++i ) {
    if ( fork() == 0 ) {
      break;
    }
  }

  char l = xtoa( ( i - 1 + PHIL_COUNT ) % PHIL_COUNT );
  char c = xtoa( i );
  char r = xtoa( ( i + 1              ) % PHIL_COUNT );

  // phil src -> dst

  char *path_wl = "phil__";
  path_wl[4] = c;
  path_wl[5] = l;

  char *path_rl = "phil__";
  path_wl[4] = l;
  path_wl[5] = c;

  char *path_wr = "phil__";
  path_wr[4] = c;
  path_wr[5] = r;

  char *path_rr = "phil__";
  path_wr[4] = r;
  path_wr[5] = c;


  mkfifo( path_rl );
  mkfifo( path_rr );
  yield();

  int pipe_rl = open( path_rl );
  int pipe_rr = open( path_rr );

  int pipe_wl, pipe_wr;
  while ( ( pipe_wl = open( path_wl ) ) == -1 ) {
    yield();
  }
  while ( ( pipe_wr = open( path_wr ) ) == -1 ) {
    yield();
  }

  state fork_l = c < l ? dirty : none ;
  state fork_r = c < r ? dirty : none ;

  char _c;
  char msg[12] = "_: spaghet\n";
  msg[0] = c;

  while ( 1 ) {
    switch ( fork_l ) {
      case dirty:
        if ( write( pipe_wl, &_c, 1 ) == 1 ) fork_l = none;
        break;
      case none:
        if ( read( pipe_rl, &_c, 1 ) == 1 ) fork_l = clean;
    }

    switch ( fork_r ) {
      case dirty:
        if ( write( pipe_wr, &_c, 1 ) == 1 ) fork_r = none;
        break;
      case none:
        if ( read( pipe_rr, &_c, 1 ) == 1 ) fork_r = clean;
    }

    if ( fork_l == clean && fork_r == clean ) {
      fork_l = fork_r = dirty;
      write( 1, msg, 11 );
    }
    yield();
  }

  close( pipe_wl );
  close( pipe_rl );
  close( pipe_wr );
  close( pipe_rr );

  unlink( path_rl );
  unlink( path_rr );

  exit( 0 );
}
