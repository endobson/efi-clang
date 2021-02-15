#include "acpi.h"
#include "descriptor_tables.h"
#include "efi.h"
#include "primitives.h"
#include "serial.h"
#include "scheduler.h"
#include "strings.h"

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

typedef uint64_t UINTN;
typedef uint32_t UINT32;

#define EFI_SUCCESS                 0x0000000000000000
#define EFI_ERR                     0x8000000000000000
#define EFI_BUFFER_TOO_SMALL        (EFI_ERR | 0x0000000000000005)

EFI_STATUS exit_boot_services(EFI_HANDLE ih, EFI_SYSTEM_TABLE* st) {
  UINTN memory_map_size = 0;
  EFI_MEMORY_DESCRIPTOR* memory_map;
  UINTN memory_map_key;
  UINTN descriptor_size;
  UINT32 descriptor_version;

  EFI_STATUS s;
  s = st->BootServices->GetMemoryMap(&memory_map_size, memory_map, &memory_map_key, &descriptor_size, &descriptor_version);
  if (s != EFI_BUFFER_TOO_SMALL) {
    st->StdErr->OutputString(st->StdErr, L"Unable to get memory map size\r\n");
    return s;
  }
  s = st->BootServices->AllocatePool(EfiLoaderData, memory_map_size, (void **)&memory_map);
  if (s != EFI_SUCCESS) {
    st->StdErr->OutputString(st->StdErr, L"Unable to get allocate pool\r\n");
    return s;
  }
  s = st->BootServices->GetMemoryMap(&memory_map_size, memory_map, &memory_map_key, &descriptor_size, &descriptor_version);
  if (s != EFI_SUCCESS) {
    st->StdErr->OutputString(st->StdErr, L"Unable to get memory map\r\n");
    return s;
  }

  // Try to Exit UEFI
  s = st->BootServices->ExitBootServices(ih, memory_map_key);
  if (s != EFI_SUCCESS) {
    st->StdErr->OutputString(st->StdErr, L"Unable to get exit boot services\r\n");
    return s;
  }

  return EFI_SUCCESS;
}

void check_acpi_tables(RSDPDescriptor* rsdp) {
  char* writer;

  {
    writer = writer_buffer;
    writer_add_cstr(&writer, "RSDP signature: ");
    writer_add_bytes(&writer, rsdp->Signature, 8);
    writer_add_newline(&writer);
    writer_terminate(&writer);
    write_serial_cstr(writer_buffer);
  }

  XSDT* xsdt = (XSDT*) rsdp->XsdtAddress;
  {
    writer = writer_buffer;
    writer_add_cstr(&writer, "XSDT signature: ");
    writer_add_bytes(&writer, xsdt->header.Signature, 4);
    writer_add_newline(&writer);
    writer_terminate(&writer);
    write_serial_cstr(writer_buffer);
  }

  {
    int num_tables = (xsdt->header.Length - sizeof(ACPISDTHeader)) / sizeof(int64_t);
    for (int i = 0; i < num_tables; i++) {
      writer = writer_buffer;
      ACPISDTHeader* sdt = (ACPISDTHeader*) xsdt->other_tables[i];
      writer_add_cstr(&writer, "Table signature: ");
      writer_add_bytes(&writer, sdt->Signature, 4);
      writer_add_newline(&writer);
      writer_terminate(&writer);
      write_serial_cstr(writer_buffer);
    }
  }


  // TODO add looking at tables

  MCFG* mcfg = (MCFG*) xsdt->other_tables[3];
  {
    writer = writer_buffer;
    writer_add_cstr(&writer, "MCFG signature: ");
    writer_add_bytes(&writer, mcfg->header.Signature, 4);
    writer_add_newline(&writer);
    writer_add_cstr(&writer, "MCFG length: 0x");
    writer_add_hex32(&writer, mcfg->header.Length);
    writer_add_newline(&writer);
    writer_terminate(&writer);
    write_serial_cstr(writer_buffer);
  }

  ConfigSpaceAllocation* alloc = &mcfg->allocations[0];
  {
    writer = writer_buffer;
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
    write_serial_cstr(writer_buffer);
  }

  {
    for (int device_num = 0; device_num < 4; device_num++) {
      writer = writer_buffer;
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
      write_serial_cstr(writer_buffer);
    }
  }
}

