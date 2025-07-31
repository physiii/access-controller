// services/drivers/i2c.h

#ifndef I2C_H
#define I2C_H

#include "driver/i2c.h"

void i2c_master_init(void);
esp_err_t i2c_master_read_slave(i2c_port_t i2c_num, uint8_t i2c_addr, uint8_t* data_rd, size_t size);
esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t i2c_addr, uint8_t* data_wr, size_t size);

#endif // I2C_H 