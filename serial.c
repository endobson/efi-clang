#include "primitives.h"

#define SERIAL_COM1_BASE                0x3F8      /* COM1 base port */

int is_transmit_empty() {
   return inb(SERIAL_COM1_BASE + 5) & 0x20;
}

void write_serial(uint8_t v) {
   while (is_transmit_empty() == 0);

   outb(v, SERIAL_COM1_BASE);
}

void write_serial_cstr(char* str) {
  while (*str != 0) {
    write_serial(*str);
    str++;
  }
}

int serial_received() {
   return inb(SERIAL_COM1_BASE + 5) & 1;
}

char read_serial() {
   while (serial_received() == 0);

   return inb(SERIAL_COM1_BASE);
}

/* Unused serial initialization.
void init_serial(unsigned short port) {
   outb(port + 1, 0x00);    // Disable all interrupts
   outb(port + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(port + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outb(port + 1, 0x00);    //                  (hi byte)
   outb(port + 3, 0x03);    // 8 bits, no parity, one stop bit
   outb(port + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outb(port + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}
*/
