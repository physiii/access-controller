#ifndef MCP23X17_H
#define MCP23X17_H

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// I2C constants and forward declarations from drivers/i2c.c
#define I2C_MASTER_NUM              0       /*!< I2C master port number */

// MCP23017 pin definitions
#define A0	0
#define A1	1
#define A2	2
#define A3	3
#define A4	4
#define A5	5
#define A6	6
#define A7	7

#define B0	8
#define B1	9
#define B2	10
#define B3	11
#define B4	12
#define B5	13
#define B6	14
#define B7	15

#define MCP23017_ADDRESS 0x20
#define IO_CHANNELS 16
#define DATA_LENGTH 3

// Register definitions
#define REG_IODIRA   0x00
#define REG_IODIRB   0x01
#define REG_IPOLA    0x02
#define REG_IPOLB    0x03
#define REG_GPINTENA 0x04
#define REG_GPINTENB 0x05
#define REG_DEFVALA  0x06
#define REG_DEFVALB  0x07
#define REG_INTCONA  0x08
#define REG_INTCONB  0x09
#define REG_IOCON    0x0A
#define REG_GPPUA    0x0C
#define REG_GPPUB    0x0D
#define REG_INTFA    0x0E
#define REG_INTFB    0x0F
#define REG_INTCAPA  0x10
#define REG_INTCAPB  0x11
#define REG_GPIOA    0x12
#define REG_GPIOB    0x13
#define REG_OLATA    0x14
#define REG_OLATB    0x15

#define MCP_OUTPUT 0
#define MCP_INPUT  1

// Function declarations
esp_err_t i2c_master_read_slave(uint8_t addr, i2c_port_t i2c_num, uint8_t *data_rd, size_t size);
esp_err_t i2c_master_write_slave(uint8_t addr, i2c_port_t i2c_num, uint8_t *data_wr, size_t size);

void read_io(void);
void set_mcp_io(uint8_t io, bool val);
bool get_mcp_io(uint8_t io);
void set_mcp_io_dir(uint8_t io, bool dir);
uint16_t get_mcp_values(void);
void printMcpData(void);
void printByte(void);
void mcp23x17_main(void);

#endif // MCP23X17_H 