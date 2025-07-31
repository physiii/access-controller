#ifndef I2C_H
#define I2C_H

#include "esp_err.h"
#include "driver/i2c.h"

// I2C constants
#define I2C_MASTER_SCL_IO           13      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           14      /*!< GPIO number used for I2C master data */
#define I2C_MASTER_NUM              0       /*!< I2C master port number */
#define I2C_MASTER_FREQ_HZ          400000  /*!< I2C master clock frequency */

// Function declarations
esp_err_t i2c_master_init(void);
void i2c_main(void);
esp_err_t i2c_master_read_slave(uint8_t addr, i2c_port_t i2c_num, uint8_t *data_rd, size_t size);
esp_err_t i2c_master_write_slave(uint8_t addr, i2c_port_t i2c_num, uint8_t *data_wr, size_t size);

#endif // I2C_H 