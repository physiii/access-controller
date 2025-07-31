#include "automation.h"
#include "driver/i2c.h"
#include "esp_timer.h"

// I2C constants and forward declarations from drivers/i2c.c
#define I2C_MASTER_NUM              0       /*!< I2C master port number */

esp_err_t i2c_master_read_slave(uint8_t addr, i2c_port_t i2c_num, uint8_t *data_rd, size_t size);
esp_err_t i2c_master_write_slave(uint8_t addr, i2c_port_t i2c_num, uint8_t *data_wr, size_t size);

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
#define IO_CHANNELS 16                  /*!< Data buffer length of test buffer */
#define DATA_LENGTH 3                  /*!< Data buffer length of test buffer */
#define RW_TEST_LENGTH 128               /*!< Data length for r/w test, [0,DATA_LENGTH] */

#define I2C_FREQ_HZ 1000000  // Max 1MHz for esp-idf, but device supports up to 1.7Mhz

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

#define BIT_IOCON_INTPOL 1
#define BIT_IOCON_ODR    2
#define BIT_IOCON_HAEN   3
#define BIT_IOCON_DISSLW 4
#define BIT_IOCON_SEQOP  5
#define BIT_IOCON_MIRROR 6
#define BIT_IOCON_BANK   7

#define MCP_OUTPUT 0
#define MCP_INPUT  1

#define BV(x) (1 << (x))

uint8_t data_wr[DATA_LENGTH];
uint8_t data_rd[DATA_LENGTH];
uint16_t MCP_IO_VALUES = 0;
uint16_t MCP_DIR = 0x0000; // default io to output

static esp_err_t mcp_read(uint8_t *data, uint8_t len)
{
	int ret = i2c_master_read_slave(MCP23017_ADDRESS, I2C_MASTER_NUM, data, len);

	if (ret == ESP_ERR_TIMEOUT) {
			ESP_LOGE(TAG, "I2C Timeout");
	} else if (ret == ESP_OK) {
			// printf("Received data");
	} else {
			ESP_LOGW(TAG, "%s: Master read slave error, IO not connected...\n", esp_err_to_name(ret));
	}

	return ret;
}

static esp_err_t mcp_write(uint8_t *data, uint8_t len)
{
	int ret = i2c_master_write_slave(MCP23017_ADDRESS, I2C_MASTER_NUM, data, len);

	if (ret == ESP_ERR_TIMEOUT) {
			ESP_LOGE(TAG, "I2C Timeout");
	} else if (ret == ESP_OK) {
			// printf("Master write to slave\n");
	} else {
			ESP_LOGW(TAG, "%s: Master write slave error, IO not connected...\n", esp_err_to_name(ret));
	}

	return ret;
}

void read_io()
{
	data_wr[0] = REG_GPIOA;
	esp_err_t write_result = mcp_write(data_wr, 1);
	if (write_result != ESP_OK) {
		// Don't attempt read if write failed
		ESP_LOGE(TAG, "MCP23017 write failed: %s", esp_err_to_name(write_result));
		return;
	}
	
	esp_err_t read_result = mcp_read(data_rd, 2);
	if (read_result != ESP_OK) {
		// Don't update values if read failed
		ESP_LOGE(TAG, "MCP23017 read failed: %s", esp_err_to_name(read_result));
		return;
	}

	uint16_t values = (data_rd[1] << 8) | data_rd[0];
	MCP_IO_VALUES = values;
}

void set_mcp_io(uint8_t io, bool val)
{
	if (val) {
		MCP_IO_VALUES |= 1 << io;
	} else {
		MCP_IO_VALUES &= ~(1 << io);
	}

	data_wr[0] = REG_GPIOA;
	data_wr[1] = MCP_IO_VALUES & 0x00ff;
	data_wr[2] = MCP_IO_VALUES >> 8;

	esp_err_t result = mcp_write(data_wr, 3);
	if (result != ESP_OK) {
		// Silently fail if I2C communication fails
		return;
	}
	// printf("set_mcp_io %d\tvalue %d\tdata [%u, %u, %u]\n", io, val, data_wr[0], data_wr[1], data_wr[2]);
}

bool get_mcp_io(uint8_t io)
{
	// printf("get_mcp_io: %u\n", MCP_IO_VALUES);
	int val = MCP_IO_VALUES >> io;

	return val &= 0x0001;
}

void set_mcp_io_dir(uint8_t io, bool dir)
{
	if (dir) {
		MCP_DIR |= 1 << io;
	} else {
		MCP_DIR &= ~(1 << io);
	}

	data_wr[0] = REG_IODIRA;
	data_wr[1] = MCP_DIR & 0x00ff;
	data_wr[2] = MCP_DIR >> 8;

	esp_err_t result = mcp_write(data_wr, 3);
	if (result != ESP_OK) {
		// Silently fail if I2C communication fails
		return;
	}

	// printf("set_mcp_io_dir(%u): io:%u\tdir:%d\n", MCP_DIR, io, dir);
	// printf("Set Direction: A:%d\tB:%d\n", data_wr[1], data_wr[2]);
}

uint16_t get_mcp_values ()
{
	return MCP_IO_VALUES;
}

void printMcpData()
{
	printf("MCP IO\t");
	for (int i=IO_CHANNELS-1; i >= 0; i--)
		printf("%d",  get_mcp_io(i));

	printf("\n");
}

void printByte()
{
	printf("MCP IO\t");
	for (int i=0; i < 8; i++)
		printf("%d ",  get_mcp_io(i));

	printf("\n");
}

static void mcp23x17_task(void* arg)
{
	ESP_LOGI(TAG, "MCP23017 task starting...");
	
	// Wait for I2C to be ready
	vTaskDelay(100 / portTICK_PERIOD_MS);
	
	// Test MCP23017 communication
	ESP_LOGI(TAG, "Testing MCP23017 communication at address 0x%02X", MCP23017_ADDRESS);
	data_wr[0] = REG_GPIOA;
	esp_err_t test_result = mcp_write(data_wr, 1);
	if (test_result == ESP_OK) {
		ESP_LOGI(TAG, "✅ MCP23017 communication successful!");
		
		// Initialize MCP23017 configuration
		ESP_LOGI(TAG, "Initializing MCP23017 registers...");
		
		// Set IOCON register (sequential mode)
		data_wr[0] = REG_IOCON;
		data_wr[1] = 0x00;  // Sequential mode, other defaults
		mcp_write(data_wr, 2);
		
		// Initialize all pins as inputs (default)
		data_wr[0] = REG_IODIRA;
		data_wr[1] = 0xFF;  // Port A all inputs
		data_wr[2] = 0xFF;  // Port B all inputs
		mcp_write(data_wr, 3);
		
		ESP_LOGI(TAG, "MCP23017 initialized successfully");
	} else {
		ESP_LOGE(TAG, "❌ MCP23017 communication failed: %s", esp_err_to_name(test_result));
		ESP_LOGE(TAG, "Check I2C wiring: SCL=%d, SDA=%d", 13, 14);
	}

	// Main loop
	while (1) {
		read_io();
    	vTaskDelay(SERVICE_LOOP / portTICK_PERIOD_MS);
	}
	vTaskDelete(NULL);
}

void mcp23x17_main(void)
{
	if (!USE_MCP23017) return;

	xTaskCreate(mcp23x17_task, "mcp23x17_task", 4096, NULL, 10, NULL); // Increased stack from 2048 to 4096
}
