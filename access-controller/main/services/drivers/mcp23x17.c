
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

#define MCP23X17_ADDR 0x20
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

static xQueueHandle mcp_evt_queue = NULL;

static void IRAM_ATTR mcp_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(mcp_evt_queue, &gpio_num, NULL);
}

static void mcp_int_task(void* arg)
{
    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(mcp_evt_queue, &io_num, portMAX_DELAY)) {
            printf("GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));
        }
    }
}

static esp_err_t mcp_write(uint8_t *data, uint8_t len)
{
	int ret = i2c_master_write_slave(MCP23X17_ADDR, I2C_MASTER_NUM, data, len);

	if (ret == ESP_ERR_TIMEOUT) {
			ESP_LOGE(TAG, "I2C Timeout");
	} else if (ret == ESP_OK) {
			// printf("Master write to slave\n");
	} else {
			ESP_LOGW(TAG, "%s: Master write slave error, IO not connected...\n", esp_err_to_name(ret));
	}

	return ret;
}

static esp_err_t mcp_read(uint8_t *data, uint8_t len)
{
	int ret = i2c_master_read_slave(MCP23X17_ADDR, I2C_MASTER_NUM, data, len);

	if (ret == ESP_ERR_TIMEOUT) {
			ESP_LOGE(TAG, "I2C Timeout");
	} else if (ret == ESP_OK) {
			// printf("Received data");
	} else {
			ESP_LOGW(TAG, "%s: Master read slave error, IO not connected...\n", esp_err_to_name(ret));
	}

	return ret;
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

	mcp_write(data_wr, 3);
}

bool get_mcp_io(uint8_t io)
{
	int val = MCP_IO_VALUES >> io;

	return val &= 0x0001;
}

void set_mcp_io_dir(uint16_t io, bool dir)
{
	uint16_t shifted = dir << io;
	MCP_DIR = MCP_DIR | shifted;

	data_wr[0] = REG_IODIRA;
	data_wr[1] = MCP_DIR & 0x00ff;
	data_wr[2] = MCP_DIR >> 8;

	mcp_write(data_wr, 3);
	// printf("Set Direction: A:%d\tB:%d\n", data_wr[1], data_wr[2]);
}

uint16_t get_mcp_values ()
{
	return MCP_IO_VALUES;
}

void read_io()
{
		data_wr[0] = REG_GPIOA;
		mcp_write(data_wr, 1);
		mcp_read(data_rd, 2);

		uint16_t values = (data_rd[1] << 8) | data_rd[0];
		MCP_IO_VALUES = (MCP_IO_VALUES & ~MCP_DIR) | (values & MCP_DIR);

		// printf("MCP_DIR %d\tvalues %d\tMCP_IO_VALUES %d\n", MCP_DIR, values, MCP_IO_VALUES);
}

void printMcpData()
{
	printf("MCP IO\t");
	for (int i=0; i < IO_CHANNELS; i++)
		printf("%d ",  get_mcp_io(i));

	printf("\n");

	// printf("HVAC IO\t\t%d\n", io_values);
}

static void mcp23x17_task(void* arg)
{
	int ret;
	uint32_t task_idx = (uint32_t)arg;
	uint8_t *data = (uint8_t *)malloc(DATA_LENGTH);
	uint8_t sensor_data_h, sensor_data_l;
	int cnt = 0;
	bool on = true;

	set_mcp_io_dir(A0, MCP_OUTPUT); // initialize io direction
	while (1) {
			read_io();
			vTaskDelay(SERVICE_LOOP_SHORT / portTICK_RATE_MS);
	}
	vTaskDelete(NULL);
}

void mcp23x17_main(void)
{
	mcp_evt_queue = xQueueCreate(10, sizeof(uint32_t));
  xTaskCreate(mcp_int_task, "mcp_int_task", 2048, NULL, 10, NULL);
	xTaskCreate(mcp23x17_task, "mcp23x17_task", 2048, NULL, 10, NULL);

  gpio_isr_handler_add(MCP_INTA_GPIO, mcp_isr_handler, (void*) MCP_INTA_GPIO);
	gpio_isr_handler_add(MCP_INTB_GPIO, mcp_isr_handler, (void*) MCP_INTB_GPIO);
}