// Currently unused as we can use the UEFI initialized GDT.
void initialize_gdt() {
  char* writer;

  // GDTDescr uefi_gdt_descr;
  // store_gdt(&uefi_gdt_descr);
  // {
  //   writer = writer_buffer;
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
  //   write_serial_cstr(writer_buffer);
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
    writer = writer_buffer;
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
    write_serial_cstr(writer_buffer);
  }

  // load_segments(0x08, 0x10);
}


typedef struct PCIHeader0 {
  uint16_t vendor_id;
  uint16_t device_id;
  uint16_t command;
  uint16_t status;
  uint8_t revison;
  uint8_t prog_if;
  uint8_t subclass;
  uint8_t class;
  uint8_t cache_line_size;
  uint8_t latency_timer;
  uint8_t header_type;
  uint8_t bist;
  uint32_t base_address[6];
  uint32_t cis_pointer;
  uint16_t subsystem_vendor_id;
  uint16_t subsystem_id;
  uint32_t expansion_rom_base_address;
  uint8_t capabilities_pointer;
  uint8_t reserved[7];
  uint8_t interrupt_line;
  uint8_t interrupt_pin;
  uint8_t min_grant;
  uint8_t max_latency;
} __attribute__ ((packed)) PCIHeader0;

void print_device(PCIHeader0* header) {
  char* writer;
  writer = writer_buffer;
  writer_add_cstr(&writer, "Vendor ID: 0x");
  writer_add_hex16(&writer, header->vendor_id);
  writer_add_newline(&writer);
  writer_add_cstr(&writer, "Device ID: 0x");
  writer_add_hex16(&writer, header->device_id);
  writer_add_newline(&writer);
  writer_add_cstr(&writer, "Command: 0x");
  writer_add_hex16(&writer, header->command);
  writer_add_newline(&writer);
  writer_add_cstr(&writer, "PCI Status: 0x");
  writer_add_hex16(&writer, header->status);
  writer_add_newline(&writer);
  writer_add_cstr(&writer, "Class: 0x");
  writer_add_hex8(&writer, header->class);
  writer_add_newline(&writer);
  writer_add_cstr(&writer, "Subclass: 0x");
  writer_add_hex8(&writer, header->subclass);
  writer_add_newline(&writer);
  writer_add_cstr(&writer, "Header Type: 0x");
  writer_add_hex8(&writer, header->header_type);
  writer_add_newline(&writer);
  writer_add_cstr(&writer, "Subsystem Vendor: 0x");
  writer_add_hex16(&writer, header->subsystem_vendor_id);
  writer_add_newline(&writer);
  writer_add_cstr(&writer, "Subsystem ID: 0x");
  writer_add_hex16(&writer, header->subsystem_id);
  writer_add_newline(&writer);
  for (int i = 0; i < 6; i++) {
    uint32_t bar = header->base_address[i];
    if (bar != 0) {
      writer_add_cstr(&writer, "Base Address ");
      writer_add_hex8(&writer, i);
      char* colon_str = "BA: ";
      writer_add_cstr(&writer, colon_str + 2);
      if (bar & 1) {
        writer_add_cstr(&writer, "I/0 @ ");
        writer_add_hex32(&writer, bar & 0xFFFFFFFC);
      } else if ((bar & 6) == 4)  {
        uint32_t bar2 = header->base_address[++i];
        uint64_t combined_bar = ((uint64_t) bar2 << 32) | (bar & 0xFFFFFFF0);
        writer_add_cstr(&writer, "64 bit Memory @ ");
        writer_add_hex64(&writer, combined_bar);
      } else {
        writer_add_cstr(&writer, "32 bit Memory @ ");
        writer_add_hex32(&writer, bar & 0xFFFFFFF0);
      }

      writer_add_newline(&writer);
    }
  }
  writer_add_cstr(&writer, "Interrupt Line: 0x");
  writer_add_hex8(&writer, header->interrupt_line);
  writer_add_newline(&writer);
  writer_add_cstr(&writer, "Interrupt Pin: 0x");
  writer_add_hex8(&writer, header->interrupt_pin);
  writer_add_newline(&writer);

  writer_terminate(&writer);
  write_serial_cstr(writer_buffer);
}

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


