#include <stdint.h>
#include <string.h>
#include <math.h>

#include "nrf_gpio.h"
#include "app_error.h"
#include "nrf_drv_twi.h"
#include "lsm9ds1.h"
#include "proto.h"

#define ENV_OFFSET 170
#define M_PI 3.14159265358979323846264338327950288

extern nav_t nav;
int failed = 0;
static const nrf_drv_twi_t m_twi_master = NRF_DRV_TWI_INSTANCE(0);

static ret_code_t i2c_write(uint8_t dev, uint8_t addr, uint8_t data)
{
    ret_code_t ret;
    uint8_t buf[2];

    buf[0] = addr;
    buf[1] = data;

    ret = nrf_drv_twi_tx(&m_twi_master, dev, buf, 3, false);
    if (NRF_SUCCESS != ret)
        failed += 1;

    return ret;
}

static ret_code_t i2c_read_block(uint8_t dev, uint8_t addr, size_t size, uint8_t * pdata)
{
    ret_code_t ret;

    do
    {
        ret = nrf_drv_twi_tx(&m_twi_master, dev, (uint8_t *)&addr, 1, 1);
        if (NRF_SUCCESS != ret)
            break;

        ret = nrf_drv_twi_rx(&m_twi_master, dev, pdata, size);
    } while (0);

    if (NRF_SUCCESS != ret)
        failed += 1;

    return ret;
}

static void readACC(int16_t a[])
{
	uint8_t block[6];

    i2c_read_block(LSM9DS1_ACC_ADDRESS, LSM9DS1_OUT_X_L_XL, sizeof(block), block);       

	a[0] = (int16_t)(block[0] | block[1] << 8);
	a[1] = (int16_t)(block[2] | block[3] << 8);
	a[2] = (int16_t)(block[4] | block[5] << 8);
}

static void readMAG(int16_t m[])
{
	uint8_t block[6];

    i2c_read_block(LSM9DS1_MAG_ADDRESS, LSM9DS1_OUT_X_L_M, sizeof(block), block);    

	m[0] = (int16_t)(block[0] | block[1] << 8);
	m[1] = (int16_t)(block[2] | block[3] << 8);
	m[2] = (int16_t)(block[4] | block[5] << 8);
}

#if 0
static void readGYR(uint16_t g[])
{
	uint8_t block[6];

    i2c_read_block(LSM9DS1_GYR_ADDRESS, LSM9DS1_OUT_X_L_M, sizeof(block), block);    

	g[0] = (int16_t)(block[0] | block[1] << 8);
	g[1] = (int16_t)(block[2] | block[3] << 8);
	g[2] = (int16_t)(block[4] | block[5] << 8);
}
#endif

static void writeAccReg(uint8_t reg, uint8_t value)
{
	i2c_write(LSM9DS1_ACC_ADDRESS, reg, value);
}

static void writeMagReg(uint8_t reg, uint8_t value)
{
	i2c_write(LSM9DS1_MAG_ADDRESS, reg, value);
}

static void writeGyrReg(uint8_t reg, uint8_t value)
{
    i2c_write(LSM9DS1_GYR_ADDRESS, reg, value);
}

static void enableIMU()
{
    // SW_RESET
    //writeGyrReg(LSM9DS1_CTRL_REG8, 0x81);
    //nrf_delay_ms(200);
    //writeGyrReg(LSM9DS1_CTRL_REG8, 0x0);
    //nrf_delay_ms(200);

    // Enable the gyroscope
    writeGyrReg(LSM9DS1_CTRL_REG4,0b00111000);      // z, y, x axis enabled for gyro
    writeGyrReg(LSM9DS1_CTRL_REG1_G,0b10111000);    // Gyro ODR = 476Hz, 2000 dps
    writeGyrReg(LSM9DS1_ORIENT_CFG_G,0b10111000);   // Swap orientation 

    // Enable the accelerometer
    writeAccReg(LSM9DS1_CTRL_REG5_XL,0b00111000);   // z, y, x axis enabled for accelerometer
    writeAccReg(LSM9DS1_CTRL_REG6_XL,0b00101000);   // +/- 16g

    //Enable the magnetometer
    writeMagReg(LSM9DS1_CTRL_REG1_M, 0b10011100);   // Temp compensation enabled,Low power mode mode,80Hz ODR
    writeMagReg(LSM9DS1_CTRL_REG2_M, 0b01000000);   // +/-12gauss
    writeMagReg(LSM9DS1_CTRL_REG3_M, 0b00000000);   // continuos update
    writeMagReg(LSM9DS1_CTRL_REG4_M, 0b00000000);   // lower power mode for Z axis
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

    enableIMU();

    return ret;
}

int heading = 0;

float get_direction()
{
    float dir;
    int16_t magRaw[3];
    int16_t accRaw[3];

    float accXnorm,accYnorm,pitch,roll,magXcomp,magYcomp;

    readMAG(magRaw);
    readACC(accRaw);

    //If your IMU is upside down, comment out the two lines below which will correct the tilt calculation
    //accRaw[0] = -accRaw[0];
    //accRaw[1] = -accRaw[1];

#if 0
    //Compute heading
    heading = 180 * atan2(magRaw[1], magRaw[0]) / M_PI + ENV_OFFSET;

    //Convert heading to 0 - 360
    if(heading < 0)
        heading += 360;
#endif

    //Normalize accelerometer raw values.
    accXnorm = accRaw[0] / sqrt(accRaw[0] * accRaw[0] + accRaw[1] * accRaw[1] + accRaw[2] * accRaw[2]);
    accYnorm = accRaw[1] / sqrt(accRaw[0] * accRaw[0] + accRaw[1] * accRaw[1] + accRaw[2] * accRaw[2]);

    //Calculate pitch and roll
    pitch = asin(accXnorm);
    roll = -asin(accYnorm / cos(pitch));

    //Calculate the new tilt compensated values
    magXcomp = magRaw[0] * cos(pitch) + magRaw[2] * sin(pitch);
    magYcomp = magRaw[0] * sin(roll) * sin(pitch) + magRaw[1] * 
        cos(roll) + magRaw[2] * sin(roll) * cos(pitch);

    //Calculate heading
    dir = 180 * atan2(magYcomp, magXcomp) / M_PI + ENV_OFFSET;

    //Convert heading to 0 - 360
    if(dir < 0)
        dir += 360;

    heading = dir;
    // Deviation from the North
    dir += nav.dir;
    if(dir > 360)
        dir -= 360;


    return dir;
}
