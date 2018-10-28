#include "sdk_common.h"
#include "nrf_lcd.h"
#include "nrf_drv_spi.h"
#include "nrf_gpio.h"
#include "frames/frame.h"
#include "frames/small_arrow_l.h"
#include "frames/small_arrow_r.h"
#include "frames/f00.h"
#include "frames/f10.h"
#include "frames/f20.h"
#include "frames/f30.h"
#include "frames/f40.h"
#include "frames/f50.h"
#include "frames/f60.h"
#include "frames/f70.h"
#include "frames/f80.h"
#include "frames/f90.h"

#include "nrf_delay.h"

#define DISP_EN_PIN 20
#define LCD_SPI_SS_PIN 28
#define LCD_SPI_SCK_PIN 4
#define LCD_SPI_MOSI_PIN 5

#define COLUMNS 128
#define ROWS 128

static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(0);

static void bytes_swap(uint8_t *a, uint8_t *b, int ai, int bi)
{
    uint8_t tmpa, tmpb;

    ai = 7 - ai;
    bi = 7 - bi;

    tmpa = ((*a >> ai) & 1);
    tmpb = ((*b >> bi) & 1);

    *a = (*a & ~(1 << ai)) | (tmpb << ai);
    *b = (*b & ~(1 << bi)) | (tmpa << bi);
}

static void reverse_columns(uint8_t *bitarr, int X, int Y)
{ 
    int i, j, k;

    for (i = 0; i < Y; i++) 
        for (j = 0, k = Y - 1; j < k; j++, k--) 
            bytes_swap(&bitarr[j * Y / 8 + i / 8], &bitarr[k * Y / 8 + i / 8], i % 8, i % 8);
} 

static void reverse_rows(uint8_t *bitarr, int X, int Y) 
{ 
    int i, j, k;

    for (i = 0; i < X; i++) 
        for (j = 0, k = Y - 1; j < k; j++, k--) 
            bytes_swap(&bitarr[i * Y / 8 + j / 8], &bitarr[i * Y / 8 + k / 8], j % 8, k % 8);
} 
  
static void transpose(uint8_t *bitarr, int X, int Y)
{ 
    int i, j;
    for (i = 0; i < X; i++) 
        for (j = i; j < Y; j++) 
            bytes_swap(&bitarr[i * Y / 8 + j / 8], &bitarr[j * Y / 8 + i / 8], j % 8, i % 8);
}

static void rotate_left(uint8_t *m, int x, int y)
{ 
    transpose(m, x, y); 
    reverse_columns(m, x, y); 
} 

static void rotate_right(uint8_t *m, int x, int y)
{ 
    transpose(m, x, y); 
    reverse_rows(m, x, y); 
} 

static void sharp_pixel_draw(uint16_t x, uint16_t y, uint32_t color)
{
    if(color)
        frame[x / 8 + 16 * y] = frame[x / 8 + 16 * y] | (1 << (7 - x % 8));
    else
        frame[x / 8 + 16 * y] = frame[x / 8 + 16 * y] & ~(1 << (7 - x % 8));
}

static void sharp_frame_draw(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *f)
{
    uint16_t i = 0, j = 0, m, n;

    for(m = 0, i = x; i < height + x; i++, m++) {
        for(n = 0, j = y; j < width + y; j++, n++) {
          sharp_pixel_draw(i, j, (f[n * width / 8 + m / 8] >> (7 - m % 8)) & 1);
        }
    }
}

static void sharp_rect_draw(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color)
{
    uint16_t i = 0, j = 0;

    for(i = x; i < width + x; i++) {
        for(j = y; j < height + y; j++) {
          sharp_pixel_draw(i, j, color);
        }
    }
    
}

