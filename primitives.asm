; UEFI calling parameters: rcx, rdx, r8, r9, stack...
;

section .text code executable readable
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

extern mark_all_runnable

; extern irqhandler
global irqfun_default
irqfun_default:
  jmp panic

global irqfun_com1
irqfun_com1:
  pushf
  push rax
  push rdx
  cld

  ; Disable other interrupts
  sti
  push rax
  push rcx
  push rdx
  push r8
  push r9
  push r10
  push r11

  call mark_all_runnable

  pop r11
  pop r10
  pop r9
  pop r8
  pop rdx
  pop rcx
  pop rax
  ; Reenable interrupts
  cli

  ; Port for PIC 1
  mov dx, 0x20
  ; EOI (End of Interrupt) command
  mov al, 0x20
  ; Send command
  out dx, al

  pop rdx
  pop rax
  popf
  iretq

global irqfun_nic
irqfun_nic:
  pushf
  push rax
  push rdx
  cld

  ; Disable other interrupts
  sti
  push rax
  push rcx
  push rdx
  push r8
  push r9
  push r10
  push r11

  call mark_all_runnable

  pop r11
  pop r10
  pop r9
  pop r8
  pop rdx
  pop rcx
  pop rax
  ; Reenable interrupts
  cli

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
  popf
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
; Disables interupts
global panic
panic:
  cli
  mov rax, 0xDEADDEAD
  hlt
  jmp panic


global switch_to_task
switch_to_task:
  push rax
  push rbx
  push rcx
  push rdx
  push rbp
  push rsi
  push rdi
  push r8
  push r9
  push r10
  push r11
  push r12
  push r13
  push r14
  push r15

  mov [rcx], rsp
  mov rsp, [rdx]

  pop r15
  pop r14
  pop r13
  pop r12
  pop r11
  pop r10
  pop r9
  pop r8
  pop rdi
  pop rsi
  pop rbp
  pop rdx
  pop rcx
  pop rbx
  pop rax
  ret
