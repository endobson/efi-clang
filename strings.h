#ifndef STRINGS_H_
#define STRINGS_H_

#include <stdint.h>

// Header file for string utilities.

// Writes the two hex characters to the buffer `chars`.
void byte_to_hex_char16(uint8_t v, uint16_t* chars);

// Writes the two hex characters to the buffer `chars`.
void byte_to_hex(uint8_t v, char* chars);

// Writer API

// Add the null terminated string to the writer.
void writer_add_cstr(char** writer, char* str);

// Add the specified bytes to the writer.
void writer_add_bytes(char** writer, char* bytes, int amt);

// Add a carriage return and line feet to the writer.
void writer_add_newline(char** writer);

// Add a null terminator to the writer.
void writer_terminate(char** writer);

// Add a hex number to the writer.
void writer_add_hex8(char** writer, uint8_t v);
void writer_add_hex16(char** writer, uint16_t v);
void writer_add_hex32(char** writer, uint32_t v);
void writer_add_hex64(char** writer, uint64_t v);

// Static buffer to make printing debug messages easier
extern char* const writer_buffer;

// Memset
void my_memset(uint8_t* ptr, uint8_t v, int amt);

#endif // STRINGS_H_
