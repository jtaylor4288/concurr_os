/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

#ifndef __HILEVEL_H
#define __HILEVEL_H

// Include functionality relating to newlib (the standard C library).

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <string.h>

// TODO: Figure out what each one does
#include   "GIC.h"
#include "PL011.h"
#include "SP804.h"

// Include functionality relating to the kernel.

#include "lolevel.h"
#include     "int.h"

typedef int pid_t;

typedef enum {
  STATUS_CREATED,
  STATUS_READY,
  STATUS_EXECUTING,
  STATUS_WAITING,
  STATUS_TERMINATED
} status_t;

typedef struct {
  uint32_t cpsr, pc, gpr[13], sp, lr;
} ctx_t;

typedef struct pipe_t pipe_t;
#define FD_LIMIT 16

typedef struct {
     pid_t pid;
  status_t status;
     ctx_t ctx;
       int priority;
       int age;
   // TODO: Add read/write flags
   pipe_t* fds[FD_LIMIT];
} pcb_t;

#define PIPE_BUFF_SIZE 64
#define PIPE_NAME_MAX_LEN 16

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

#endif
