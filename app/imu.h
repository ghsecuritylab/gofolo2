ret_code_t twi_master_init(void);
ret_code_t i2c_read(uint8_t addr, uint8_t * pdata, size_t size);
ret_code_t i2c_write(uint8_t addr, uint8_t data);
