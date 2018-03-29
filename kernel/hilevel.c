/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

#include "hilevel.h"

#define PROC_LIMIT 15

pcb_t pcb[PROC_LIMIT];
pcb_t *curr_proc;
uint32_t proc_count;

void pick_next_proc() {
  uint32_t max_imp = 0;
  for ( int i = 0; i < proc_count; ++i ) {
    ++pcb[i].age;
    uint32_t imp = pcb[i].age + pcb[i].priority;
    if ( imp > max_imp ) {
      max_imp = imp;
      curr_proc = &pcb[i];
    }
  }
}

// Currently a round robin approach
void scheduler( ctx_t *ctx ) {
  memcpy( &curr_proc->ctx, ctx, sizeof(ctx_t) );
  curr_proc->status = STATUS_READY;

  // curr_proc = (curr_proc + 1) % proc_count;
  pick_next_proc();

  memcpy( ctx, &curr_proc->ctx, sizeof(ctx_t) );
  curr_proc->status = STATUS_EXECUTING;
}


extern void main_P3();
extern void main_P4();

extern uint32_t tos_P3;
extern uint32_t tos_P4;

void hilevel_handler_rst( ctx_t *ctx ) {

  char msg[15] = "Hello, world!\n";
  for ( int i = 0; i < 14; ++i ) {
    PL011_putc( UART0, msg[i], true );
  }

  memset( &pcb[ 0 ], 0, sizeof( pcb_t ) );
  pcb[ 0 ].pid      = 1;
  pcb[ 0 ].status   = STATUS_READY;
  pcb[ 0 ].ctx.cpsr = 0x50;
  pcb[ 0 ].ctx.pc   = ( uint32_t )( &main_P3 );
  pcb[ 0 ].ctx.sp   = ( uint32_t )( &tos_P3  );
  pcb[ 0 ].priority = 0;

  memset( &pcb[ 1 ], 0, sizeof( pcb_t ) );
  pcb[ 1 ].pid      = 2;
  pcb[ 1 ].status   = STATUS_READY;
  pcb[ 1 ].ctx.cpsr = 0x50;
  pcb[ 1 ].ctx.pc   = ( uint32_t )( &main_P4 );
  pcb[ 1 ].ctx.sp   = ( uint32_t )( &tos_P4  );
  pcb[ 1 ].priority = 0;

  memcpy( ctx, &pcb[0].ctx, sizeof( ctx_t ) );
  pcb[ 0 ].status = STATUS_EXECUTING;
  curr_proc = &pcb[0]
  proc_count = 2;

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
      break;
    }

    case 0x01 : {
      int  fd = ( int ) ( ctx->gpr[ 0 ] );
      char *c = ( char* ) ( ctx->gpr[ 1 ] );
      int   n = ( int ) ( ctx->gpr[ 2 ] );

      for ( int i = 0; i < n; ++i ) {
        PL011_putc( UART0, *c++, true );
      }

      ctx->gpr[ 0 ] = n;
      break;
    }

    default: {
      // unknown
    }
  }

  return;
}
