#include "sdk_common.h"
#include "nrf_lcd.h"
#include "nrf_drv_spi.h"
#include "nrf_gpio.h"
#include "frame.h"

#define DISP_EN_PIN 20

uint8_t **m = (uint8_t **)frame;

// Set of commands described in ILI9341 datasheet.

static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(1);

static void sharp_pixel_draw(uint16_t x, uint16_t y, uint32_t color)
{
    if(color)
        m[x / 8][y] = m[x / 8][y] & (1 << x % 8);
    else
        m[x / 8][y] = m[x / 8][y] & ~(1 << x % 8);

    return;
}

static void sharp_rect_draw(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color)
{
    int i, j;

    for(i = x; i <= width; i++) {
        for(j = y; j <= height; j++) {
            sharp_pixel_draw(i, j, color);
        }
    }

    return;
}

static void sharp_rotation_set(nrf_lcd_rotation_t rotation)
{
}

static void sharp_display_invert(bool invert)
{
}

static lcd_cb_t sharp_cb = {
    .height = 128,
    .width = 128
};

/* LCD resolution */
#define LCD_XRES 128
#define LCD_YRES 128
#define LCD_BYTES_LINE LCD_XRES / 8
#define LCD_BUF_SIZE LCD_YRES * LCD_BYTES_LINE

static void sharp_display(void)
{
    int i;
    uint8_t tx_data[LCD_BYTES_LINE + 4];

    memset(tx_data, 0, sizeof(tx_data));
    for(i = 1; i <= LCD_YRES; ++i) {
        tx_data[0] = 1 | ((1 & (i / 2)) << 1);
        tx_data[1] = i;
        memcpy(tx_data + 2, frame + (i - 1) * LCD_BYTES_LINE, LCD_BYTES_LINE);
        nrf_drv_spi_transfer(&spi, tx_data, LCD_BYTES_LINE + 4, NULL, 0);
    }

    return;
}

static ret_code_t sharp_init(void)
{
    ret_code_t err_code;

    nrf_gpio_cfg_output(DISP_EN_PIN);

    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.frequency = NRF_DRV_SPI_FREQ_1M;
    spi_config.ss_pin = 28;
    spi_config.mosi_pin = 5;
    spi_config.sck_pin = 4;

    spi_config.mode = NRF_DRV_SPI_MODE_0;
    spi_config.bit_order = NRF_DRV_SPI_BIT_ORDER_LSB_FIRST;

    err_code = nrf_drv_spi_init(&spi, &spi_config, NULL, NULL);

    return err_code;
}

static void sharp_uninit(void)
{
    nrf_drv_spi_uninit(&spi);
}

const nrf_lcd_t nrf_lcd_sharp = {
    .lcd_init = sharp_init,
    .lcd_uninit = sharp_uninit,
    .lcd_pixel_draw = sharp_pixel_draw,
    .lcd_rect_draw = sharp_rect_draw,
    .lcd_display = sharp_display,
    .lcd_rotation_set = sharp_rotation_set,
    .lcd_display_invert = sharp_display_invert,
    .p_lcd_cb = &sharp_cb
};
