#include <stdint.h>
#include <string.h>

#include "nrf_gpio.h"
#include "app_error.h"
#include "nrf_drv_twi.h"

#define IMU_MAG_ADDR 0x1C
#define IMU_GYRO_ADDR 0x6a

static const nrf_drv_twi_t m_twi_master = NRF_DRV_TWI_INSTANCE(0);

ret_code_t i2c_write(uint8_t addr, uint8_t data)
{
    uint8_t buf[2];

    buf[0] = addr;
    buf[1] = data;

    return nrf_drv_twi_tx(&m_twi_master, IMU_MAG_ADDR, buf, 3, false);
}

ret_code_t i2c_read(uint8_t addr, uint8_t * pdata, size_t size)
{
    ret_code_t ret;

    do
    {
        ret = nrf_drv_twi_tx(&m_twi_master, IMU_MAG_ADDR, (uint8_t *)&addr, 1, 1);
        if (NRF_SUCCESS != ret)
            break;

        ret = nrf_drv_twi_rx(&m_twi_master, IMU_MAG_ADDR, pdata, size);
    } while (0);

    return ret;
}

ret_code_t twi_master_init(void)
{
    ret_code_t ret;
    const nrf_drv_twi_config_t config =
    {
        .scl                = TWI_SCL_M,
        .sda                = TWI_SDA_M,
        .frequency          = NRF_DRV_TWI_FREQ_400K,
        .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
        .clear_bus_init     = true
    };

    // Configure I2c Address straping pins
    nrf_gpio_cfg_output(GPIO_SDO_M);
    nrf_gpio_cfg_output(GPIO_SDO_AG);
    nrf_gpio_pin_clear(GPIO_SDO_M); 
    nrf_gpio_pin_clear(GPIO_SDO_AG); 

    ret = nrf_drv_twi_init(&m_twi_master, &config, NULL, NULL);
    if (NRF_SUCCESS == ret)
        nrf_drv_twi_enable(&m_twi_master);

    i2c_write(0x21, 0x60);
    i2c_write(0x22, 0x00);

    return ret;
}
