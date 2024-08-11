#ifndef MCP23X17_H
#define MCP23X17_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

// Pin Definitions
#define A0  0
#define A1  1
#define A2  2
#define A3  3
#define A4  4
#define A5  5
#define A6  6
#define A7  7
#define B0  8
#define B1  9
#define B2  10
#define B3  11
#define B4  12
#define B5  13
#define B6  14
#define B7  15

// MCP23017 specific definitions
#define MCP23017_ADDRESS    0x20
#define IO_CHANNELS         16  // Total number of IO channels
#define DATA_LENGTH         3   // Length of data for transactions
#define RW_TEST_LENGTH      128 // Length for read/write tests

// MCP23017 Register addresses
#define REG_IODIRA      0x00
#define REG_IODIRB      0x01
#define REG_IPOLA       0x02
#define REG_IPOLB       0x03
#define REG_GPINTENA    0x04
#define REG_GPINTENB    0x05
#define REG_DEFVALA     0x06
#define REG_DEFVALB     0x07
#define REG_INTCONA     0x08
#define REG_INTCONB     0x09
#define REG_IOCON       0x0A
#define REG_GPPUA       0x0C
#define REG_GPPUB       0x0D
#define REG_INTFA       0x0E
#define REG_INTFB       0x0F
#define REG_INTCAPA     0x10
#define REG_INTCAPB     0x11
#define REG_GPIOA       0x12
#define REG_GPIOB       0x13
#define REG_OLATA       0x14
#define REG_OLATB       0x15

// IOCON register bits
#define BIT_IOCON_INTPOL 1
#define BIT_IOCON_ODR    2
#define BIT_IOCON_HAEN   3
#define BIT_IOCON_DISSLW 4
#define BIT_IOCON_SEQOP  5
#define BIT_IOCON_MIRROR 6
#define BIT_IOCON_BANK   7

// Direction definitions
#define MCP_OUTPUT 0
#define MCP_INPUT  1

// Utility macros
#define BV(x) (1 << (x))

// Function prototypes
esp_err_t mcp_read(uint8_t *data, uint8_t len);
esp_err_t mcp_write(uint8_t *data, uint8_t len);
void read_io(void);
void set_mcp_io(uint8_t io, bool val);
bool get_mcp_io(uint8_t io);
void set_mcp_io_dir(uint8_t io, bool dir);
uint16_t get_mcp_values(void);
void printMcpData(void);
void printByte(void);
void mcp23x17_main(void);

#endif // MCP23X17_H
