#include "pipe.h"

#define PIPE_LIMIT 64

pipe_t pipe_array[PIPE_LIMIT];
pipe_t *next_pipe = &pipe_array[0];


void init_pipes() {
  memset( pipe_array, 0, PIPE_LIMIT * sizeof( pipe_t ) );
  for ( size_t i = 0; i < PIPE_LIMIT; ++i ) {
    pipe_array[i].next = &pipe_array[i+1];
  }
  pipe_array[PIPE_LIMIT-1].next = NULL;
}


pipe_t* create_pipe( const char *name ) {
  if ( next_pipe == NULL ) return NULL;

  pipe_t *new_pipe = next_pipe;
  next_pipe = new_pipe->next;

  size_t len = strlen( name );
  if ( len >= PIPE_NAME_MAX_LEN ) return NULL;
  memset( &new_pipe->name, '\x00', PIPE_NAME_MAX_LEN );
  memcpy( &new_pipe->name, name, len );

  new_pipe->read = 0;
  new_pipe->write = 0;
  memset( &new_pipe->buff, '\x00', PIPE_BUFF_SIZE );

  return new_pipe;
}


bool pipe_is_removed( pipe_t *pipe ) {
  return pipe->open_count == 0 && pipe->name[0] == '\x00';
}


void try_remove_pipe( pipe_t *pipe ) {
  if ( pipe_is_removed( pipe ) ) {
    pipe->next = next_pipe;
    next_pipe = pipe;
  }
}


void unlink_pipe( pipe_t *pipe ) {
  memset( &pipe->name, '\x00', PIPE_NAME_MAX_LEN );
  try_remove_pipe( pipe );
}


pipe_t* find_pipe( const char *name ) {
  if ( name[0] == '\x00' ) return NULL;
  for ( size_t i = 0; i < PIPE_LIMIT; ++i ) {
    if ( strcmp( pipe_array[i].name, name ) == 0 ) {
      return &pipe_array[i];
    }
  }
  return NULL;
}


pipe_t* open_pipe( pipe_t *pipe ) {
  pipe->open_count++;
  return pipe;
}


void close_pipe( pipe_t *pipe ) {
  pipe->open_count--;
  try_remove_pipe( pipe );
}


int read_pipe( pipe_t *pipe, char *buff, size_t n ) {
  if ( pipe_is_removed( pipe ) ) return -1;

  int b = 0;
  while ( pipe->read != pipe->write && b < n ) {
    buff[ b++ ] = pipe->buff[ pipe->read++ ];
    pipe->read %= PIPE_BUFF_SIZE;
  }
  return b;
}


int write_pipe( pipe_t *pipe, const char *buff, size_t n ) {
  if ( pipe_is_removed( pipe ) ) return -1;

  int b = 0;
  size_t next_write = ( pipe->write + 1 ) % PIPE_BUFF_SIZE;
  while ( next_write != pipe->read && b < n ) {
    pipe->buff[ pipe->write ] = buff[ b++ ];
    pipe->write = next_write++;
    next_write %= PIPE_BUFF_SIZE;
  }
  return b;
}