void print_network_status() {
  char* writer;

  uint16_t device_status_port   = 0x12;
  uint16_t isr_status_port   = 0x13;

  uint16_t net_base_port = 0x6060;



  uint8_t device_status = inb(net_base_port + device_status_port);
  uint8_t isr_status = inb(net_base_port + isr_status_port);

  {
    writer = writer_buffer;
    writer_add_cstr(&writer, "Device Status: 0x");
    writer_add_hex16(&writer, device_status);
    writer_add_newline(&writer);
    writer_add_cstr(&writer, "ISR Status: 0x");
    writer_add_hex16(&writer, isr_status);
    writer_add_newline(&writer);
    writer_add_cstr(&writer, "Network Recv flags: 0x");
    writer_add_hex16(&writer, net_recv_queue.used.flags);
    writer_add_newline(&writer);
    writer_add_cstr(&writer, "Network Recv used: 0x");
    writer_add_hex16(&writer, net_recv_queue.used.index);
    writer_add_newline(&writer);

    writer_terminate(&writer);
    write_serial_cstr(writer_buffer);
  }

  {
    writer = writer_buffer;
    writer_add_cstr(&writer, "Network Buffer:");
    writer_add_newline(&writer);

    uint32_t ring_index = net_recv_queue.used.index - 1;
    uint32_t buffer_num = net_recv_queue.used.ring[ring_index].index;
    writer_add_cstr(&writer, "Network Recv Buffer Num: 0x");
    writer_add_hex32(&writer, buffer_num);
    writer_add_newline(&writer);
    uint32_t buffer_length = net_recv_queue.used.ring[ring_index].length;
    writer_add_cstr(&writer, "Network Recv Buffer length: 0x");
    writer_add_hex32(&writer, buffer_length);
    writer_add_newline(&writer);
    writer_add_cstr(&writer, "Network Recv Buffer Addr: 0x");
    writer_add_hex32(&writer, net_recv_queue.buffers[buffer_num].address);
    writer_add_newline(&writer);

    uint8_t* buffer = (uint8_t*) &net_recv_buffers[buffer_num].buf;

    for (int i = 0; i < 64; i++) {
      writer_add_hex8(&writer, buffer[i]);
      if (i % 8 == 7) {
        writer_add_newline(&writer);
      }
    }

    writer_terminate(&writer);
    write_serial_cstr(writer_buffer);
  }

}

uint8_t serial_task_stack[8192];
TaskDescriptor serial_task;

uint8_t network_task_stack[8192];
TaskDescriptor network_task;

void serial_task_start() {
  while (1) {
    char c = read_serial();
    char* writer = writer_buffer;
    writer_add_cstr(&writer, "Console: ");
    writer_add_hex8(&writer, c);
    writer_add_newline(&writer);
    writer_terminate(&writer);
    write_serial_cstr(writer_buffer);
  }
}

void writer_add_hex_buffer(char** writer, uint8_t* buf, int length) {
  int i;
  for (i = 0; i < length; i++) {
    writer_add_hex8(writer, buf[i]);
    if (i % 8 == 7) {
      writer_add_newline(writer);
    }
  }
  if (i % 8 != 0) {
    writer_add_newline(writer);
  }
}

typedef struct BigEndianU16 {
  uint8_t byte0;
  uint8_t byte1;
} __attribute__ ((packed)) BigEndianU16;

