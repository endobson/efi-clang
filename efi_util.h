#ifndef EFI_UTIL_H_
#define EFI_UTIL_H_

#include "efi.h"

void guid_to_hex(EFI_GUID guid, CHAR16* chars);

int guid_equal(EFI_GUID g1, EFI_GUID g2);

extern CHAR16* newline_char16;

#endif // EFI_UTIL_H_
