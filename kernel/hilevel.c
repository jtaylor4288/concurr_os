/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

#include "hilevel.h"

// The maximum number of allowed processes. This is static as the pcb is statically allocated
#define PROC_LIMIT 15

pcb_t pcb[PROC_LIMIT];
pcb_t *curr_proc = NULL;
size_t proc_count = 0;

// TODO: Make this more efficient
// TODO: Pick a good loop condition
pid_t get_new_pid() {
  for ( pid_t p = 1; p < PROC_LIMIT; ++p ) {
    int used = 0;
    for ( size_t i = 0; i < proc_count; ++i ) {
      if ( p == pcb[i].pid ) {
        used = 1;
        break;
      }
    }
    if ( !used ) return p;
  }
  return -1;
}

pcb_t* get_by_pid( pid_t pid ) {
  for ( size_t i = 0; i < proc_count; ++i ) {
    if ( pcb[i].pid == pid ) return &pcb[i];
  }
  return NULL;
}

// Iterates through all processes to find the most important
// Importance is determined by adding the priority and the age
// Each process has its age incremented by 1 and the current process' is reset to 0
// The curr_proc pointer updated to the newly chosen process
void pick_next_proc() {
  int max_imp = 0;
  curr_proc->age = -1;
  for ( size_t i = 0; i < proc_count; ++i ) {
    pcb[i].age += 1;
    int imp = pcb[i].age + pcb[i].priority;
    if ( imp > max_imp ) {
      max_imp = imp;
      curr_proc = &pcb[i];
    }
  }
}

// TODO: document this
pcb_t* add_proc(uint32_t pc, uint32_t sp) {
  pcb_t *new_proc = &pcb[proc_count++];
  memset( new_proc, 0, sizeof( pcb_t) );
  new_proc->pid      = get_new_pid();
  new_proc->status   = STATUS_READY;
  new_proc->ctx.cpsr = 0x50;
  new_proc->ctx.pc   = pc;
  new_proc->ctx.sp   = sp;
  new_proc->priority = 0;
  return new_proc;
}

// TODO: document this
// TODO: zero memory?
void remove_proc( pid_t pid ) {
  pcb_t *to_remove = get_by_pid( pid );
  if ( to_remove != NULL ) {
    pcb_t *to_swap = &pcb[--proc_count];
    memcpy( to_remove, to_swap, sizeof( pcb_t ) );
  }
}

// Swaps out the current process for the new one
void scheduler( ctx_t *ctx ) {
  memcpy( &curr_proc->ctx, ctx, sizeof(ctx_t) );
  curr_proc->status = STATUS_READY;

  pick_next_proc();

  memcpy( ctx, &curr_proc->ctx, sizeof(ctx_t) );
  curr_proc->status = STATUS_EXECUTING;
}


extern void main_console();
extern uint32_t tos_console;

void printstr(const char *c) {
  while ( *c != '\x00' ) {
    PL011_putc( UART0, *c++, true );
  }
}

void hilevel_handler_rst( ctx_t *ctx ) {

  printstr("Starting up:\n");

  // memset( &pcb[ 0 ], 0, sizeof( pcb_t ) );
  // pcb[ 0 ].pid      = 1;
  // pcb[ 0 ].status   = STATUS_READY;
  // pcb[ 0 ].ctx.cpsr = 0x50;
  // pcb[ 0 ].ctx.pc   = ( uint32_t )( &main_console );
  // pcb[ 0 ].ctx.sp   = ( uint32_t )( &tos_console  );
  // pcb[ 0 ].priority = 0;
  pcb_t *console = add_proc( (uint32_t)main_console, tos_console );
  memcpy( ctx, &console->ctx, sizeof( ctx_t ) );
  console->status = STATUS_EXECUTING;
  curr_proc = console;

  printstr("[1/3] Created the console\n");

  TIMER0->Timer1Load  = 0x00100000; // select period = 2^20 ticks ~= 1 sec
  TIMER0->Timer1Ctrl  = 0x00000002; // select 32-bit   timer
  TIMER0->Timer1Ctrl |= 0x00000040; // select periodic timer
  TIMER0->Timer1Ctrl |= 0x00000020; // enable          timer interrupt
  TIMER0->Timer1Ctrl |= 0x00000080; // enable          timer

  GICC0->PMR          = 0x000000F0; // unmask all            interrupts
  GICD0->ISENABLER1  |= 0x00000010; // enable timer          interrupt
  GICC0->CTLR         = 0x00000001; // enable GIC interface
  GICD0->CTLR         = 0x00000001; // enable GIC distributor

  printstr("[2/3] Started timers\n");

  int_enable_irq();

  printstr("[3/3] Started interrupts\n");

  return;
}

void hilevel_handler_irq( ctx_t *ctx ) {
  uint32_t id = GICC0->IAR;

  switch ( id ) {
    case GIC_SOURCE_TIMER0: {
      PL011_putc( UART0, '\n', true );
      scheduler( ctx );
      TIMER0->Timer1IntClr = 0x01;
      break;
    }

    default: {
      // Look up other codes?
    }
  }

  GICC0->EOIR = id;

  return;
}

void hilevel_handler_svc( ctx_t *ctx, uint32_t id ) {

  switch ( id ) {
    case 0x00 : {
      // yield
      //
      // inputs: none
      //
      // output: none
      scheduler( ctx );
      break;
    }

    case 0x01 : {
      // write
      //
      // inputs: r0  int          file descriptor
      //         r1  const void*  buffer to read from
      //         r2  size_t       size of buffer
      //
      // output: r0  int          number of bytes written
      int  fd = ( int ) ( ctx->gpr[ 0 ] );
      char *c = ( char* ) ( ctx->gpr[ 1 ] );
      int   n = ( int ) ( ctx->gpr[ 2 ] );

      for ( int i = 0; i < n; ++i ) {
        PL011_putc( UART0, *c++, true );
      }

      ctx->gpr[ 0 ] = n;
      break;
    }

    case 0x02 : {
      // read
      //
      // inputs: r0  int     file descriptor
      //         r1  void*   buffer to write into
      //         r2  size_t  number of bytes to read
      //
      // output: r0  int     number of bytes read

      // TODO: implement this
      ctx->gpr[0] = 0;
      break;
    }

    case 0x03 : {
      // fork
      //
      // inputs: none
      //
      // output: r0  int  for the parent process, the child process' pid
      //                  for the child process, 0
      //                  in the event of an error, -1

      // TODO: implement this
      ctx->gpr[0] = -1;
      break;
    }

    case 0x04 : {
      // exit
      //
      // inputs: r0  int  status with which to exit
      //
      // output: none

      // TODO: implement this
      break;
    }

    case 0x05 : {
      // exec
      //
      // inputs: r0  void*  pointer to the entry point of the new process
      //
      // output: none

      // TODO: implement this
      break;
    }

    case 0x06 : {
      // kill
      //
      // inputs: r0  int  pid of the process to kill
      //         r1  int  status with which to kill the process
      //
      // output: r0  int  ???

      // TODO: implement this
      break;
    }

    case 0x07 : {
      // nice
      //
      // inputs: r0  int  pid of the process to modify
      //         r1  int  new priority of the process
      //
      // output: none

      // TODO: implement this
      break;
    }

    default: {
      // unknown
    }
  }

  return;
}
