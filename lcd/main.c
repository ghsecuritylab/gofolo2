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

static char test_text1[16];

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
    nrf_gfx_point_t text_start1 = NRF_GFX_POINT(0, 1);
    nrf_gfx_point_t text_start2 = NRF_GFX_POINT(0, 128 - 15);
    APP_ERROR_CHECK(nrf_gfx_print(p_lcd, &text_start1, 0, test_text1, p_font, true));
    APP_ERROR_CHECK(nrf_gfx_print(p_lcd, &text_start2, 0, test_text1, p_font, true));
    nrf_gfx_display(p_lcd);
}

void toggle_vcom();
int select_frame();

int main(void)
{
    int f = 0;
    gfx_initialization();
        /*toggle_vcom();*/
    while (1)
    {
        nrf_delay_ms(100);
        f = select_frame();
        sprintf(test_text1, "%02d==GoFolo====", f);
        text_print();
    }
}