typedef struct VirtioNetHeader {
  uint8_t header[10];
  uint8_t data[];
} __attribute__ ((packed)) VirtioNetHeader;

typedef struct EthernetHeader {
  uint8_t destination_mac[6];
  uint8_t source_mac[6];
  BigEndianU16 ethertype;
  uint8_t data[];
} __attribute__ ((packed)) EthernetHeader;

// This struct assumes that the sizing is consistent with IPv4/Ethernet
// ARP packets.
typedef struct ArpPacket {
  BigEndianU16 hardware_type;
  BigEndianU16 protocol_type;
  uint8_t hardware_length;
  uint8_t protocol_length;
  BigEndianU16 operation_type;
  uint8_t sender_hardware_address[6];
  uint8_t sender_protocol_address[4];
  uint8_t target_hardware_address[6];
  uint8_t target_protocol_address[4];
} __attribute__ ((packed)) ArpPacket;


typedef struct IpHeader {
  uint8_t version_header_length;
  uint8_t dcsp_ecn;
  BigEndianU16 total_length;
  BigEndianU16 identification;
  BigEndianU16 flags_fragment_offset;
  uint8_t time_to_live;
  uint8_t protocol;
  BigEndianU16 header_checksum;
  uint8_t source_ip[4];
  uint8_t destination_ip[4];
  uint8_t data[];
} __attribute__ ((packed)) IpHeader;

typedef struct UdpHeader {
  BigEndianU16 source_port;
  BigEndianU16 destination_port;
  BigEndianU16 length;
  BigEndianU16 checksum;
  uint8_t data[];
} __attribute__ ((packed)) UdpHeader;



uint16_t be_u16_to_le(BigEndianU16 be) {
  return be.byte0 << 8 | be.byte1 << 0;
}


void send_arp_packet() {
  write_serial_cstr("Sending Arp Packet\r\n");

  uint16_t net_base_port = 0x6060;
  uint16_t isr_status_port   = 0x13;
  uint16_t queue_notify_port    = 0x10;

  int index = net_send_queue.available.index;
  {
    int i = index % 256;



    VirtioNetHeader* virtio_header = (VirtioNetHeader*) &net_send_buffers[i];
    for (int j = 0; j < 10; j++) {
      virtio_header->header[i] = 0;
    }

    EthernetHeader* ethernet_header = (EthernetHeader*) &virtio_header->data;
    ethernet_header->destination_mac[0] = 0x52;
    ethernet_header->destination_mac[1] = 0x55;
    ethernet_header->destination_mac[2] = 0x0a;
    ethernet_header->destination_mac[3] = 0x00;
    ethernet_header->destination_mac[4] = 0x02;
    ethernet_header->destination_mac[5] = 0x02;
    ethernet_header->source_mac[0] = 0x52;
    ethernet_header->source_mac[1] = 0x54;
    ethernet_header->source_mac[2] = 0x00;
    ethernet_header->source_mac[3] = 0x12;
    ethernet_header->source_mac[4] = 0x34;
    ethernet_header->source_mac[5] = 0x56;
    ethernet_header->ethertype.byte0 = 0x08;
    ethernet_header->ethertype.byte1 = 0x06;

    ArpPacket* arp_packet = (ArpPacket*) &ethernet_header->data;
    arp_packet->hardware_type.byte0 = 0x00;
    arp_packet->hardware_type.byte1 = 0x01;
    arp_packet->protocol_type.byte0 = 0x08;
    arp_packet->protocol_type.byte1 = 0x00;
    arp_packet->hardware_length = 6;
    arp_packet->protocol_length = 4;
    arp_packet->operation_type.byte0 = 0x00;
    arp_packet->operation_type.byte1 = 0x02;
    arp_packet->sender_hardware_address[0] = 0x52;
    arp_packet->sender_hardware_address[1] = 0x54;
    arp_packet->sender_hardware_address[2] = 0x00;
    arp_packet->sender_hardware_address[3] = 0x12;
    arp_packet->sender_hardware_address[4] = 0x34;
    arp_packet->sender_hardware_address[5] = 0x56;

    arp_packet->sender_protocol_address[0] = 0x0a;
    arp_packet->sender_protocol_address[1] = 0x00;
    arp_packet->sender_protocol_address[2] = 0x02;
    arp_packet->sender_protocol_address[3] = 0x0f;

    arp_packet->target_hardware_address[0] = 0x52;
    arp_packet->target_hardware_address[1] = 0x55;
    arp_packet->target_hardware_address[2] = 0x0a;
    arp_packet->target_hardware_address[3] = 0x00;
    arp_packet->target_hardware_address[4] = 0x02;
    arp_packet->target_hardware_address[5] = 0x02;
    arp_packet->target_protocol_address[0] = 0x0a;
    arp_packet->target_protocol_address[1] = 0x00;
    arp_packet->target_protocol_address[2] = 0x02;
    arp_packet->target_protocol_address[3] = 0x02;

    net_send_queue.buffers[i].address = (uint64_t) &net_send_buffers[i];
    net_send_queue.buffers[i].length  =
      sizeof(ArpPacket) + sizeof(EthernetHeader) + sizeof(VirtioNetHeader);
    net_send_queue.buffers[i].flags   = 0;
    net_send_queue.buffers[i].next    = 0;

    net_send_queue.available.ring[i] = i;
    net_send_queue.available.index++;
  }

  // Tell the device that queue 1 (send) has a new buffer.
  outw(1, net_base_port + queue_notify_port);

  disable_interrupts();
  do {
    // Read the interrupt status register to clear the interrupt state.
    inb(net_base_port + isr_status_port);
    if (net_send_queue.used.index == index + 1) break;
    yield(TaskState_Blocked);
  } while (1);
  enable_interrupts();

}

