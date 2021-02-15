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

typedef enum TaskState {
  TaskState_Runnable,
  TaskState_Blocked,
} TaskState;

extern int yield(TaskState state);

// This assumes that interrupts are enabled when it is called.
char read_serial() {
   if (serial_received() == 0) {
     disable_interrupts();
     while (1) {
       if (serial_received() != 0) break;
       yield(TaskState_Blocked);
     }
     enable_interrupts();
   }

   return inb(SERIAL_COM1_BASE);
}

void drain_serial() {
  while (serial_received()) {
    inb(SERIAL_COM1_BASE);
  }
}
