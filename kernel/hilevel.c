/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

#include "hilevel.h"

// The maximum number of allowed processes. This is static as the pcb is statically allocated
#define PROC_LIMIT 15
#define STACK_SIZE 0x1000


pcb_t pcb[PROC_LIMIT];
pcb_t *curr_proc = NULL;
size_t proc_count = 0;


extern uint32_t tos_user;
extern uint32_t bos_user;


void init_pcb() {
  pid_t    pid = 1;
  uint32_t sp  = (uint32_t) &tos_user;
  for ( size_t i = 0; i < PROC_LIMIT; ( ++i, sp -= STACK_SIZE ) ) {
    pcb[i].pid      = pid++;
    pcb[i].ctx.pc   = (uint32_t) NULL;
    pcb[i].ctx.sp   = sp;
  }
}


uint32_t get_tos( uint32_t sp ) {
  uint32_t tos = (uint32_t) &bos_user;
  while ( tos <= sp ) {
    tos += STACK_SIZE;
  }
  return tos;
}


// TODO: document this
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


typedef void(*void_fn)();


// TODO: document this
pcb_t* create_proc(void_fn pc) {
  pcb_t *new_proc = &pcb[proc_count++];

  new_proc->status   = STATUS_READY;
  new_proc->ctx.cpsr = 0x50;
  new_proc->ctx.pc   = (uint32_t) pc;

  memset( &new_proc->ctx.gpr, 0, 13 * sizeof(uint32_t) );
  uint32_t *bos = (uint32_t*) ( new_proc->ctx.sp - STACK_SIZE );
  memset( bos, 0, STACK_SIZE );

  new_proc->priority = 0;
  new_proc->age      = 0;
  return new_proc;
}


// TODO: document this
void remove_proc( pcb_t *to_remove ) {
  if ( to_remove == NULL ) return;

  to_remove->ctx.sp = get_tos( to_remove->ctx.sp );

  pcb_t *to_swap = &pcb[--proc_count];
  if ( to_swap == to_remove ) return;

  pcb_t temp;
  memcpy( &temp,     to_remove, sizeof( pcb_t ) );
  memcpy( to_remove, to_swap,   sizeof( pcb_t ) );
  memcpy( to_swap,   &temp,     sizeof( pcb_t ) );
}


pcb_t* duplicate_proc( pcb_t *src_proc ) {
  pcb_t *dst_proc = create_proc( (void_fn) src_proc->ctx.pc );

  dst_proc->ctx.cpsr = src_proc->ctx.cpsr;
  dst_proc->ctx.lr   = src_proc->ctx.lr;
  dst_proc->age      = src_proc->age;

  memcpy( dst_proc->ctx.gpr, src_proc->ctx.gpr, 13 * sizeof(uint32_t) );

  uint32_t *tos_src = (uint32_t*) get_tos( src_proc->ctx.sp );
  uint32_t *bos_src = tos_src - STACK_SIZE;
  uint32_t *bos_dst = (uint32_t*) dst_proc->ctx.sp - STACK_SIZE;
  memcpy( bos_dst, bos_src, STACK_SIZE );

  uint32_t stack_delta = (uint32_t)(tos_src) - src_proc->ctx.sp;
  dst_proc->ctx.sp -= stack_delta;

  return dst_proc;
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

void printstr(const char *c) {
  while ( *c != '\x00' ) {
    PL011_putc( UART0, *c++, true );
  }
}


void hilevel_handler_rst( ctx_t *ctx ) {

  printstr("Hello!\n");

  init_pcb();

  pcb_t *console = create_proc( &main_console );
  memcpy( ctx, &console->ctx, sizeof( ctx_t ) );
  console->status = STATUS_EXECUTING;
  curr_proc = console;

  TIMER0->Timer1Load  = 0x00100000; // select period = 2^20 ticks ~= 1 sec
  TIMER0->Timer1Ctrl  = 0x00000002; // select 32-bit   timer
  TIMER0->Timer1Ctrl |= 0x00000040; // select periodic timer
  TIMER0->Timer1Ctrl |= 0x00000020; // enable          timer interrupt
  TIMER0->Timer1Ctrl |= 0x00000080; // enable          timer

  GICC0->PMR          = 0x000000F0; // unmask all            interrupts
  GICD0->ISENABLER1  |= 0x00000010; // enable timer          interrupt
  GICC0->CTLR         = 0x00000001; // enable GIC interface
  GICD0->CTLR         = 0x00000001; // enable GIC distributor

  int_enable_irq();

  return;
}


void hilevel_handler_irq( ctx_t *ctx ) {
  uint32_t id = GICC0->IAR;

  switch ( id ) {
    case GIC_SOURCE_TIMER0: {
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
      memcpy( &curr_proc->ctx, ctx, sizeof(ctx_t) ); // save_ctx
      pcb_t *child = duplicate_proc( curr_proc );
      child->ctx.gpr[0] = 0;
      ctx->gpr[0] = child->pid;
      break;
    }

    case 0x04 : {
      // exit
      //
      // inputs: r0  int  status with which to exit
      //
      // output: none

      remove_proc( curr_proc );
      // scheduler();

      // stolen from scheduler:
      pick_next_proc();

      memcpy( ctx, &curr_proc->ctx, sizeof(ctx_t) );
      curr_proc->status = STATUS_EXECUTING;
      //

      break;
    }

    case 0x05 : {
      // exec
      //
      // inputs: r0  void*  pointer to the entry point of the new process
      //
      // output: none

      // TODO: Zero stack and stuff?
      ctx->pc = ctx->gpr[0];
      ctx->sp = get_tos( ctx->sp );
      break;
    }

    case 0x06 : {
      // kill
      //
      // inputs: r0  int  pid of the process to kill
      //         r1  int  status with which to kill the process
      //
      // output: r0  int  0 on success, -1 on error
      pcb_t *to_remove = get_by_pid( ctx->gpr[0] );
      if ( to_remove ) {
        remove_proc( to_remove );
        ctx->gpr[0] = 0;
      } else {
        ctx->gpr[0] = -1;
      }
      break;
    }

    case 0x07 : {
      // nice
      //
      // inputs: r0  int  pid of the process to modify
      //         r1  int  new priority of the process
      //
      // output: none
      pcb_t *to_change = get_by_pid( ctx->gpr[0] );
      if ( to_change ) {
        to_change->pid = ctx->gpr[1];
      }
      break;
    }

    default: {
      // unknown
    }
  }

  return;
}