void send_udp_packet(uint16_t dest_port) {
  write_serial_cstr("Sending UDP Packet\r\n");

  uint16_t net_base_port = 0x6060;
  uint16_t isr_status_port   = 0x13;
  uint16_t queue_notify_port    = 0x10;

  int index = net_send_queue.available.index;
  {
    int i = index % 256;



    VirtioNetHeader* virtio_header = (VirtioNetHeader*) &net_send_buffers[i];
    for (int j = 0; j < 10; j++) {
      virtio_header->header[i] = 0;
    }

    EthernetHeader* ethernet_header = (EthernetHeader*) &virtio_header->data;
    ethernet_header->destination_mac[0] = 0x52;
    ethernet_header->destination_mac[1] = 0x55;
    ethernet_header->destination_mac[2] = 0x0a;
    ethernet_header->destination_mac[3] = 0x00;
    ethernet_header->destination_mac[4] = 0x02;
    ethernet_header->destination_mac[5] = 0x02;
    ethernet_header->source_mac[0] = 0x52;
    ethernet_header->source_mac[1] = 0x54;
    ethernet_header->source_mac[2] = 0x00;
    ethernet_header->source_mac[3] = 0x12;
    ethernet_header->source_mac[4] = 0x34;
    ethernet_header->source_mac[5] = 0x56;
    ethernet_header->ethertype.byte0 = 0x08;
    ethernet_header->ethertype.byte1 = 0x00;

    char* response  = "abcdefghijklmnop\n";
    uint8_t response_length = 17;

    IpHeader* ip_header = (IpHeader*) &ethernet_header->data;
    ip_header->version_header_length = 0x45;
    ip_header->dcsp_ecn = 0x00;
    uint16_t total_length = sizeof(UdpHeader) + sizeof(IpHeader) + response_length;
    ip_header->total_length.byte0 = total_length >> 8 & 0xff;
    ip_header->total_length.byte1 = total_length >> 0 & 0xff;
    ip_header->flags_fragment_offset.byte0 = 0;
    ip_header->flags_fragment_offset.byte1 = 0;
    ip_header->time_to_live = 64;
    ip_header->protocol = 17; // UDP
    ip_header->header_checksum.byte0 = 0x62;
    ip_header->header_checksum.byte1 = 0xb0;
    ip_header->source_ip[0] = 10;
    ip_header->source_ip[1] = 0;
    ip_header->source_ip[2] = 2;
    ip_header->source_ip[3] = 15;
    ip_header->destination_ip[0] = 10;
    ip_header->destination_ip[1] = 0;
    ip_header->destination_ip[2] = 2;
    ip_header->destination_ip[3] = 2;


    UdpHeader* udp_header = (UdpHeader*) &ip_header->data;
    udp_header->source_port.byte0 = 0;
    udp_header->source_port.byte1 = 7;
    udp_header->destination_port.byte0 = dest_port >> 8 & 0xff;
    udp_header->destination_port.byte1 = dest_port >> 0 & 0xff;
    int udp_packet_length = sizeof(UdpHeader) + response_length;
    udp_header->length.byte0 = udp_packet_length >> 8 & 0xff;
    udp_header->length.byte1 = udp_packet_length >> 0 & 0xff;
    udp_header->checksum.byte0 = 0;
    udp_header->checksum.byte1 = 0;

    for (int j = 0; j < response_length; j++) {
      udp_header->data[j] = response[j];
    }


    net_send_queue.buffers[i].address = (uint64_t) &net_send_buffers[i];
    net_send_queue.buffers[i].length  =
      response_length + sizeof(UdpHeader) + sizeof(IpHeader) + sizeof(EthernetHeader) + sizeof(VirtioNetHeader);
    net_send_queue.buffers[i].flags   = 0;
    net_send_queue.buffers[i].next    = 0;

    net_send_queue.available.ring[i] = i;
    net_send_queue.available.index++;
  }

  // Tell the device that queue 1 (send) has a new buffer.
  outw(1, net_base_port + queue_notify_port);

  disable_interrupts();
  do {
    // Read the interrupt status register to clear the interrupt state.
    inb(net_base_port + isr_status_port);
    if (net_send_queue.used.index == index + 1) break;
    yield(TaskState_Blocked);
  } while (1);
  enable_interrupts();

}



