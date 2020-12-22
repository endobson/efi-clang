#include "descriptor_tables.h"

IDTEntry idt_entries[256];
IDTDescr idt_descr;

GDTEntry gdt_entries[3];
GDTDescr gdt_descr;
