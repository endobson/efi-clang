#ifndef EFI_H_
#define EFI_H_

#include <stdint.h>

typedef struct EFI_GUID {
    uint32_t  Data1;
    uint16_t  Data2;
    uint16_t  Data3;
    uint8_t   Data4[8];
} EFI_GUID;

typedef struct EFI_TABLE_HEADER {
    uint32_t  Signature;
    uint32_t  Revision;
    uint32_t  HeaderSize;
    uint32_t  CRC32;
    uint32_t  Reserved;
} EFI_TABLE_HEADER;


typedef uint64_t    EFI_STATUS;
typedef void        *EFI_EVENT;
typedef void        *EFI_HANDLE;
typedef void        *EFI_UNDEFINED_PTR;


struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef EFI_STATUS (*EFI_TEXT_STRING)(struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, uint16_t *String);


typedef struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
    EFI_UNDEFINED_PTR  Reset;
    EFI_TEXT_STRING    OutputString;
    EFI_UNDEFINED_PTR  TestString;
    EFI_UNDEFINED_PTR  QueryMode;
    EFI_UNDEFINED_PTR  SetMode;
    EFI_UNDEFINED_PTR  SetAttribute;
    EFI_UNDEFINED_PTR  ClearScreen;
    EFI_UNDEFINED_PTR  SetCursorPosition;
    EFI_UNDEFINED_PTR  EnableCursor;
    EFI_UNDEFINED_PTR  Mode;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;


typedef enum EFI_RESET_TYPE {
    EfiResetCold,
    EfiResetWarm,
    EfiResetShutdown,
    EfiResetPlatformSpecific
} EFI_RESET_TYPE;

typedef EFI_STATUS (*EFI_RESET_SYSTEM)(EFI_RESET_TYPE ResetType, EFI_STATUS ResetStatus, uint64_t DataSize, void *ResetData);

typedef struct EFI_RUNTIME_SERVICES {
    EFI_TABLE_HEADER                Hdr;
    EFI_UNDEFINED_PTR                    GetTime;
    EFI_UNDEFINED_PTR                    SetTime;
    EFI_UNDEFINED_PTR             GetWakeupTime;
    EFI_UNDEFINED_PTR             SetWakeupTime;
    EFI_UNDEFINED_PTR     SetVirtualAddressMap;
    EFI_UNDEFINED_PTR             ConvertPointer;
    EFI_UNDEFINED_PTR                GetVariable;
    EFI_UNDEFINED_PTR      GetNextVariableName;
    EFI_UNDEFINED_PTR                SetVariable;
    EFI_UNDEFINED_PTR    GetNextHighMonotonicCount;
    EFI_RESET_SYSTEM                ResetSystem;
    EFI_UNDEFINED_PTR              UpdateCapsule;
    EFI_UNDEFINED_PTR  QueryCapsuleCapabilities;
    EFI_UNDEFINED_PTR         QueryVariableInfo;
} EFI_RUNTIME_SERVICES;


typedef struct EFI_MEMORY_DESCRIPTOR {
    uint32_t Type;
    uint64_t PhysicalStart;
    uint64_t VirtualStart;
    uint64_t NumberOfPages;
    uint64_t Attribute;
} EFI_MEMORY_DESCRIPTOR;


#define EfiLoaderData               0x00000002

typedef EFI_STATUS (*EFI_GET_MEMORY_MAP)(uint64_t *MemoryMapSize, EFI_MEMORY_DESCRIPTOR *MemoryMap, uint64_t *MapKey, uint64_t *DescriptorSize, uint32_t *DescriptorVersion);
typedef EFI_STATUS (*EFI_ALLOCATE_POOL)(uint64_t PoolType, uint64_t Size, void **Buffer);
typedef EFI_STATUS (*EFI_EXIT_BOOT_SERVICES)(EFI_HANDLE ImageHandle, uint64_t MapKey);


