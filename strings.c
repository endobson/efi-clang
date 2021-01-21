#include "strings.h"

uint16_t nibble_to_hex_char16(uint8_t v) {
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

char nibble_to_hex(uint8_t v) {
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

void byte_to_hex_char16(uint8_t v, uint16_t* chars) {
  chars[0] = nibble_to_hex_char16((v >> 4) & 0xF);
  chars[1] = nibble_to_hex_char16((v >> 0) & 0xF);
}

void byte_to_hex(uint8_t v, char* chars) {
  chars[0] = nibble_to_hex((v >> 4) & 0xF);
  chars[1] = nibble_to_hex((v >> 0) & 0xF);
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
  // Use unique string.
  char* newline = "Z\r\n";
  writer_add_cstr(writer, newline+1);
}

void writer_terminate(char** writer) {
  // Don't use "" string literal.
  // Hack to avoid string sharing in linker
  **writer = 0;
  (*writer)++;
}

void writer_add_hex8(char** writer, uint8_t v) {
  char buf[2];
  byte_to_hex(v, &buf[0]);
  writer_add_bytes(writer, buf, 2);
}
void writer_add_hex16(char** writer, uint16_t v) {
  char buf[4];
  byte_to_hex((v >> 8)  & 0xff, &buf[0]);
  byte_to_hex((v >> 0)  & 0xff, &buf[2]);
  writer_add_bytes(writer, buf, 4);
}
void writer_add_hex32(char** writer, uint32_t v) {
  char buf[8];
  byte_to_hex((v >> 24) & 0xff, &buf[0]);
  byte_to_hex((v >> 16) & 0xff, &buf[2]);
  byte_to_hex((v >> 8)  & 0xff, &buf[4]);
  byte_to_hex((v >> 0)  & 0xff, &buf[6]);
  writer_add_bytes(writer, buf, 8);
}
void writer_add_hex64(char** writer, uint64_t v) {
  char buf[16];
  byte_to_hex((v >> 56) & 0xff, &buf[0]);
  byte_to_hex((v >> 48) & 0xff, &buf[2]);
  byte_to_hex((v >> 40) & 0xff, &buf[4]);
  byte_to_hex((v >> 32) & 0xff, &buf[6]);
  byte_to_hex((v >> 24) & 0xff, &buf[8]);
  byte_to_hex((v >> 16) & 0xff, &buf[10]);
  byte_to_hex((v >> 8)  & 0xff, &buf[12]);
  byte_to_hex((v >> 0)  & 0xff, &buf[14]);
  writer_add_bytes(writer, buf, 16);
}

// Static array to make printing debug messages easier
char writer_buffer_array[4096];

char* const writer_buffer = (char* const) writer_buffer_array;

// Memset
void my_memset(uint8_t* ptr, uint8_t v, int amt) {
  for (int i = 0; i < amt; i++) {
    ptr[i] = v;
  }
}
