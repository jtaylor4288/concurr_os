#ifndef __PIPE_H
#define __PIPE_H


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <string.h>


#define PIPE_BUFF_SIZE 64
#define PIPE_NAME_MAX_LEN 16


typedef struct pipe_t pipe_t;


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


void init_pipes();

pipe_t* create_pipe( const char *name );
bool pipe_is_removed( pipe_t *pipe );
void try_remove_pipe( pipe_t *pipe );
void unlink_pipe( pipe_t *pipe );


pipe_t* find_pipe( const char *name );

pipe_t* open_pipe( pipe_t *pipe );
void close_pipe( pipe_t *pipe );

int read_pipe( pipe_t *pipe, char *buff, size_t n );
int write_pipe( pipe_t *pipe, const char *buff, size_t n );



#endif
