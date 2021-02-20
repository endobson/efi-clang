#include "acpi.h"
#include "descriptor_tables.h"
#include "efi.h"
#include "primitives.h"
#include "serial.h"
#include "scheduler.h"
#include "strings.h"

void* call_sysv0(void* f);
void* call_sysv1(void* f, void* v1);
void* call_sysv2(void* f, void* v1, void* v2);
void* yos_sendUdpPacket(void*);
void* yos_sendArpPacket();
void* yos_waitNetworkInterrupt();

// RSDPDescriptor* find_rsdp(EFI_SYSTEM_TABLE* st) {
//   for (int i = 0; i < st->NumberOfTableEntries; i++) {
//       EFI_CONFIGURATION_TABLE ct = st->ConfigurationTable[i];
//       EFI_GUID acpi_table_guid = EFI_ACPI_TABLE_GUID;
//       if (guid_equal(ct.VendorGuid, acpi_table_guid)) {
//         return (RSDPDescriptor*) ct.VendorTable;
//       }
//   }
//   return 0;
// }
//

// typedef uint64_t UINTN;
// typedef uint32_t UINT32;

// #define EFI_SUCCESS                 0x0000000000000000
// #define EFI_ERR                     0x8000000000000000
// #define EFI_BUFFER_TOO_SMALL        (EFI_ERR | 0x0000000000000005)

// EFI_STATUS exit_boot_services(EFI_HANDLE ih, EFI_SYSTEM_TABLE* st) {
//   UINTN memory_map_size = 0;
//   EFI_MEMORY_DESCRIPTOR* memory_map;
//   UINTN memory_map_key;
//   UINTN descriptor_size;
//   UINT32 descriptor_version;
// 
//   EFI_STATUS s;
//   s = st->BootServices->GetMemoryMap(&memory_map_size, memory_map, &memory_map_key, &descriptor_size, &descriptor_version);
//   if (s != EFI_BUFFER_TOO_SMALL) {
//     st->StdErr->OutputString(st->StdErr, L"Unable to get memory map size\r\n");
//     return s;
//   }
//   s = st->BootServices->AllocatePool(EfiLoaderData, memory_map_size, (void **)&memory_map);
//   if (s != EFI_SUCCESS) {
//     st->StdErr->OutputString(st->StdErr, L"Unable to get allocate pool\r\n");
//     return s;
//   }
//   s = st->BootServices->GetMemoryMap(&memory_map_size, memory_map, &memory_map_key, &descriptor_size, &descriptor_version);
//   if (s != EFI_SUCCESS) {
//     st->StdErr->OutputString(st->StdErr, L"Unable to get memory map\r\n");
//     return s;
//   }
// 
//   // Try to Exit UEFI
//   s = st->BootServices->ExitBootServices(ih, memory_map_key);
//   if (s != EFI_SUCCESS) {
//     st->StdErr->OutputString(st->StdErr, L"Unable to get exit boot services\r\n");
//     return s;
//   }
// 
//   return EFI_SUCCESS;
// }

