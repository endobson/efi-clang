; UEFI calling parameters: rcx, rdx, r8, r9, stack...
;

section '.text' code executable readable
 
global inb
inb:
  mov dx, cx
  in al, dx
  ret

global outb 
outb:
  mov al, cl ; Move the data to be sent into the al register
             ; The address of the I/O port is already in the dx register
  out dx, al
  ret

global load_idt
load_idt:
  lidt [ecx]
  ret

global store_gdt
store_gdt:
  sgdt [ecx]
  ret

global load_gdt
load_gdt:
  lgdt [ecx]
  ret

global load_segments
load_segments:
  mov ss, dx
  mov ds, dx
  ret


global irqfun
irqfun:
  iretq


global halt 
halt:
  hlt
  ret
