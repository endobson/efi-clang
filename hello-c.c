#include "efi.h"
#include "efi_util.h"
#include "primitives.h"
#include "serial.h"
#include "strings.h"


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

GDTEntry gdt_entries[3];
GDTDescr gdt_descr;


// Static array to make printing debug messages easier
char buffer_array[4096];

EFI_STATUS efi_main(EFI_HANDLE ih, EFI_SYSTEM_TABLE* st)
{
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
        byte_to_hex_char16(char_acpi_table[0], &hex_sig[0]);
        byte_to_hex_char16(char_acpi_table[1], &hex_sig[2]);
        byte_to_hex_char16(char_acpi_table[2], &hex_sig[4]);
        byte_to_hex_char16(char_acpi_table[3], &hex_sig[6]);
        byte_to_hex_char16(char_acpi_table[4], &hex_sig[8]);
        byte_to_hex_char16(char_acpi_table[5], &hex_sig[10]);
        byte_to_hex_char16(char_acpi_table[6], &hex_sig[12]);
        byte_to_hex_char16(char_acpi_table[7], &hex_sig[14]);
        hex_sig[16] = 0;
        st->ConOut->OutputString(st->ConOut, hex_sig);
        st->ConOut->OutputString(st->ConOut, newline_char16);
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
        st->ConOut->OutputString(st->ConOut, newline_char16);
      }
    }


    if (acpi_table == 0) {
      st->StdErr->OutputString(st->StdErr, L"Unable to get exit boot services");
      st->StdErr->OutputString(st->StdErr, newline_char16);
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
      st->StdErr->OutputString(st->StdErr, newline_char16);
      return s;
    }
    s = st->BootServices->AllocatePool(EfiLoaderData, memory_map_size, (void **)&memory_map);
    if (s != EFI_SUCCESS) {
      st->StdErr->OutputString(st->StdErr, L"Unable to get allocate pool");
      st->StdErr->OutputString(st->StdErr, newline_char16);
      return s;
    }
    s = st->BootServices->GetMemoryMap(&memory_map_size, memory_map, &memory_map_key, &descriptor_size, &descriptor_version);
    if (s != EFI_SUCCESS) {
      st->StdErr->OutputString(st->StdErr, L"Unable to get memory map");
      st->StdErr->OutputString(st->StdErr, newline_char16);
      return s;
    }

    // Try to Exit UEFI
    s = st->BootServices->ExitBootServices(ih, memory_map_key);
    if (s != EFI_SUCCESS) {
      st->StdErr->OutputString(st->StdErr, L"Unable to get exit boot services");
      st->StdErr->OutputString(st->StdErr, newline_char16);
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
    //   byte_to_hex((xsdt->header.Length - sizeof(ACPISDTHeader)) / sizeof(ACPISDTHeader), &sig[0]);
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

    // GDTDescr uefi_gdt_descr;
    // store_gdt(&uefi_gdt_descr);
    // {
    //   writer = buffer;
    //   writer_add_cstr(&writer, "GDT Descriptor Limit: 0x");
    //   writer_add_hex16(&writer, uefi_gdt_descr.limit);
    //   writer_add_newline(&writer);
    //   writer_add_cstr(&writer, "GDT Descriptor Base: 0x");
    //   writer_add_hex64(&writer, uefi_gdt_descr.base_addr);
    //   writer_add_newline(&writer);
    //   writer_add_cstr(&writer, "GDT Bytes");
    //   writer_add_newline(&writer);
    //   for (int i = 0; i < 18; i++) {
    //     writer_add_hex32(&writer, ((uint32_t*) uefi_gdt_descr.base_addr)[i]);
    //     writer_add_newline(&writer);
    //   }

    //   writer_terminate(&writer);
    //   write_serial_cstr(buffer);
    // }


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

    st->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, 0);
    return(EFI_SUCCESS);
}