// void check_acpi_tables(RSDPDescriptor* rsdp) {
//   char* writer;
// 
//   {
//     writer = writer_buffer;
//     writer_add_cstr(&writer, "RSDP signature: ");
//     writer_add_bytes(&writer, rsdp->Signature, 8);
//     writer_add_newline(&writer);
//     writer_terminate(&writer);
//     write_serial_cstr(writer_buffer);
//   }
// 
//   XSDT* xsdt = (XSDT*) rsdp->XsdtAddress;
//   {
//     writer = writer_buffer;
//     writer_add_cstr(&writer, "XSDT signature: ");
//     writer_add_bytes(&writer, xsdt->header.Signature, 4);
//     writer_add_newline(&writer);
//     writer_terminate(&writer);
//     write_serial_cstr(writer_buffer);
//   }
// 
//   {
//     int num_tables = (xsdt->header.Length - sizeof(ACPISDTHeader)) / sizeof(int64_t);
//     for (int i = 0; i < num_tables; i++) {
//       writer = writer_buffer;
//       ACPISDTHeader* sdt = (ACPISDTHeader*) xsdt->other_tables[i];
//       writer_add_cstr(&writer, "Table signature: ");
//       writer_add_bytes(&writer, sdt->Signature, 4);
//       writer_add_newline(&writer);
//       writer_terminate(&writer);
//       write_serial_cstr(writer_buffer);
//     }
//   }
// 
// 
//   // TODO add looking at tables
// 
//   MCFG* mcfg = (MCFG*) xsdt->other_tables[3];
//   {
//     writer = writer_buffer;
//     writer_add_cstr(&writer, "MCFG signature: ");
//     writer_add_bytes(&writer, mcfg->header.Signature, 4);
//     writer_add_newline(&writer);
//     writer_add_cstr(&writer, "MCFG length: 0x");
//     writer_add_hex32(&writer, mcfg->header.Length);
//     writer_add_newline(&writer);
//     writer_terminate(&writer);
//     write_serial_cstr(writer_buffer);
//   }
// 
//   ConfigSpaceAllocation* alloc = &mcfg->allocations[0];
//   {
//     writer = writer_buffer;
//     writer_add_cstr(&writer, "Allocation BaseAddress: ");
//     writer_add_hex64(&writer, alloc->base_address);
//     writer_add_newline(&writer);
//     writer_add_cstr(&writer, "Allocation SegmentGroup: ");
//     writer_add_hex16(&writer, alloc->group_number);
//     writer_add_newline(&writer);
//     writer_add_cstr(&writer, "Allocation Start Bus: ");
//     writer_add_hex8(&writer, alloc->start_bus_number);
//     writer_add_newline(&writer);
//     writer_add_cstr(&writer, "Allocation End Bus: ");
//     writer_add_hex8(&writer, alloc->end_bus_number);
//     writer_add_newline(&writer);
//     writer_terminate(&writer);
//     write_serial_cstr(writer_buffer);
//   }
// 
//   {
//     for (int device_num = 0; device_num < 4; device_num++) {
//       writer = writer_buffer;
//       uint32_t* ptr = (uint32_t*) (alloc->base_address + (device_num << 15));
//       writer_add_cstr(&writer, "Device: 0x");
//       writer_add_hex8(&writer, device_num);
//       writer_add_newline(&writer);
//       writer_add_cstr(&writer, "At Address: 0x");
//       writer_add_hex64(&writer, (uint64_t) ptr);
//       writer_add_newline(&writer);
//       writer_add_hex32(&writer, ptr[0]);
//       writer_add_newline(&writer);
//       writer_add_hex32(&writer, ptr[1]);
//       writer_add_newline(&writer);
//       writer_add_hex32(&writer, ptr[2]);
//       writer_add_newline(&writer);
//       writer_add_hex32(&writer, ptr[3]);
//       writer_add_newline(&writer);
//       writer_add_hex32(&writer, ptr[4]);
//       writer_add_newline(&writer);
//       writer_add_hex32(&writer, ptr[5]);
//       writer_add_newline(&writer);
//       writer_add_hex32(&writer, ptr[6]);
//       writer_add_newline(&writer);
//       writer_add_hex32(&writer, ptr[7]);
//       writer_add_newline(&writer);
//       writer_terminate(&writer);
//       write_serial_cstr(writer_buffer);
//     }
//   }
// }

