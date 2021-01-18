#include "efi_util.h"
#include "strings.h"

void guid_to_hex_char16(EFI_GUID guid, uint16_t* chars) {
  byte_to_hex_char16((guid.Data1 >> 24) & 0xff, &chars[0]);
  byte_to_hex_char16((guid.Data1 >> 16) & 0xff, &chars[2]);
  byte_to_hex_char16((guid.Data1 >> 8)  & 0xff, &chars[4]);
  byte_to_hex_char16((guid.Data1 >> 0)  & 0xff, &chars[6]);
  chars[8] = L'-';
  byte_to_hex_char16((guid.Data2 >> 8) & 0xff, &chars[9]);
  byte_to_hex_char16((guid.Data2 >> 0) & 0xff, &chars[11]);
  chars[13] = L'-';
  byte_to_hex_char16((guid.Data3 >> 8) & 0xff, &chars[14]);
  byte_to_hex_char16((guid.Data3 >> 0) & 0xff, &chars[16]);
  chars[18] = L'-';
  byte_to_hex_char16(guid.Data4[0], &chars[19]);
  byte_to_hex_char16(guid.Data4[1], &chars[21]);
  byte_to_hex_char16(guid.Data4[2], &chars[23]);
  byte_to_hex_char16(guid.Data4[3], &chars[25]);
  byte_to_hex_char16(guid.Data4[4], &chars[27]);
  byte_to_hex_char16(guid.Data4[5], &chars[29]);
  byte_to_hex_char16(guid.Data4[6], &chars[31]);
  byte_to_hex_char16(guid.Data4[7], &chars[33]);
  chars[35] = 0;
}

uint16_t* const newline_char16 = L"\r\n";

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
