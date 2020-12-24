#ifndef PRIMITIVES_H_
#define PRIMITIVES_H_

// Header file for the assembly primitives.

#include <stdint.h>

//// Ports

// Read a byte from the specified port
uint8_t inb(uint16_t port);
// Read 2 bytes from the specified port
uint16_t inw(uint16_t port);
// Read 4 bytes from the specified port
uint32_t inl(uint16_t port);

// Write a byte to the specified port
void outb(uint8_t v, uint16_t port);
// Write 2 bytes to the specified port
void outw(uint16_t v, uint16_t port);
// Write 4 bytes to the specified port
void outl(uint32_t v, uint16_t port);

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


// Assembly routines that handle interrupts.
// Do not follow standard calling convention.

// Default IRQ function. Panics the machine.
void irqfun_default();

// IRQ function for COM1.
void irqfun_com1();

// IRQ function for .
void irqfun_nic();

// Halt until an interrupt.
void halt();

// Enable interrupts and halt until one.
void enable_interrupts_and_halt();

// disable interrupts.
void disable_interrupts();

// Enable interrupts.
void enable_interrupts();

// Panic. Enters inifinite halt loop.
void panic();

#endif // PRIMITIVES_H_
