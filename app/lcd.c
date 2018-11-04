#include "nrf_gfx.h"
//#include "nrf52_dk.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"
#include "app_error.h"
#include "nrf_calendar.h"
#include <string.h>

#define BLACK           0
#define WHITE           1

extern const nrf_lcd_t nrf_lcd_sharp;

/* Font data for Motor Oil 1937 M54 14pt */
extern const nrf_gfx_font_desc_t orkney_24ptFontInfo;
extern const nrf_gfx_font_desc_t orkney_8ptFontInfo;
extern const nrf_gfx_font_desc_t motorOil1937M54_14ptFontInfo;
extern const nrf_gfx_font_desc_t microsoftSansSerif_14ptFontInfo;

static const nrf_lcd_t * p_lcd = &nrf_lcd_sharp;

void clear_lcd();
void gfx_initialization(void)
{
    APP_ERROR_CHECK(nrf_gfx_init(p_lcd));
}

void show_time()
{
    char time_str[20];

    clear_lcd();

    snprintf(time_str, 20, "%s", nrf_cal_get_time_string(false));

    nrf_gfx_point_t time_point = NRF_GFX_POINT(0, 40);
    APP_ERROR_CHECK(nrf_gfx_print(p_lcd, &time_point, 0, time_str, &orkney_24ptFontInfo, true));
    nrf_gfx_display(p_lcd);
    nrf_delay_ms(1000);
}

void show_distance(void)
{
    char distance_str[20];

    snprintf(distance_str, 20, "%s", get_distance_str(false));
    nrf_gfx_point_t distance_point = NRF_GFX_POINT(35, 128 - 15);
    APP_ERROR_CHECK(nrf_gfx_print(p_lcd, &distance_point, 0, distance_str, &motorOil1937M54_14ptFontInfo, true));
    nrf_gfx_display(p_lcd);
}

int select_frame(const nrf_lcd_t * p_lcd);

void show_arrow(void)
{
    select_frame(p_lcd);
    show_distance();
    nrf_delay_ms(200);
}