// Currently unused as we can use the UEFI initialized GDT.
// void initialize_gdt() {
//   char* writer;
// 
//   // GDTDescr uefi_gdt_descr;
//   // store_gdt(&uefi_gdt_descr);
//   // {
//   //   writer = writer_buffer;
//   //   writer_add_cstr(&writer, "GDT Descriptor Limit: 0x");
//   //   writer_add_hex16(&writer, uefi_gdt_descr.limit);
//   //   writer_add_newline(&writer);
//   //   writer_add_cstr(&writer, "GDT Descriptor Base: 0x");
//   //   writer_add_hex64(&writer, uefi_gdt_descr.base_addr);
//   //   writer_add_newline(&writer);
//   //   writer_add_cstr(&writer, "GDT Bytes");
//   //   writer_add_newline(&writer);
//   //   for (int i = 0; i < 18; i++) {
//   //     writer_add_hex32(&writer, ((uint32_t*) uefi_gdt_descr.base_addr)[i]);
//   //     writer_add_newline(&writer);
//   //   }
// 
//   //   writer_terminate(&writer);
//   //   write_serial_cstr(writer_buffer);
//   // }
// 
// 
//   my_memset((uint8_t*) &gdt_entries, 0, sizeof(GDTEntry) * 3);
//   {
//     // Set low bits of limit
//     gdt_entries[1].limit_1 = 0xffff;
// 
//     gdt_entries[1].flags1 = 0x9a;
// 
//     // Set top 4 bits of limit;
//     gdt_entries[1].flags2 |= 0x0f;
//     // Set 64 bit flag on code segment
//     gdt_entries[1].flags2 |= 0x20;
//     // Increase granularity for limit
//     gdt_entries[1].flags2 |= 0x80;
//   }
//   {
//     // Set low bits of limit
//     gdt_entries[1].limit_1 = 0xffff;
// 
//     gdt_entries[2].flags1 = 0x92;
// 
//     // Set top 4 bits of limit;
//     gdt_entries[1].flags2 |= 0x0f;
//     // Increase granularity for limit
//     gdt_entries[1].flags2 |= 0x80;
//   }
// 
//   gdt_descr.limit = sizeof(GDTEntry) * 3 - 1;
//   gdt_descr.base_addr = (uint64_t) &gdt_entries;
// 
//   // load_gdt(&gdt_descr);
// 
//   {
//     writer = writer_buffer;
//     writer_add_cstr(&writer, "My GDT Descriptor Limit: 0x");
//     writer_add_hex16(&writer, gdt_descr.limit);
//     writer_add_newline(&writer);
//     writer_add_cstr(&writer, "My GDT Descriptor Base: 0x");
//     writer_add_hex64(&writer, gdt_descr.base_addr);
//     writer_add_newline(&writer);
//     writer_add_cstr(&writer, "My GDT Bytes");
//     writer_add_newline(&writer);
//     for (int i = 0; i < 6; i++) {
//       writer_add_hex32(&writer, ((uint32_t*) gdt_descr.base_addr)[i]);
//       writer_add_newline(&writer);
//     }
// 
//     writer_terminate(&writer);
//     write_serial_cstr(writer_buffer);
//   }
// 
//   // load_segments(0x08, 0x10);
// }


// typedef struct PCIHeader0 {
//   uint16_t vendor_id;
//   uint16_t device_id;
//   uint16_t command;
//   uint16_t status;
//   uint8_t revison;
//   uint8_t prog_if;
//   uint8_t subclass;
//   uint8_t class;
//   uint8_t cache_line_size;
//   uint8_t latency_timer;
//   uint8_t header_type;
//   uint8_t bist;
//   uint32_t base_address[6];
//   uint32_t cis_pointer;
//   uint16_t subsystem_vendor_id;
//   uint16_t subsystem_id;
//   uint32_t expansion_rom_base_address;
//   uint8_t capabilities_pointer;
//   uint8_t reserved[7];
//   uint8_t interrupt_line;
//   uint8_t interrupt_pin;
//   uint8_t min_grant;
//   uint8_t max_latency;
// } __attribute__ ((packed)) PCIHeader0;

