#ifndef DESCRIPTOR_TABLES_H_
#define DESCRIPTOR_TABLES_H_

#include <stdint.h>
#include "primitives.h"

typedef struct IDTEntry {
  uint16_t offset_1; // offset bits 0..15
  uint16_t selector; // a code segment selector in GDT or LDT
  uint8_t ist;       // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
  uint8_t type_attr; // type and attributes
  uint16_t offset_2; // offset bits 16..31
  uint32_t offset_3; // offset bits 32..63
  uint32_t zero;     // reserved
} __attribute__ ((packed)) IDTEntry;


typedef struct GDTEntry {
  uint16_t limit_1; // limit bits 0..15
  uint16_t base_1;  // base addr bits 0..15
  uint8_t base_2;  // base addr bits 16..23
  uint8_t flags1; // In BigEndian: P(1) - DPL(2) - S(1) - Type (4)
  uint8_t flags2; // In BigEndian: G(1) - D/B(1) - L(1) - AVL(1) - Limit(4)[19..16]
  uint8_t base_3; // base addr bits 24..31
} __attribute__ ((packed)) GDTEntry;


// The actual staticly allocated descriptor tables.

extern IDTEntry idt_entries[];
extern IDTDescr idt_descr;

extern GDTEntry gdt_entries[];
extern GDTDescr gdt_descr;

#endif // DESCRIPTOR_TABLES_H_
