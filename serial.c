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

// This assumes that interrupts are enabled when it is called.
char read_serial() {
   while (1) {
     disable_interrupts();
     if (serial_received() != 0) break;
     enable_interrupts_and_halt();
   }
   enable_interrupts();

   return inb(SERIAL_COM1_BASE);
}

void drain_serial() {
  while (serial_received()) {
    inb(SERIAL_COM1_BASE);
  }
}


void init_serial() {
   uint16_t port = SERIAL_COM1_BASE;
   // Disable all interrupts on the port while setup happens
   outb(0x00, port + 1);
   // Enable the DLAB. This changes the meaning of ports 0/1 which allows
   // setting the baud rate divisor.
   outb(0x80, port + 3);
   // Set divisor to 3 (low byte) 38400 baud
   outb(0x03, port + 0);
   outb(0x00, port + 1);

   // Clear the DLAB, and set the protcol as:
   // 8 bits, no parity, one stop bit
   outb(0x03, port + 3);
   // Don't set FIFOs as they don't seem to do anything in QEMU.
   // Enable IRQs on Receive.
   outb(0x01, port + 1);
}
