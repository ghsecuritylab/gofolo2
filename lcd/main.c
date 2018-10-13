#include "nrf_gfx.h"
#include "nrf52_dk.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"
#include "app_error.h"
#include <string.h>


#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define BLACK           0
#define WHITE           1

#define LINE_STEP       10

#define CIRCLE_RADIUS   10
#define CIRCLE_STEP     ((2 * CIRCLE_RADIUS) + 1)

#define BORDER          2

static const char * test_text = "01234 Mika 56789";

extern const nrf_gfx_font_desc_t orkney_8ptFontInfo;
extern const nrf_lcd_t nrf_lcd_sharp;

static const nrf_gfx_font_desc_t * p_font = &orkney_8ptFontInfo;
static const nrf_lcd_t * p_lcd = &nrf_lcd_sharp;

static void gfx_initialization(void)
{
    APP_ERROR_CHECK(nrf_gfx_init(p_lcd));
}

static void text_print(void)
{
    nrf_gfx_point_t text_start = NRF_GFX_POINT(0, 128 - 15);
    APP_ERROR_CHECK(nrf_gfx_print(p_lcd, &text_start, 0, test_text, p_font, true));
    nrf_gfx_display(p_lcd);
}

static void screen_clear(void)
{
    nrf_gfx_screen_fill(p_lcd, WHITE);
    nrf_gfx_display(p_lcd);
}

static void line_draw(void)
{
    nrf_gfx_line_t my_line = NRF_GFX_LINE(0, 0, 0, nrf_gfx_height_get(p_lcd), 2);
    nrf_gfx_line_t my_line_2 = NRF_GFX_LINE(nrf_gfx_width_get(p_lcd), nrf_gfx_height_get(p_lcd), 0, nrf_gfx_height_get(p_lcd), 1);

    for (uint16_t i = 0; i <= nrf_gfx_width_get(p_lcd); i += LINE_STEP)
    {
        my_line.x_end = i;
        APP_ERROR_CHECK(nrf_gfx_line_draw(p_lcd, &my_line, BLACK));
    }

    my_line.x_end = nrf_gfx_width_get(p_lcd);

    for (uint16_t i = 0; i <= nrf_gfx_height_get(p_lcd); i += LINE_STEP)
    {
        my_line.y_end = (nrf_gfx_height_get(p_lcd) - i);
        APP_ERROR_CHECK(nrf_gfx_line_draw(p_lcd, &my_line, BLACK));
    }

    for (uint16_t i = 0; i <= nrf_gfx_height_get(p_lcd); i += LINE_STEP)
    {
        my_line_2.y_end = (nrf_gfx_height_get(p_lcd) - i);
        APP_ERROR_CHECK(nrf_gfx_line_draw(p_lcd, &my_line_2, WHITE));
    }

    my_line_2.y_end = 0;

    for (uint16_t i = 0; i <= nrf_gfx_width_get(p_lcd); i += LINE_STEP)
    {
        my_line_2.x_end = i;
        APP_ERROR_CHECK(nrf_gfx_line_draw(p_lcd, &my_line_2, WHITE));
    }
    nrf_gfx_display(p_lcd);
}

static void circle_draw(void)
{
    nrf_gfx_circle_t my_circle = NRF_GFX_CIRCLE(0, 0, CIRCLE_RADIUS);

    for (uint16_t j = 0; j <= nrf_gfx_height_get(p_lcd); j += CIRCLE_STEP)
    {
        my_circle.y = j;
        for (uint16_t i = 0; i <= nrf_gfx_width_get(p_lcd); i += CIRCLE_STEP)
        {
            my_circle.x = i;
            APP_ERROR_CHECK(nrf_gfx_circle_draw(p_lcd, &my_circle, WHITE, true));
        }
    }

    for (uint16_t j = CIRCLE_RADIUS; j <= nrf_gfx_height_get(p_lcd) + CIRCLE_RADIUS; j += CIRCLE_STEP)
    {
        my_circle.y = j;
        for (uint16_t i = CIRCLE_RADIUS; i <= nrf_gfx_width_get(p_lcd) + CIRCLE_RADIUS; i += CIRCLE_STEP)
        {
            my_circle.x = i;
            APP_ERROR_CHECK(nrf_gfx_circle_draw(p_lcd, &my_circle, BLACK, false));
        }
    }
    nrf_gfx_display(p_lcd);
}

static void rect_draw(void)
{
    nrf_gfx_rect_t my_rect = NRF_GFX_RECT(nrf_gfx_width_get(p_lcd) / 2,
                             nrf_gfx_height_get(p_lcd) / nrf_gfx_width_get(p_lcd),
                             nrf_gfx_height_get(p_lcd),
                             BORDER);

    my_rect.x = 0;
    my_rect.y = 0;
    my_rect.width = 50;
    my_rect.height = 50;

    // Draw using pseudo-random colors.
    APP_ERROR_CHECK(nrf_gfx_rect_draw(p_lcd, &my_rect, 1, BLACK, true));

    nrf_gfx_display(p_lcd);
}

#if 0
static ret_code_t test(void)
{
    ret_code_t err_code;

    nrf_gpio_cfg_output(20);
    nrf_gpio_pin_set(20); 

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
#endif

ret_code_t sharp_init(void);
void toggle_vcom();

int main(void)
{
    gfx_initialization();
    text_print();
    while (1)
    {
        toggle_vcom();
        if(0) {
        rect_draw();
            nrf_delay_ms(100);
            screen_clear();
            line_draw();
            screen_clear();
            circle_draw();
            screen_clear();
            rect_draw();
        }
    }
}

