#include "display.h"

uint16_t fb_a[ DISPLAY_HEIGHT ][ DISPLAY_WIDTH ];
uint16_t fb_b[ DISPLAY_HEIGHT ][ DISPLAY_WIDTH ];

uint32_t write_buff, both_buff;

display_t* get_display() {
  // return (display_t*) &fb_a;
  return ( display_t* ) write_buff;
}

display_t* flip_display() {
  LCD->LCDUPBASE = write_buff;
  write_buff ^= both_buff;
  return (display_t*) write_buff;
}

void init_display() {
  write_buff = (uint32_t)&fb_a;
  both_buff = ((uint32_t)&fb_a) ^ ((uint32_t)&fb_b);

  SYSCONF->CLCD      = 0x2CAC;     // per per Table 4.3 of datasheet
  LCD->LCDTiming0    = 0x1313A4C4; // per per Table 4.3 of datasheet
  LCD->LCDTiming1    = 0x0505F657; // per per Table 4.3 of datasheet
  LCD->LCDTiming2    = 0x071F1800; // per per Table 4.3 of datasheet

  LCD->LCDUPBASE = (uint32_t) get_display() ;

  LCD->LCDControl    = 0x00000020; // select TFT   display type
  LCD->LCDControl   |= 0x00000008; // select 16BPP display mode
  LCD->LCDControl   |= 0x00000800; // power-on LCD controller
  LCD->LCDControl   |= 0x00000001; // enable   LCD controller

  PS20->CR           = 0x00000010; // enable PS/2    (Rx) interrupt
  PS20->CR          |= 0x00000004; // enable PS/2 (Tx+Rx)
  PS21->CR           = 0x00000010; // enable PS/2    (Rx) interrupt
  PS21->CR          |= 0x00000004; // enable PS/2 (Tx+Rx)

  uint8_t ack;

        PL050_putc( PS20, 0xF4 );  // transmit PS/2 enable command
  ack = PL050_getc( PS20       );  // receive  PS/2 acknowledgement
        PL050_putc( PS21, 0xF4 );  // transmit PS/2 enable command
  ack = PL050_getc( PS21       );  // receive  PS/2 acknowledgement

  GICC0->PMR         = 0x000000F0; // unmask all          interrupts
  GICD0->ISENABLER1 |= 0x00300000; // enable PS2          interrupts
  GICC0->CTLR        = 0x00000001; // enable GIC interface
  GICD0->CTLR        = 0x00000001; // enable GIC distributor
}