void wait_network_interrupt() {
  char* writer;

  uint16_t device_status_port   = 0x12;
  uint16_t isr_status_port   = 0x13;
  uint16_t queue_notify_port    = 0x10;

  uint16_t net_base_port = 0x6060;

  int index = net_recv_queue.available.index;
  {
    int i = index % 256;

    net_recv_queue.buffers[i].address = (uint64_t) &net_recv_buffers[i];
    net_recv_queue.buffers[i].length  = 4096;
    net_recv_queue.buffers[i].flags   = 2; // Device writable
    net_recv_queue.buffers[i].next    = 0;

    net_recv_queue.available.ring[i] = i;
    net_recv_queue.available.index++;
  }

  // Tell the device that queue 0 (recv) has a new buffer.
  outw(0, net_base_port + queue_notify_port);

  disable_interrupts();
  do {
    // Read the interrupt status register to clear the interrupt state.
    inb(net_base_port + isr_status_port);
    if (net_recv_queue.used.index == index + 1) break;
    yield(TaskState_Blocked);
  } while (1);
  enable_interrupts();


  {
    int ring_i = index % 256;
    int buffer_i = net_recv_queue.used.ring[ring_i].index;
    if (buffer_i != ring_i) {
      panic();
    }
    int remaining_packet_length = net_recv_queue.used.ring[ring_i].length;
    void* raw_buffer = (void*) net_recv_queue.buffers[buffer_i].address;

    if (remaining_packet_length < sizeof(VirtioNetHeader)) {
      panic();
    }
    VirtioNetHeader* virtio_header = (VirtioNetHeader*) raw_buffer;
    remaining_packet_length -= sizeof(VirtioNetHeader);

    if (remaining_packet_length < sizeof(EthernetHeader)) {
      panic();
    }
    EthernetHeader* ethernet_header = (EthernetHeader*) &virtio_header->data;
    remaining_packet_length -= sizeof(EthernetHeader);

    if (ethernet_header->ethertype.byte0 == 0x08 &&
        ethernet_header->ethertype.byte1 == 0x06) {
      if (remaining_packet_length < sizeof(ArpPacket)) {
        panic();
      }
      ArpPacket* arp_packet = (ArpPacket*) &ethernet_header->data;
      remaining_packet_length -= sizeof(ArpPacket);

      if (remaining_packet_length != 0) {
        panic();
      }

      if (be_u16_to_le(arp_packet->hardware_type) != 1) {
        panic();
      }
      if (be_u16_to_le(arp_packet->protocol_type) != 0x0800) {
        panic();
      }
      if (arp_packet->hardware_length != 6) {
        panic();
      }
      if (arp_packet->protocol_length != 4) {
        panic();
      }

      send_arp_packet();
    } else if (ethernet_header->ethertype.byte0 == 0x08 &&
               ethernet_header->ethertype.byte1 == 0x00) {
      write_serial_cstr("IPv4 packet\r\n");
      if (remaining_packet_length < sizeof(IpHeader)) {
        panic();
      }
      IpHeader* ip_header = (IpHeader*) &ethernet_header->data;

      if (remaining_packet_length != be_u16_to_le(ip_header->total_length)) {
        panic();
      }

      int header_length = ip_header->version_header_length & 0x0F;
      if (header_length != 5) {
        panic();
      }
      remaining_packet_length -= sizeof(IpHeader);

      if (remaining_packet_length < sizeof(UdpHeader)) {
        panic();
      }
      UdpHeader* udp_header = (UdpHeader*) &ip_header->data;

      send_udp_packet(be_u16_to_le(udp_header->source_port));

      if (remaining_packet_length != be_u16_to_le(udp_header->length)) {
        panic();
      }
      remaining_packet_length -= sizeof(UdpHeader);

    } else {
      write_serial_cstr("Unknown packet type\r\n");
    }
  }
}

