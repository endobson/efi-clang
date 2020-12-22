#ifndef ACPI_H_
#define ACPI_H_

#include <stdint.h>

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


#endif // ACPI_H_
