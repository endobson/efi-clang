#include "acpi.h"
#include "descriptor_tables.h"
#include "efi.h"
#include "efi_util.h"
#include "primitives.h"
#include "serial.h"
#include "strings.h"

RSDPDescriptor* find_rsdp(EFI_SYSTEM_TABLE* st) {
  for (int i = 0; i < st->NumberOfTableEntries; i++) {
      EFI_CONFIGURATION_TABLE ct = st->ConfigurationTable[i];
      EFI_GUID acpi_table_guid = EFI_ACPI_TABLE_GUID;
      if (guid_equal(ct.VendorGuid, acpi_table_guid)) {
        return (RSDPDescriptor*) ct.VendorTable;
      }
  }
  return 0;
}

EFI_STATUS exit_boot_services(EFI_HANDLE ih, EFI_SYSTEM_TABLE* st) {
  UINTN memory_map_size = 0;
  EFI_MEMORY_DESCRIPTOR* memory_map;
  UINTN memory_map_key;
  UINTN descriptor_size;
  UINT32 descriptor_version;

  EFI_STATUS s;
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

void init_pic() {
  // Base port numbers for the Master/Slave PICs.
  uint8_t pic1 = 0x20;
  uint8_t pic2 = 0xA0;
  // Command and data port numbers
  uint8_t pic1_command = pic1 + 0;
  uint8_t pic1_data = pic1 + 1;
  uint8_t pic2_command = pic2 + 0;
  uint8_t pic2_data = pic2 + 1;

  uint8_t icw1_init = 0x10; // This is an initialization command
  uint8_t icw1_icw4 = 0x01; // This initialization uses command word 4
  // Start the initialization sequence (in cascade mode)
  outb(icw1_init | icw1_icw4, pic1_command);
  outb(icw1_init | icw1_icw4, pic2_command);

  // Set the PICs to use the entries in the IDT range [32, 47).
  uint8_t offset1 = 32;
  uint8_t offset2 = 40;
  outb(offset1, pic1_data);    // ICW2: Master PIC vector offset
  outb(offset2, pic2_data);    // ICW2: Slave PIC vector offset
  outb(0b00000100, pic1_data); // ICW3: tell Master PIC that there is a slave PIC at IRQ2
  outb(2, pic2_data);          // ICW3: tell Slave PIC its cascade identity

  // Set 8086 mode
  uint8_t icw4_8086 = 0x01;
  outb(icw4_8086, pic1_data);
  outb(icw4_8086, pic2_data);

  // Only enable some interrupts.
  // PIC 1, bit 2: Allow PIC2 through
  // PIC 1, bit 4: COM1 serial port
  uint8_t pic1_interrupts = (1 << 2) | (1 << 4);
  // PIC 2, bit 3: NIC
  uint8_t pic2_interrupts = (1 << 3);
  // Mask all interrupts that shouldn't be enabled.
  outb(~pic1_interrupts, pic1_data);
  outb(~pic2_interrupts, pic2_data);
}

void init_idt() {
  my_memset((uint8_t*) &idt_entries, 0, sizeof(IDTEntry) * 256);
  for (int i = 0; i < 256; i++) {
    uint64_t irq_addr;
    if (i == 36) {
      irq_addr = (uint64_t) irqfun_com1;
    } else if (i == 43) {
      irq_addr = (uint64_t) irqfun_nic;
    } else {
      irq_addr = (uint64_t) irqfun_default;
    }

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
  writer_add_cstr(&writer, "Status: 0x");
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
      writer_add_cstr(&writer, ": ");
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

void init_network() {
  uint64_t base_address = 0x00000000b0000000;
  int device_num = 2;

  PCIHeader0* header = (PCIHeader0*) (base_address + (device_num << 15));
  PCIHeader0* header2 = (PCIHeader0*) (base_address + (1 << 15));

  // print_device(header);
  // print_device(header2);

  int feature_mac = 5;
  uint16_t device_features_port = 0x00;
  uint16_t guest_features_port  = 0x04;
  uint16_t queue_address_port   = 0x08;
  uint16_t queue_size_port      = 0x0c;
  uint16_t queue_select_port    = 0x0e;
  uint16_t device_status_port   = 0x12;

  uint8_t device_acknowledged = 0x01;
  uint8_t device_driver       = 0x02;
  uint8_t device_features_ok  = 0x08;
  uint8_t device_driver_ok    = 0x04;

  uint16_t net_base_port = 0x6060;


  // Acknowledge the device.
  outb(device_acknowledged,
       net_base_port + device_status_port);
  // Tell the device that we know how to drive it.
  outb(device_acknowledged | device_driver,
       net_base_port + device_status_port);

  uint32_t device_features = inl(net_base_port + device_features_port);
  if (!(device_features & (1 << feature_mac))) {
    panic();
  }

  uint32_t guest_features = (1 << feature_mac);
  outl(guest_features, net_base_port + guest_features_port);

  // Tell the device that we are finalized on our feature decisions.
  outb(device_acknowledged | device_driver | device_features_ok,
       net_base_port + device_status_port);

  int8_t device_status = inb(net_base_port + device_status_port);
  if (device_status != (device_acknowledged | device_driver | device_features_ok)) {
    panic();
  }

  char* writer;
  for (uint16_t queue_num = 0; queue_num < 2; queue_num ++) {
    outw(queue_num, net_base_port + queue_select_port);
    uint16_t queue_size = inw(net_base_port + queue_size_port);
    if (queue_size != 0x100) {
      panic();
    }

    {
      writer = writer_buffer;
      writer_add_cstr(&writer, "Queue 0x");
      writer_add_hex16(&writer, queue_num);
      writer_add_cstr(&writer, ": ");
      writer_add_hex16(&writer, queue_size);
      writer_add_newline(&writer);

      writer_terminate(&writer);
      write_serial_cstr(writer_buffer);
    }
  }

  my_memset((uint8_t*) &net_send_queue, 0, sizeof(net_send_queue));
  my_memset((uint8_t*) &net_recv_queue, 0, sizeof(net_recv_queue));

  for (int i = 0; i < 256; i++) {
    net_recv_queue.buffers[i].address = (uint64_t) &net_recv_buffers[i];
    net_recv_queue.buffers[i].length  = 4096;
    net_recv_queue.buffers[i].flags   = 2; // Device writable
    net_recv_queue.buffers[i].next    = 0;

    net_recv_queue.available.ring[i] = i;
  }
  net_recv_queue.available.index = 256;


  // Tell the device about our queues
  outw(0, net_base_port + queue_select_port);

  outl(((uint64_t) &net_recv_queue) >> 12,
       net_base_port + queue_address_port);
  outw(1, net_base_port + queue_select_port);
  outl(((uint64_t) &net_send_queue) >> 12,
       net_base_port + queue_address_port);

  // Tell the device that we are ready!
  outb(device_acknowledged | device_driver | device_features_ok | device_driver_ok,
       net_base_port + device_status_port);
  device_status = inb(net_base_port + device_status_port);
  if (device_status != (device_acknowledged | device_driver | device_features_ok | device_driver_ok)) {
    panic();
  }


  if (0) {
    writer = writer_buffer;
    writer_add_cstr(&writer, "Send Queue Addr: ");
    writer_add_hex64(&writer, (uint64_t) &net_send_queue);
    writer_add_newline(&writer);
    writer_add_cstr(&writer, "Send Queue used Addr: ");
    writer_add_hex64(&writer, (uint64_t) &net_send_queue.used);
    writer_add_newline(&writer);

    writer_terminate(&writer);
    write_serial_cstr(writer_buffer);
  }

  {
    writer = writer_buffer;
    writer_add_cstr(&writer, "Devices Features: ");
    writer_add_hex32(&writer, device_features);
    writer_add_newline(&writer);
    writer_add_cstr(&writer, "Guest Features: ");
    writer_add_hex32(&writer, inl(net_base_port + guest_features_port));
    writer_add_newline(&writer);
    writer_add_cstr(&writer, "Device Status: ");
    writer_add_hex8(&writer, device_status);
    writer_add_newline(&writer);

    writer_terminate(&writer);
    write_serial_cstr(writer_buffer);
  }

}


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


typedef enum TaskState {
  TaskState_Runnable,
  TaskState_Blocked,
} TaskState;

typedef struct TaskDescriptor {
  void* stack_pointer;
  struct TaskDescriptor* next;
  TaskState state;
} __attribute__ ((packed)) TaskDescriptor;



TaskDescriptor* current_task = 0;
TaskDescriptor* all_tasks = 0;

TaskDescriptor root_task;

uint8_t serial_stack[8192];
TaskDescriptor serial_task;

uint8_t serial_stack2[8192];
TaskDescriptor serial_task2;


int yield(TaskState new_old_state) {
  TaskDescriptor* old_task = current_task;
  TaskDescriptor* new_task = old_task->next;

  while (new_task != old_task) {
    if (new_task->state == TaskState_Runnable) break;
    new_task = new_task->next;
  }

  int ret_val = 0;
  if (new_task != old_task) {
    ret_val = 1;
    old_task->state = new_old_state;
    current_task = new_task;
    switch_to_task(old_task, new_task);
  }

  return ret_val;
}


void mark_all_runnable() {
  disable_interrupts();
  TaskDescriptor* task = current_task;
  do {
    task->state = TaskState_Runnable;
    task = task->next;
  } while (task != current_task);

  enable_interrupts();
}



void serial_task_start() {
  enable_interrupts();

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

void wait_network_interrupt() {
  char* writer;

  uint16_t device_status_port   = 0x12;
  uint16_t isr_status_port   = 0x13;

  uint16_t net_base_port = 0x6060;

  uint8_t isr_status;
  disable_interrupts();
  do {
    isr_status = inb(net_base_port + isr_status_port);
    if (isr_status != 0) break;
    yield(TaskState_Blocked);
  } while (1);
  enable_interrupts();

  uint8_t device_status = inb(net_base_port + device_status_port);

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

}

void serial_task2_start() {
  enable_interrupts();

  while (1) {
    wait_network_interrupt();
  }
}



void init_scheduler() {
  // Initialize root_task
  root_task.stack_pointer = 0;
  root_task.next = &root_task;
  root_task.state = TaskState_Runnable;

  // Initialize the current task as the root_task;
  current_task = &root_task;

  {
    serial_task.stack_pointer = &serial_stack[8192];
    serial_task.next = current_task->next;
    current_task->next = &serial_task;

    uint64_t* u64_sp = (uint64_t*) serial_task.stack_pointer;

    *(--u64_sp) = (uint64_t) serial_task_start;
    for (int i = 0; i < 15; i++) {
      *(--u64_sp) = 0;
    }

    serial_task.stack_pointer = u64_sp;
  }

  {
    serial_task2.stack_pointer = &serial_stack2[8192];
    serial_task2.next = current_task->next;
    current_task->next = &serial_task2;

    uint64_t* u64_sp = (uint64_t*) serial_task2.stack_pointer;

    *(--u64_sp) = (uint64_t) serial_task2_start;
    for (int i = 0; i < 15; i++) {
      *(--u64_sp) = 0;
    }

    serial_task2.stack_pointer = u64_sp;
  }



}





EFI_STATUS efi_main(EFI_HANDLE ih, EFI_SYSTEM_TABLE* st)
{
    RSDPDescriptor* rsdp = find_rsdp(st);

    if (rsdp == 0) {
      st->StdErr->OutputString(st->StdErr, L"Unable to get RSDP acpi table");
      st->StdErr->OutputString(st->StdErr, newline_char16);
      return EFI_NOT_FOUND;
    }

    EFI_STATUS s = exit_boot_services(ih, st);
    if (s != EFI_SUCCESS) {
      return s;
    }
    // UEFI is now finished.
    // Start initializing sub systems.

    // check_acpi_tables(rsdp);

    // TODO Actually set up the GDT
    // initialize_gdt();

    init_idt();
    init_serial();
    init_pic();
    init_network();
    init_scheduler();

    enable_interrupts();

    // Run the main OS loop

    write_serial_cstr("\033c");
    write_serial_cstr("Welcome to Yaspl OS.\r\n");
    while (1) {
      disable_interrupts();
      while (yield(TaskState_Runnable)) {}
      enable_interrupts_and_halt();
    }

    // Shutdown
    st->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, 0);
    return(EFI_SUCCESS);
}