typedef struct EFI_BOOT_SERVICES {
    EFI_TABLE_HEADER                            Hdr;
    EFI_UNDEFINED_PTR                               RaiseTPL;
    EFI_UNDEFINED_PTR                             RestoreTPL;
    EFI_UNDEFINED_PTR                          AllocatePages;
    EFI_UNDEFINED_PTR                              FreePages;
    EFI_GET_MEMORY_MAP                          GetMemoryMap;
    EFI_ALLOCATE_POOL                           AllocatePool;
    EFI_UNDEFINED_PTR                               FreePool;
    EFI_UNDEFINED_PTR                            CreateEvent;
    EFI_UNDEFINED_PTR                               SetTimer;
    EFI_UNDEFINED_PTR                          WaitForEvent;
    EFI_UNDEFINED_PTR                            SignalEvent;
    EFI_UNDEFINED_PTR                             CloseEvent;
    EFI_UNDEFINED_PTR                             CheckEvent;
    EFI_UNDEFINED_PTR              InstallProtocolInterface;
    EFI_UNDEFINED_PTR            ReinstallProtocolInterface;
    EFI_UNDEFINED_PTR            UninstallProtocolInterface;
    EFI_UNDEFINED_PTR                         HandleProtocol;
    EFI_UNDEFINED_PTR                                        *Reserved;
    EFI_UNDEFINED_PTR                RegisterProtocolNotify;
    EFI_UNDEFINED_PTR                           LocateHandle;
    EFI_UNDEFINED_PTR                      LocateDevicePath;
    EFI_UNDEFINED_PTR             InstallConfigurationTable;
    EFI_UNDEFINED_PTR                              LoadImage;
    EFI_UNDEFINED_PTR                             StartImage;
    EFI_UNDEFINED_PTR                                    Exit;
    EFI_UNDEFINED_PTR                            UnloadImage;
    EFI_EXIT_BOOT_SERVICES                      ExitBootServices;
    EFI_UNDEFINED_PTR                GetNextMonotonicCount;
    EFI_UNDEFINED_PTR                                   Stall;
    EFI_UNDEFINED_PTR                      SetWatchdogTimer;
    EFI_UNDEFINED_PTR                      ConnectController;
    EFI_UNDEFINED_PTR                   DisconnectController;
    EFI_UNDEFINED_PTR                           OpenProtocol;
    EFI_UNDEFINED_PTR                          CloseProtocol;
    EFI_UNDEFINED_PTR               OpenProtocolInformation;
    EFI_UNDEFINED_PTR                    ProtocolsPerHandle;
    EFI_UNDEFINED_PTR                    LocateHandleBuffer;
    EFI_UNDEFINED_PTR                         LocateProtocol;
    EFI_UNDEFINED_PTR    InstallMultipleProtocolInterfaces;
    EFI_UNDEFINED_PTR  UninstallMultipleProtocolInterfaces;
    EFI_UNDEFINED_PTR                         CalculateCrc32;
    EFI_UNDEFINED_PTR                                CopyMem;
    EFI_UNDEFINED_PTR                                 SetMem;
    EFI_UNDEFINED_PTR                         CreateEventEx;
} EFI_BOOT_SERVICES;

typedef struct EFI_SYSTEM_TABLE {
    EFI_TABLE_HEADER                Hdr;
    uint16_t                        *FirmwareVendor;
    uint32_t                        FirmwareRevision;
    EFI_HANDLE                      ConsoleInHandle;
    EFI_UNDEFINED_PTR               ConIn;
    EFI_HANDLE                      ConsoleOutHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
    EFI_HANDLE                      StandardErrorHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *StdErr;
    EFI_RUNTIME_SERVICES            *RuntimeServices;
    EFI_BOOT_SERVICES               *BootServices;
    uint64_t                        NumberOfTableEntries;
    EFI_UNDEFINED_PTR               ConfigurationTable;
} EFI_SYSTEM_TABLE;


#endif /* EFI_H_ */