// void print_device(PCIHeader0* header) {
//   char* writer;
//   writer = writer_buffer;
//   writer_add_cstr(&writer, "Vendor ID: 0x");
//   writer_add_hex16(&writer, header->vendor_id);
//   writer_add_newline(&writer);
//   writer_add_cstr(&writer, "Device ID: 0x");
//   writer_add_hex16(&writer, header->device_id);
//   writer_add_newline(&writer);
//   writer_add_cstr(&writer, "Command: 0x");
//   writer_add_hex16(&writer, header->command);
//   writer_add_newline(&writer);
//   writer_add_cstr(&writer, "PCI Status: 0x");
//   writer_add_hex16(&writer, header->status);
//   writer_add_newline(&writer);
//   writer_add_cstr(&writer, "Class: 0x");
//   writer_add_hex8(&writer, header->class);
//   writer_add_newline(&writer);
//   writer_add_cstr(&writer, "Subclass: 0x");
//   writer_add_hex8(&writer, header->subclass);
//   writer_add_newline(&writer);
//   writer_add_cstr(&writer, "Header Type: 0x");
//   writer_add_hex8(&writer, header->header_type);
//   writer_add_newline(&writer);
//   writer_add_cstr(&writer, "Subsystem Vendor: 0x");
//   writer_add_hex16(&writer, header->subsystem_vendor_id);
//   writer_add_newline(&writer);
//   writer_add_cstr(&writer, "Subsystem ID: 0x");
//   writer_add_hex16(&writer, header->subsystem_id);
//   writer_add_newline(&writer);
//   for (int i = 0; i < 6; i++) {
//     uint32_t bar = header->base_address[i];
//     if (bar != 0) {
//       writer_add_cstr(&writer, "Base Address ");
//       writer_add_hex8(&writer, i);
//       char* colon_str = "BA: ";
//       writer_add_cstr(&writer, colon_str + 2);
//       if (bar & 1) {
//         writer_add_cstr(&writer, "I/0 @ ");
//         writer_add_hex32(&writer, bar & 0xFFFFFFFC);
//       } else if ((bar & 6) == 4)  {
//         uint32_t bar2 = header->base_address[++i];
//         uint64_t combined_bar = ((uint64_t) bar2 << 32) | (bar & 0xFFFFFFF0);
//         writer_add_cstr(&writer, "64 bit Memory @ ");
//         writer_add_hex64(&writer, combined_bar);
//       } else {
//         writer_add_cstr(&writer, "32 bit Memory @ ");
//         writer_add_hex32(&writer, bar & 0xFFFFFFF0);
//       }
// 
//       writer_add_newline(&writer);
//     }
//   }
//   writer_add_cstr(&writer, "Interrupt Line: 0x");
//   writer_add_hex8(&writer, header->interrupt_line);
//   writer_add_newline(&writer);
//   writer_add_cstr(&writer, "Interrupt Pin: 0x");
//   writer_add_hex8(&writer, header->interrupt_pin);
//   writer_add_newline(&writer);
// 
//   writer_terminate(&writer);
//   write_serial_cstr(writer_buffer);
// }

typedef struct VirtioQueue256 {
  struct {
    uint64_t address;
    uint32_t length;
    uint16_t flags;
    uint16_t next;
  } buffers[256];

  struct {
    uint16_t flags;
    uint16_t index;
    uint16_t ring[256];
    uint16_t event_index;
  } available;

  struct {
    uint16_t flags;
    uint16_t index;
    struct {
      uint32_t index;
      uint32_t length;
    } ring[256];
    uint16_t avail_event;
  } used __attribute__ ((aligned(4096)));
} __attribute__ ((packed, aligned(4096))) VirtioQueue256;

VirtioQueue256 net_send_queue;
VirtioQueue256 net_recv_queue;

typedef struct VirtioBuffer {
  uint8_t buf[4096];
} __attribute__ ((packed, aligned(4096))) VirtioBuffer;

VirtioBuffer net_send_buffers[256];
VirtioBuffer net_recv_buffers[256];

uint8_t serial_task_stack[8192];
uint8_t network_task_stack[8192];




void network_task_start() {
  int num_network_packets;
  while (1) {
    call_sysv0(yos_waitNetworkInterrupt);
  }
}


