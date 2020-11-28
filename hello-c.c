
#include "efi.h"



CHAR16* hello_str = L"Hello, you slab of warm meat!\r\n";
CHAR16* newline = L"\r\n";


EFI_STATUS efi_main(EFI_HANDLE ih, EFI_SYSTEM_TABLE* st) 
{
    EFI_HANDLE image_handle = ih;
    EFI_SYSTEM_TABLE* system_table = st;
    CHAR16 chars[3];
    chars[0] = 0;
    chars[1] = 0;
    chars[2] = 0;
    while (1) {
      for (int i = 1; i <= 26; i++) {
        chars[0] = i + 64;
        for (int j = 1; j <= 26; j++) {
          chars[1] = j + 64;
          system_table->ConOut->OutputString(system_table->ConOut, chars);
          system_table->ConOut->OutputString(system_table->ConOut, newline);
        }
      }
    }

    return(EFI_SUCCESS);
}
