// Header file for the assembly primitives.

#ifndef PRIMITIVES_H_
#define PRIMITIVES_H_

#include <stdint.h>

//// Ports

// Read a byte from the specified port
uint8_t inb(uint16_t port);

// Write a byte to the specified port
void outb(uint8_t v, uint16_t port);

//// Segments

typedef struct GDTDescr {
  uint16_t limit;
  uint64_t base_addr;
} __attribute__ ((packed)) GDTDescr;

/* Untested
// Global Descriptor Tables
// Load the specified GDT.
void load_gdt(GDTDescr* gdt);
// Store the current GDT to the specified location.
void store_gdt(GDTDescr* gdt);

extern void load_segments(uint16_t code, uint16_t data);
*/

typedef struct IDTDescr {
  uint16_t limit;
  uint64_t base_addr;
} __attribute__ ((packed)) IDTDescr;

// Interrupt Descriptor Table
// Load the specified IDT;
void load_idt(IDTDescr* idt);


// Assembly routine that handles interrupts.
// Does not follow standard calling convention.
void irqfun();

// Halt until an interrupt.
void halt();

// Panic. Enters inifinite halt loop.
void panic();

#endif // PRIMITIVES_H_
