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
