#ifndef __PIPE_H
#define __PIPE_H


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <string.h>


#define PIPE_BUFF_SIZE 64
#define PIPE_NAME_MAX_LEN 16


typedef struct pipe_t pipe_t;

// A struct for storing named pipes.
//
// The pipe is implemented with a circular buffer ( the annoymous struct in the union ).
//
// A pipe will be removed once two conditions have been met:
//   1) open_count == 0    ( no open file descriptors )
//   2) name[0] == '\x00'  ( pipe has been unlinked )
// A pipe in this state is called 'dead'.
//
// Dead pipes form a linked list to ease finding a pipe for creation. The head of this
// list is stored next_pipe [pipe.c].
//
// Pipe related functions, especially related to creation and deletion, will not work
// correctly until init_pipes has been called.

struct pipe_t {
  uint32_t open_count;
  char name[PIPE_NAME_MAX_LEN];
  union {
    struct {
      size_t read;
      size_t write;
        char buff[PIPE_BUFF_SIZE];
    };
    pipe_t *next;
  };
};


// Initialises the pipe array  ( pipe_array [pipe.c] )
void init_pipes();

// Creates a new, unopened, pipe and returns a pointer to it
pipe_t* create_pipe( const char *name );

// Returns true if the pipe is dead
bool pipe_is_dead( pipe_t *pipe );

// Adds the pipe to the dead list if it is dead
void try_remove_pipe( pipe_t *pipe );

// Clears the name of this pipe.
// Removes the pipe if it subsequently dies
void unlink_pipe( pipe_t *pipe );

// Returns a pointer to the pipe with the specified name
// Returns NULL if no such pipe exists
pipe_t* find_pipe( const char *name );

// Increments the pipe's open_count and returns the pointer to it
pipe_t* open_pipe( pipe_t *pipe );

// Decrements the pipe's open_count
// Removes the pipe if it subsequently dies
void close_pipe( pipe_t *pipe );

// Reads up to n bytes from the pipe
// Returns the actual number read ( -1 on error )
int read_pipe( pipe_t *pipe, char *buff, size_t n );

// Writes up to n bytes into the pipe
// Returns the actual number written ( -1 on error )
int write_pipe( pipe_t *pipe, const char *buff, size_t n );

#endif
