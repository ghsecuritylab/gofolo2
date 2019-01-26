#include "sdk_common.h"
#include "math.h"
#include "nrf_lcd.h"
#include "nrf_drv_spi.h"
#include "nrf_gpio.h"
#include "frames/frame.h"

#include "arrows/arrow_00.h"
#include "arrows/small_arrow_l.h"
#include "arrows/small_arrow_r.h"

#include "nrf_delay.h"
#include "proto.h"

#define ARROW_S 112
#define M_PI 3.14159265358979323846264338327950288
#define M_GET_BYTE(a, i, j) (a[(i) * ARROW_S / 8 + (j) / 8])
#define M_GET_BIT(a, i, j) ((M_GET_BYTE((a), (i), (j)) >> (7 - (j) % 8)) & 1)
#define M_SET_BIT(a, i, j, v) (M_GET_BYTE((a), (i), (j)) = (M_GET_BYTE((a), (i), (j)) & ~(1 << (7 - (j) % 8))) | ((v) << (7 - (j) % 8)))

extern nav_t nav;

static float fastsinf[360];
static float fastcosf[360];
static uint8_t arrow[ARROW_S  * ARROW_S / 8];

void init_fast_sin_cos()
{
    int i;
    for(i = 1; i < 360; ++i) {
        fastsinf[i] = sinf(i * M_PI / 180);
        fastcosf[i] = cosf(i * M_PI / 180);
    }
}

void rotate_matrix(int degree)
{ 
    int i,j, ii, jj, v;
    float x0, y0, x1, y1;

    if(degree == 0 || degree == 360)
        return;

    // Rotation center
    x0 = y0 = ARROW_S / 2 - 1; 

    memset(arrow, 0xFF, sizeof(arrow));
    for(i = 0; i < ARROW_S; ++i) {
        for(j = 0; j < ARROW_S; ++j) {
            x1 = fastcosf[degree] * (j - x0) - fastsinf[degree] * (i - y0) + x0;
            y1 = fastsinf[degree] * (j - x0) + fastcosf[degree] * (i - y0) + y0;

            ii = round(y1);
            jj = round(x1);

            if(ii >= 0 && ii < ARROW_S && jj >= 0 && jj < ARROW_S)
                M_SET_BIT(arrow, ii, jj, M_GET_BIT(a0, i, j));
        }
    }

#if 1
    // anti-aliasing
    for(i = 0; i < ARROW_S; ++i) {
        for(j = 0; j < ARROW_S; ++j) {
            if(M_GET_BIT(arrow, i, j)) {
                if(i > 0 && i < ARROW_S - 1 && j > 0 && j < ARROW_S - 1)
                    v = round((float)(
                                M_GET_BIT(arrow, i, j) + 
                                M_GET_BIT(arrow, i + 1, j) +
                                M_GET_BIT(arrow, i - 1, j) +
                                M_GET_BIT(arrow, i + 1, j + 1) +
                                M_GET_BIT(arrow, i - 1, j + 1) +
                                M_GET_BIT(arrow, i + 1, j - 1) +
                                M_GET_BIT(arrow, i - 1, j - 1) +
                                M_GET_BIT(arrow, i, j - 1) +
                                M_GET_BIT(arrow, i, j + 1)) / 9.0);
                M_SET_BIT(arrow, i, j, v);
            }
        }
    }
#endif
}

static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(PCB_SPI_INSTANCE);

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
            break;
        case NRF_LCD_ROTATE_90:
            break;
        case NRF_LCD_ROTATE_180:
            break;
        case NRF_LCD_ROTATE_270:
            break;
        default:
            break;
    }
}

static void sharp_display_invert(bool invert) { }

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
        nrf_delay_ms(100);
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
#if 0
    uint8_t data[2];

    data[0] = 4;
    data[1] = 0;
    data[1] = data[0];

    nrf_gpio_pin_set(LCD_SPI_SS_PIN);
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, data, 2, NULL, 0));
    nrf_gpio_pin_clear(LCD_SPI_SS_PIN); 
#endif
    memset(frame, 0xFF, sizeof(frame));
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

    // Init lookup table for fast cosf() sinf() calculations.
    init_fast_sin_cos();

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

int select_frame(const nrf_lcd_t * p_lcd, int ang)
{
    memset(frame, 0xFF, sizeof(frame));

    rotate_matrix(ang);

    sharp_frame_draw(8, 0, 112, 112, arrow);

    switch(nav.next){
        case 1:
            sharp_frame_draw(1, 0, 24, 24, small_arrow_l);
            break;
        case 2:
            sharp_frame_draw(1, 0, 24, 24, small_arrow_r);
            break;
        default:
            break;
    }

    return 0;
}