static void sharp_rotation_set(nrf_lcd_rotation_t rotation)
{
    switch (rotation) {
        case NRF_LCD_ROTATE_0:
            sharp_frame_draw(4, 4, 16, 16, small_arrow_l);
            break;
        case NRF_LCD_ROTATE_90:
            rotate_right(frame, ROWS, COLUMNS);
            sharp_frame_draw(4, 4, 16, 16, small_arrow_r);
            break;
        case NRF_LCD_ROTATE_180:
            rotate_right(frame, ROWS, COLUMNS);
            rotate_right(frame, ROWS, COLUMNS);
            sharp_frame_draw(4, 4, 16, 16, small_arrow_l);
            break;
        case NRF_LCD_ROTATE_270:
            rotate_left(frame, ROWS, COLUMNS);
            sharp_frame_draw(4, 4, 16, 16, small_arrow_r);
            break;
        default:
            break;
    }
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

uint8_t bit_swap(uint8_t b) 
{
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;

    return b;
}

void toggle_vcom()
{
    int i = 0;
    uint8_t vcom[2];

    vcom[0] = 0x00;
    vcom[1] = 0x00;

    while(i == 2) {
        vcom[0] = ((1 & (i / 2)) << 1);
        nrf_gpio_pin_set(LCD_SPI_SS_PIN); 
        nrf_delay_ms(5000);
        APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, vcom, 2, NULL, 0));
        nrf_gpio_pin_clear(LCD_SPI_SS_PIN); 
        i++;
    }
}

static void sharp_display(void)
{
    int i, j;
    uint8_t tx_data[LCD_BYTES_LINE + 4];

    memset(tx_data, 0, sizeof(tx_data));
    for(i = 1; i <= LCD_YRES; ++i) {
        tx_data[0] = 1 | ((1 & (i / 2)) << 1);
        tx_data[1] = i;
        memcpy(tx_data + 2, frame + (i - 1) * LCD_BYTES_LINE, LCD_BYTES_LINE);

        for(j = 2; j < sizeof(tx_data) - 2; ++j)
            tx_data[j] = bit_swap(tx_data[j]);

        nrf_gpio_pin_set(LCD_SPI_SS_PIN); 
        nrf_drv_spi_transfer(&spi, tx_data, LCD_BYTES_LINE + 4, NULL, 0);
        nrf_gpio_pin_clear(LCD_SPI_SS_PIN); 
    }

    //toggle_vcom();
}

void clear_lcd()
{
    uint8_t data[2];

    data[0] = 4;
    data[1] = 0;
    data[1] = data[0];

    nrf_gpio_pin_set(LCD_SPI_SS_PIN);
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, data, 2, NULL, 0));
    nrf_gpio_pin_clear(LCD_SPI_SS_PIN); 
}

ret_code_t sharp_init(void)
{
    ret_code_t err_code;

    nrf_gpio_cfg_output(DISP_EN_PIN);
    nrf_gpio_pin_set(DISP_EN_PIN); 

    nrf_gpio_pin_set(LCD_SPI_SS_PIN); 
    nrf_gpio_cfg_output(LCD_SPI_SS_PIN);

    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.frequency = NRF_SPIM_FREQ_1M;
    spi_config.mosi_pin = LCD_SPI_MOSI_PIN;
    spi_config.sck_pin = LCD_SPI_SCK_PIN;

    spi_config.mode = NRF_DRV_SPI_MODE_0;
    spi_config.bit_order = NRF_DRV_SPI_BIT_ORDER_LSB_FIRST;

    err_code = nrf_drv_spi_init(&spi, &spi_config, NULL, NULL);

    clear_lcd();
    nrf_delay_ms(1000);
    sharp_display();

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

const uint8_t *ptr[10] = {
    f0,
    f1,
    f2,
    f3,
    f4,
    f5,
    f6,
    f7,
    f8,
    f9,
};

int select_frame(const nrf_lcd_t * p_lcd)
{
    uint8_t f;
    uint16_t tmp = 0;
    static uint16_t a = 0;
    nrf_lcd_rotation_t r = NRF_LCD_ROTATE_0;

    tmp = a;

    if(a > 90 && a <= 180) {
        r = NRF_LCD_ROTATE_90; tmp -= 90;
    } else if(a > 180 && a <= 270) {
        r = NRF_LCD_ROTATE_180; tmp -= 180;
    } else if(a > 270 && a <= 360) {
        r = NRF_LCD_ROTATE_270; tmp -= 270;
    }

    f = (tmp % 10) < 5 ? (tmp / 10) : (tmp / 10 + 1);
    memcpy(frame, ptr[f], sizeof(frame));

    p_lcd->lcd_rotation_set(r);

    a += 10; if(a == 360) a = 0;

    return f;
}
