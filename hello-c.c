
#include "efi.h"

#define SERIAL_COM1_BASE                0x3F8      /* COM1 base port */

uint8_t inb(uint16_t port);
void outb(uint8_t v, uint16_t port);
void halt();

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

char read_serial() {
   while (serial_received() == 0);

   return inb(SERIAL_COM1_BASE);
}

void init_serial(unsigned short port) {
   outb(port + 1, 0x00);    // Disable all interrupts
   outb(port + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(port + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outb(port + 1, 0x00);    //                  (hi byte)
   outb(port + 3, 0x03);    // 8 bits, no parity, one stop bit
   outb(port + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outb(port + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

CHAR16 nibble_to_hexchar16(uint8_t v) {
  switch (v) {
    case 0: return L'0';
    case 1: return L'1';
    case 2: return L'2';
    case 3: return L'3';
    case 4: return L'4';
    case 5: return L'5';
    case 6: return L'6';
    case 7: return L'7';
    case 8: return L'8';
    case 9: return L'9';
    case 10: return L'a';
    case 11: return L'b';
    case 12: return L'c';
    case 13: return L'd';
    case 14: return L'e';
    case 15: return L'f';
  }
  return L'X';
}

char nibble_to_hexchar(uint8_t v) {
  switch (v) {
    case 0: return L'0';
    case 1: return L'1';
    case 2: return L'2';
    case 3: return L'3';
    case 4: return L'4';
    case 5: return L'5';
    case 6: return L'6';
    case 7: return L'7';
    case 8: return L'8';
    case 9: return L'9';
    case 10: return L'a';
    case 11: return L'b';
    case 12: return L'c';
    case 13: return L'd';
    case 14: return L'e';
    case 15: return L'f';
  }
  return L'X';
}


void byte_to_hex(uint8_t v, CHAR16* chars) {
  chars[0] = nibble_to_hexchar16((v >> 4) & 0xF);
  chars[1] = nibble_to_hexchar16((v >> 0) & 0xF);
}

void byte_to_hex2(uint8_t v, char* chars) {
  chars[0] = nibble_to_hexchar((v >> 4) & 0xF);
  chars[1] = nibble_to_hexchar((v >> 0) & 0xF);
}

void guid_to_hex(EFI_GUID guid, CHAR16* chars) {
  byte_to_hex((guid.Data1 >> 24) & 0xff, &chars[0]);
  byte_to_hex((guid.Data1 >> 16) & 0xff, &chars[2]);
  byte_to_hex((guid.Data1 >> 8)  & 0xff, &chars[4]);
  byte_to_hex((guid.Data1 >> 0)  & 0xff, &chars[6]);
  chars[8] = L'-';
  byte_to_hex((guid.Data2 >> 8) & 0xff, &chars[9]);
  byte_to_hex((guid.Data2 >> 0) & 0xff, &chars[11]);
  chars[13] = L'-';
  byte_to_hex((guid.Data3 >> 8) & 0xff, &chars[14]);
  byte_to_hex((guid.Data3 >> 0) & 0xff, &chars[16]);
  chars[18] = L'-';
  byte_to_hex(guid.Data4[0], &chars[19]);
  byte_to_hex(guid.Data4[1], &chars[21]);
  byte_to_hex(guid.Data4[2], &chars[23]);
  byte_to_hex(guid.Data4[3], &chars[25]);
  byte_to_hex(guid.Data4[4], &chars[27]);
  byte_to_hex(guid.Data4[5], &chars[29]);
  byte_to_hex(guid.Data4[6], &chars[31]);
  byte_to_hex(guid.Data4[7], &chars[33]);
  chars[35] = 0;
}


CHAR16* hello_str = L"Hello, you slab of warm meat!\r\n";
CHAR16* newline = L"\r\n";

int guid_equal(EFI_GUID g1, EFI_GUID g2) {
  return g1.Data1 == g2.Data1 &&
         g1.Data2 == g2.Data2 &&
         g1.Data3 == g2.Data3 &&
         g1.Data4[0] == g2.Data4[0] &&
         g1.Data4[1] == g2.Data4[1] &&
         g1.Data4[2] == g2.Data4[2] &&
         g1.Data4[3] == g2.Data4[3] &&
         g1.Data4[4] == g2.Data4[4] &&
         g1.Data4[5] == g2.Data4[5] &&
         g1.Data4[6] == g2.Data4[6] &&
         g1.Data4[7] == g2.Data4[7];
}

void writer_add_cstr(char** writer, char* str) {
  while (*str != 0) {
    **writer = *str;
    (*writer)++;
    str++;
  }
}

void writer_add_bytes(char** writer, char* bytes, int amt) {
  for (int i = 0; i < amt; i++) {
    **writer = bytes[i];
    (*writer)++;
  }
}


void writer_add_newline(char** writer) {
  writer_add_cstr(writer, "\r\n");
}

void writer_terminate(char** writer) {
  writer_add_bytes(writer, "", 1);
}

void writer_add_hex8(char** writer, uint8_t v) {
  char buf[2];
  byte_to_hex2(v, &buf[0]);
  writer_add_bytes(writer, buf, 2);
}
void writer_add_hex16(char** writer, uint32_t v) {
  char buf[4];
  byte_to_hex2((v >> 8)  & 0xff, &buf[0]);
  byte_to_hex2((v >> 0)  & 0xff, &buf[2]);
  writer_add_bytes(writer, buf, 4);
}
void writer_add_hex32(char** writer, uint32_t v) {
  char buf[8];
  byte_to_hex2((v >> 24) & 0xff, &buf[0]);
  byte_to_hex2((v >> 16) & 0xff, &buf[2]);
  byte_to_hex2((v >> 8)  & 0xff, &buf[4]);
  byte_to_hex2((v >> 0)  & 0xff, &buf[6]);
  writer_add_bytes(writer, buf, 8);
}
void writer_add_hex64(char** writer, uint64_t v) {
  char buf[16];
  byte_to_hex2((v >> 56) & 0xff, &buf[0]);
  byte_to_hex2((v >> 48) & 0xff, &buf[2]);
  byte_to_hex2((v >> 40) & 0xff, &buf[4]);
  byte_to_hex2((v >> 32) & 0xff, &buf[6]);
  byte_to_hex2((v >> 24) & 0xff, &buf[8]);
  byte_to_hex2((v >> 16) & 0xff, &buf[10]);
  byte_to_hex2((v >> 8)  & 0xff, &buf[12]);
  byte_to_hex2((v >> 0)  & 0xff, &buf[14]);
  writer_add_bytes(writer, buf, 16);
}


// ACPI structs

// Root System Description Pointer 2.0

typedef struct RSDPDescriptor {
 char Signature[8];
 uint8_t Checksum;
 char OEMID[6];
 uint8_t Revision;
 uint32_t RsdtAddress;
 uint32_t Length;
 uint64_t XsdtAddress;
 uint8_t ExtendedChecksum;
 uint8_t reserved[3];
} __attribute__ ((packed)) RSDPDescriptor;

typedef struct ACPISDTHeader {
  char Signature[4];
  uint32_t Length;
  uint8_t Revision;
  uint8_t Checksum;
  char OEMID[6];
  char OEMTableID[8];
  uint32_t OEMRevision;
  uint32_t CreatorID;
  uint32_t CreatorRevision;
} __attribute__ ((packed)) ACPISDTHeader;

typedef struct XSDT {
  ACPISDTHeader header;
  uint64_t other_tables[];
} __attribute__ ((packed)) XSDT;

typedef struct ConfigSpaceAllocation {
  uint64_t base_address;
  uint16_t group_number;
  uint8_t start_bus_number;
  uint8_t end_bus_number;
  uint32_t reserved;
} __attribute__ ((packed)) ConfigSpaceAllocation;


typedef struct MCFG {
  ACPISDTHeader header;
  uint64_t reserved;
  ConfigSpaceAllocation allocations[];
} __attribute__ ((packed)) MCFG;

typedef struct IDTEntry {
  uint16_t offset_1; // offset bits 0..15
  uint16_t selector; // a code segment selector in GDT or LDT
  uint8_t ist;       // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
  uint8_t type_attr; // type and attributes
  uint16_t offset_2; // offset bits 16..31
  uint32_t offset_3; // offset bits 32..63
  uint32_t zero;     // reserved
} __attribute__ ((packed)) IDTEntry;

typedef struct IDTDescr {
  uint16_t limit;
  uint64_t base_addr;
} __attribute__ ((packed)) IDTDescr;

IDTEntry idt_entries[256];
IDTDescr idt_descr;

void my_memset(uint8_t* ptr, uint8_t v, int amt) {
  for (int i = 0; i < amt; i++) {
    ptr[i] = v;
  }
}

typedef struct GDTEntry {
  uint16_t limit_1; // limit bits 0..15
  uint16_t base_1;  // base addr bits 0..15
  uint8_t base_2;  // base addr bits 16..23
  uint8_t flags1; // In BigEndian: P(1) - DPL(2) - S(1) - Type (4)
  uint8_t flags2; // In BigEndian: G(1) - D/B(1) - L(1) - AVL(1) - Limit(4)[19..16]
  uint8_t base_3; // base addr bits 24..31
} __attribute__ ((packed)) GDTEntry;

typedef struct GDTDescr {
  uint16_t limit;
  uint64_t base_addr;
} __attribute__ ((packed)) GDTDescr;

GDTEntry gdt_entries[3];
GDTDescr gdt_descr;

extern void store_gdt(GDTDescr* gdt);
extern void load_gdt(GDTDescr* gdt);
// extern void load_segments(GDTEntry* code, GDTEntry* data);
extern void load_segments(uint16_t code, uint16_t data);

extern void load_idt(IDTDescr* idt);
extern void irqfun();


// Static array to make printing debug messages easier
char buffer_array[4096];

EFI_STATUS efi_main(EFI_HANDLE ih, EFI_SYSTEM_TABLE* st)
{
    EFI_RUNTIME_SERVICES* runtime_services = st->RuntimeServices;
    CHAR16 chars[3];
    chars[0] = 0;
    chars[1] = 0;
    chars[2] = 0;

    for (int i = 1; i <= 4; i++) {
      chars[0] = i + 64;
      for (int j = 1; j <= 3; j++) {
        chars[1] = j + 64;
        st->ConOut->OutputString(st->ConOut, chars);
        st->ConOut->OutputString(st->ConOut, newline);
      }
    }
    EFI_STATUS s;

    void* acpi_table = 0;
    for (int i = 0; i < st->NumberOfTableEntries; i++) {
        EFI_CONFIGURATION_TABLE ct = st->ConfigurationTable[i];
        EFI_GUID acpi_table_guid = EFI_ACPI_TABLE_GUID;
        if (guid_equal(ct.VendorGuid, acpi_table_guid)) {
          acpi_table = ct.VendorTable;
        }
    }

    {
      char* char_acpi_table = acpi_table;
      {
        CHAR16 hex_sig[17];
        byte_to_hex(char_acpi_table[0], &hex_sig[0]);
        byte_to_hex(char_acpi_table[1], &hex_sig[2]);
        byte_to_hex(char_acpi_table[2], &hex_sig[4]);
        byte_to_hex(char_acpi_table[3], &hex_sig[6]);
        byte_to_hex(char_acpi_table[4], &hex_sig[8]);
        byte_to_hex(char_acpi_table[5], &hex_sig[10]);
        byte_to_hex(char_acpi_table[6], &hex_sig[12]);
        byte_to_hex(char_acpi_table[7], &hex_sig[14]);
        hex_sig[16] = 0;
        st->ConOut->OutputString(st->ConOut, hex_sig);
        st->ConOut->OutputString(st->ConOut, newline);
      }
      {
        CHAR16 sig[9];
        sig[0] = char_acpi_table[0];
        sig[1] = char_acpi_table[1];
        sig[2] = char_acpi_table[2];
        sig[3] = char_acpi_table[3];
        sig[4] = char_acpi_table[4];
        sig[5] = char_acpi_table[5];
        sig[6] = char_acpi_table[6];
        sig[7] = char_acpi_table[7];
        sig[8] = 0;
        st->ConOut->OutputString(st->ConOut, sig);
        st->ConOut->OutputString(st->ConOut, newline);
      }
    }


    if (acpi_table == 0) {
      st->StdErr->OutputString(st->StdErr, L"Unable to get exit boot services");
      st->StdErr->OutputString(st->StdErr, newline);
      return s;
    }



    UINTN memory_map_size = 0;
    EFI_MEMORY_DESCRIPTOR* memory_map;
    UINTN memory_map_key;
    UINTN descriptor_size;
    UINT32 descriptor_version;

    s = st->BootServices->GetMemoryMap(&memory_map_size, memory_map, &memory_map_key, &descriptor_size, &descriptor_version);
    if (s != EFI_BUFFER_TOO_SMALL) {
      st->StdErr->OutputString(st->StdErr, L"Unable to get memory map size");
      st->StdErr->OutputString(st->StdErr, newline);
      return s;
    }
    s = st->BootServices->AllocatePool(EfiLoaderData, memory_map_size, (void **)&memory_map);
    if (s != EFI_SUCCESS) {
      st->StdErr->OutputString(st->StdErr, L"Unable to get allocate pool");
      st->StdErr->OutputString(st->StdErr, newline);
      return s;
    }
    s = st->BootServices->GetMemoryMap(&memory_map_size, memory_map, &memory_map_key, &descriptor_size, &descriptor_version);
    if (s != EFI_SUCCESS) {
      st->StdErr->OutputString(st->StdErr, L"Unable to get memory map");
      st->StdErr->OutputString(st->StdErr, newline);
      return s;
    }

    // Try to Exit UEFI
    s = st->BootServices->ExitBootServices(ih, memory_map_key);
    if (s != EFI_SUCCESS) {
      st->StdErr->OutputString(st->StdErr, L"Unable to get exit boot services");
      st->StdErr->OutputString(st->StdErr, newline);
      return s;
    }

    char* buffer = (char*) buffer_array;
    char* writer;

    RSDPDescriptor* rsdp = acpi_table;
    {
      writer = buffer;
      writer_add_cstr(&writer, "RSDP signature: ");
      writer_add_bytes(&writer, rsdp->Signature, 8);
      writer_add_newline(&writer);
      writer_terminate(&writer);
      write_serial_cstr(buffer);
    }

    XSDT* xsdt = (XSDT*) rsdp->XsdtAddress;
    {
      writer = buffer;
      writer_add_cstr(&writer, "XSDT signature: ");
      writer_add_bytes(&writer, xsdt->header.Signature, 4);
      writer_add_newline(&writer);
      writer_terminate(&writer);
      write_serial_cstr(buffer);
    }

    {
      int num_tables = (xsdt->header.Length - sizeof(ACPISDTHeader)) / sizeof(int64_t);
	  for (int i = 0; i < num_tables; i++) {
        writer = buffer;
        ACPISDTHeader* sdt = (ACPISDTHeader*) xsdt->other_tables[i];
        writer_add_cstr(&writer, "Table signature: ");
        writer_add_bytes(&writer, sdt->Signature, 4);
        writer_add_newline(&writer);
        writer_terminate(&writer);
        write_serial_cstr(buffer);
      }
    }


    // TODO add looking at tables

    MCFG* mcfg = (MCFG*) xsdt->other_tables[3];
    {
      writer = buffer;
      writer_add_cstr(&writer, "MCFG signature: ");
      writer_add_bytes(&writer, mcfg->header.Signature, 4);
      writer_add_newline(&writer);
      writer_add_cstr(&writer, "MCFG length: 0x");
      writer_add_hex32(&writer, mcfg->header.Length);
      writer_add_newline(&writer);
      writer_terminate(&writer);
      write_serial_cstr(buffer);
    }

    ConfigSpaceAllocation* alloc = &mcfg->allocations[0];
    {
      writer = buffer;
      writer_add_cstr(&writer, "Allocation BaseAddress: ");
      writer_add_hex64(&writer, alloc->base_address);
      writer_add_newline(&writer);
      writer_add_cstr(&writer, "Allocation SegmentGroup: ");
      writer_add_hex16(&writer, alloc->group_number);
      writer_add_newline(&writer);
      writer_add_cstr(&writer, "Allocation Start Bus: ");
      writer_add_hex8(&writer, alloc->start_bus_number);
      writer_add_newline(&writer);
      writer_add_cstr(&writer, "Allocation End Bus: ");
      writer_add_hex8(&writer, alloc->end_bus_number);
      writer_add_newline(&writer);
      writer_terminate(&writer);
      write_serial_cstr(buffer);
    }

    // {
    //   char sig[5];
    //   byte_to_hex2((xsdt->header.Length - sizeof(ACPISDTHeader)) / sizeof(ACPISDTHeader), &sig[0]);
    //   sig[2] = '\r';
    //   sig[3] = '\n';
    //   sig[4] = 0;
    //   write_serial_cstr(sig);
    // }

    //
    //
    {
      for (int device_num = 0; device_num < 4; device_num++) {
        writer = buffer;
        uint32_t* ptr = (uint32_t*) (alloc->base_address + (device_num << 15));
        writer_add_cstr(&writer, "Device: 0x");
        writer_add_hex8(&writer, device_num);
        writer_add_newline(&writer);
        writer_add_cstr(&writer, "At Address: 0x");
        writer_add_hex64(&writer, (uint64_t) ptr);
        writer_add_newline(&writer);
        writer_add_hex32(&writer, ptr[0]);
        writer_add_newline(&writer);
        writer_add_hex32(&writer, ptr[1]);
        writer_add_newline(&writer);
        writer_add_hex32(&writer, ptr[2]);
        writer_add_newline(&writer);
        writer_add_hex32(&writer, ptr[3]);
        writer_add_newline(&writer);
        writer_add_hex32(&writer, ptr[4]);
        writer_add_newline(&writer);
        writer_add_hex32(&writer, ptr[5]);
        writer_add_newline(&writer);
        writer_add_hex32(&writer, ptr[6]);
        writer_add_newline(&writer);
        writer_add_hex32(&writer, ptr[7]);
        writer_add_newline(&writer);
        writer_terminate(&writer);
        write_serial_cstr(buffer);
      }
    }

    // uint32_t* vga_mem = (uint32_t*) 0xc0000000;
    // for (int i = 0; i < 0x100000; i++) {
    //   if (i < 0x10100) {
    //     vga_mem[i] = (i % 256) + (i % 256 << 8);
    //   } else if (i < 0x20000) {
    //     vga_mem[i] = 0xFF00;
    //   } else if (i < 0x30000) {
    //     vga_mem[i] = 0xFF0000;
    //   } else if (i < 0x40000) {
    //     vga_mem[i] = 0xFF000000;
    //   } else {
    //     vga_mem[i] = 0xFFFFFFFF;
    //   }
    // }



	// init_serial(SERIAL_COM1_BASE);
    // write_serial(65);
    // write_serial(66);
    // write_serial(67);
    // write_serial(68);
    // for (int i = 0; i < 10; i++) {
    //   uint8_t input = read_serial();
    //   write_serial(input);
    // }

    GDTDescr uefi_gdt_descr;
    store_gdt(&uefi_gdt_descr);
    {
      writer = buffer;
      writer_add_cstr(&writer, "GDT Descriptor Limit: 0x");
      writer_add_hex16(&writer, uefi_gdt_descr.limit);
      writer_add_newline(&writer);
      writer_add_cstr(&writer, "GDT Descriptor Base: 0x");
      writer_add_hex64(&writer, uefi_gdt_descr.base_addr);
      writer_add_newline(&writer);
      writer_add_cstr(&writer, "GDT Bytes");
      writer_add_newline(&writer);
      for (int i = 0; i < 18; i++) {
        writer_add_hex32(&writer, ((uint32_t*) uefi_gdt_descr.base_addr)[i]);
        writer_add_newline(&writer);
      }

      writer_terminate(&writer);
      write_serial_cstr(buffer);
    }


    my_memset((uint8_t*) &gdt_entries, 0, sizeof(GDTEntry) * 3);
    {
      // Set low bits of limit
      gdt_entries[1].limit_1 = 0xffff;

      gdt_entries[1].flags1 = 0x9a;

      // Set top 4 bits of limit;
      gdt_entries[1].flags2 |= 0x0f;
      // Set 64 bit flag on code segment
      gdt_entries[1].flags2 |= 0x20;
      // Increase granularity for limit
      gdt_entries[1].flags2 |= 0x80;
    }
    {
      // Set low bits of limit
      gdt_entries[1].limit_1 = 0xffff;

      gdt_entries[2].flags1 = 0x92;

      // Set top 4 bits of limit;
      gdt_entries[1].flags2 |= 0x0f;
      // Increase granularity for limit
      gdt_entries[1].flags2 |= 0x80;
    }

    gdt_descr.limit = sizeof(GDTEntry) * 3 - 1;
    gdt_descr.base_addr = (uint64_t) &gdt_entries;

    // load_gdt(&gdt_descr);

    {
      writer = buffer;
      writer_add_cstr(&writer, "My GDT Descriptor Limit: 0x");
      writer_add_hex16(&writer, gdt_descr.limit);
      writer_add_newline(&writer);
      writer_add_cstr(&writer, "My GDT Descriptor Base: 0x");
      writer_add_hex64(&writer, gdt_descr.base_addr);
      writer_add_newline(&writer);
      writer_add_cstr(&writer, "My GDT Bytes");
      writer_add_newline(&writer);
      for (int i = 0; i < 6; i++) {
        writer_add_hex32(&writer, ((uint32_t*) gdt_descr.base_addr)[i]);
        writer_add_newline(&writer);
      }

      writer_terminate(&writer);
      write_serial_cstr(buffer);
    }

    // load_segments(0x08, 0x10);

    my_memset((uint8_t*) &idt_entries, 0, sizeof(IDTEntry) * 256);
    uint64_t irq_addr = (uint64_t) irqfun;
    for (int i = 0; i < 256; i++) {
      idt_entries[i].offset_1 = irq_addr & 0xffff;
      idt_entries[i].selector = 0x38;
      idt_entries[i].ist = 0;
      idt_entries[i].type_attr = 0x8e;
      idt_entries[i].offset_2 = (irq_addr >> 16) & 0xffff;
      idt_entries[i].offset_3 = irq_addr >> 32;
      idt_entries[i].zero = 0;
    }



    idt_descr.limit = sizeof(IDTEntry) * 256 - 1;
    idt_descr.base_addr = (uint64_t) &idt_entries;
    load_idt(&idt_descr);
    //{
    //  writer = buffer;
    //  writer_add_cstr(&writer, "IDT Base Address: 0x");
    //  writer_add_hex64(&writer, (uint64_t) &idt_entries);
    //  writer_add_newline(&writer);
    //  writer_add_cstr(&writer, "IDT Descriptor: 0x");
    //  writer_add_hex64(&writer, (uint64_t) &idt_descr);
    //  writer_add_newline(&writer);
    //  writer_terminate(&writer);
    //  write_serial_cstr(buffer);
    //}


    for (int i = 0; i < 3; i++) {
      write_serial_cstr("Halting\r\n");
      halt();
      write_serial_cstr("Done Halting\r\n");
    }

    runtime_services->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, 0);
    return(EFI_SUCCESS);
}
