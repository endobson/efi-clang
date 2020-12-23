#ifndef SERIAL_H_
#define SERIAL_H_

// Interface to the serial port.
// Currently hard codes COM1

// Write the null terminated string to the serial port.
void write_serial_cstr(char* str);

// Read a single character from the serial port.
char read_serial();

// Configures the serial port.
void init_serial();

#endif // SERIAL_H_