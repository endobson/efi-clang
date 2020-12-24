; UEFI calling parameters: rcx, rdx, r8, r9, stack...
;

section '.text' code executable readable
;;;;;;;;
; Input from Port
global inb
inb:
  mov dx, cx ; Move the port to the dx register
  in al, dx  ; Must use these registers for this instruction
  ret

global inw
inw:
  mov dx, cx ; Move the port to the dx register
  in ax, dx  ; Must use these registers for this instruction
  ret

global inl
inl:
  mov dx, cx ; Move the port to the dx register
  in eax, dx  ; Must use these registers for this instruction
  ret


; Output to Port
global outb
outb:
  mov al, cl ; Move the data to be sent into the al register
             ; The address of the I/O port is already in the dx register
  out dx, al ; Must use these registers for this instruction
  ret

global outw
outw:
  mov ax, cx ; Move the data to be sent into the ax register
             ; The address of the I/O port is already in the dx register
  out dx, ax ; Must use these registers for this instruction
  ret

global outl
outl:
  mov eax, ecx ; Move the data to be sent into the eax register
               ; The address of the I/O port is already in the dx register
  out dx, eax  ; Must use these registers for this instruction
  ret

;;;;;;;;
; Load the specified Interrupt Descriptor Table
global load_idt
load_idt:
  lidt [ecx]
  ret


; extern irqhandler
global irqfun_default
irqfun_default:
  jmp panic

global irqfun_com1
irqfun_com1:
  push rax
  push rdx
  ; Port for PIC 1
  mov dx, 0x20
  ; EOI (End of Interrupt) command
  mov al, 0x20
  ; Send command
  out dx, al

  pop rdx
  pop rax
  iretq

global irqfun_nic
irqfun_nic:
  push rax
  push rdx
  ; Port for PIC 1
  mov dx, 0x20
  ; EOI (End of Interrupt) command
  mov al, 0x20
  ; Send command
  out dx, al

  ; Port for PIC 2
  mov dx, 0xa0
  ; Send command (Still EOI)
  out dx, al

  pop rdx
  pop rax
  iretq

;; Untested Global Descriptor Table code
; global store_gdt
; store_gdt:
;   sgdt [ecx]
;   ret
;
; global load_gdt
; load_gdt:
;   lgdt [ecx]
;   ret
;
; global load_segments
; load_segments:
;   mov ss, dx
;   mov ds, dx
;   ret

;;;;;;;;
; Halt until an interrupt
global halt
halt:
  hlt
  ret

; Enable interrupts and wait for one.
global enable_interrupts_and_halt
enable_interrupts_and_halt:
  sti
  hlt
  ret

; Disable interrupts
global disable_interrupts
disable_interrupts:
  cli
  ret

; Enable interrupts
global enable_interrupts
enable_interrupts:
  sti
  ret

; Panic, and stop forever
global panic
panic:
  mov rax, 0xDEADDEAD
  hlt
  jmp panic