void network_task_start() {
  int num_network_packets;
  while (1) {
    wait_network_interrupt();
  }
}


void yos_serialTaskStart();
void yos_welcomeMessage();
void yos_exitBootServices(void *, void *);
void yos_initializeIdt();
void yos_initializeSerial();
void yos_initializePic();
void yos_initializeNetwork();
void yos_initializeScheduler();
void* call_sysv0(void* f);
void* call_sysv1(void* f, void* v1);
void* call_sysv2(void* f, void* v1, void* v2);

void add_initial_tasks() {
  add_task(&serial_task, &serial_task_stack[8192], yos_serialTaskStart);
  //add_task(&serial_task, &serial_task_stack[8192], serial_task_start);
  add_task(&network_task, &network_task_stack[8192], network_task_start);
}

EFI_STATUS efi_main(EFI_HANDLE ih, EFI_SYSTEM_TABLE* st)
{
    //RSDPDescriptor* rsdp = find_rsdp(st);

    EFI_STATUS s = (EFI_STATUS) call_sysv2(yos_exitBootServices, ih, st);
    if (s != EFI_SUCCESS) {
      return s;
    }
    // UEFI is now finished.
    // Start initializing sub systems.

    // check_acpi_tables(rsdp);

    // TODO Actually set up the GDT
    // initialize_gdt();

    call_sysv0(yos_initializeIdt);
    call_sysv0(yos_initializeSerial);
    call_sysv0(yos_initializePic);
    if (((uint64_t) call_sysv0(yos_initializeNetwork)) != 0) {
      write_serial_cstr("Network initialization failed\r\n");
      panic();
    }

    call_sysv0(yos_initializeScheduler);

    // init_scheduler();

    add_initial_tasks();

    enable_interrupts();

    // Print hello message.
    call_sysv0(yos_welcomeMessage);

    // Run the main OS loop
    run_scheduler_loop();

    // Shutdown
    st->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, 0);
    return(EFI_SUCCESS);
}
