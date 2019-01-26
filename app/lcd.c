#include <string.h>
#include "nrf_gfx.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"
#include "app_error.h"
#include "nrf_calendar.h"
#include "imu.h"
#include "math.h"
#include "lcd.h"

#define A 1
#define W 127
#define T 4

#define BLACK           0
#define WHITE           1

extern int failed;
extern const nrf_lcd_t nrf_lcd_sharp;
int select_frame(const nrf_lcd_t * p_lcd, int ang);

/* Font data for Motor Oil 1937 M54 14pt */
extern const nrf_gfx_font_desc_t motorOil1937M54_14ptFontInfo;
extern const nrf_gfx_font_desc_t carson_50ptFontInfo;
extern const nrf_gfx_font_desc_t roboto_12ptFontInfo;

static const nrf_lcd_t * p_lcd = &nrf_lcd_sharp;

void clear_lcd();
void gfx_initialization(void)
{
    APP_ERROR_CHECK(nrf_gfx_init(p_lcd));
}

void show_seconds()
{
    struct tm *t;
    char s_str[32] = {0};

    t = nrf_cal_get_time();

    if (t->tm_sec < 15) {
        nrf_gfx_line_t a = NRF_GFX_LINE(A, A, (t->tm_sec * 8), A, T);
        APP_ERROR_CHECK(nrf_gfx_line_draw(p_lcd, &a, 0));
    } else if(t->tm_sec <= 30) {
        nrf_gfx_line_t a = NRF_GFX_LINE(A, A, W - A, A, T);
        nrf_gfx_line_t b = NRF_GFX_LINE(W - T, A, W - T, (t->tm_sec - 15) * 8, T);
        APP_ERROR_CHECK(nrf_gfx_line_draw(p_lcd, &a, 0));
        APP_ERROR_CHECK(nrf_gfx_line_draw(p_lcd, &b, 0));
    } else if(t->tm_sec <= 45) {
        nrf_gfx_line_t a = NRF_GFX_LINE(A, A, W - A, A, T);
        nrf_gfx_line_t b = NRF_GFX_LINE(W - T, A, W - T, W - A, T);
        nrf_gfx_line_t c = NRF_GFX_LINE(W - (t->tm_sec - 30) * 8, W - T, W - A, W - T, T);
        APP_ERROR_CHECK(nrf_gfx_line_draw(p_lcd, &a, 0));
        APP_ERROR_CHECK(nrf_gfx_line_draw(p_lcd, &b, 0));
        APP_ERROR_CHECK(nrf_gfx_line_draw(p_lcd, &c, 0));
    } else if(t->tm_sec <= 60) {
        nrf_gfx_line_t a = NRF_GFX_LINE(A, A, W - A, A, T);
        nrf_gfx_line_t b = NRF_GFX_LINE(W - T, A, W - T, W - A, T);
        nrf_gfx_line_t c = NRF_GFX_LINE(A, W - T, W - A, W - T, T);
        nrf_gfx_line_t d = NRF_GFX_LINE(A, W - (t->tm_sec - 45) * 8, A, W - T, T);

        APP_ERROR_CHECK(nrf_gfx_line_draw(p_lcd, &a, 0));
        APP_ERROR_CHECK(nrf_gfx_line_draw(p_lcd, &b, 0));
        APP_ERROR_CHECK(nrf_gfx_line_draw(p_lcd, &c, 0));
        APP_ERROR_CHECK(nrf_gfx_line_draw(p_lcd, &d, 0));
    }

    strftime(s_str, 80, "%D", t);
    nrf_gfx_point_t s = NRF_GFX_POINT(30, 105);
    APP_ERROR_CHECK(nrf_gfx_print(p_lcd, &s, 0, s_str, &roboto_12ptFontInfo, true));

    strftime(s_str, 80, "%a", t);
    nrf_gfx_point_t wk = NRF_GFX_POINT(91, 10);
    APP_ERROR_CHECK(nrf_gfx_print(p_lcd, &wk, 0, s_str, &roboto_12ptFontInfo, true));
}

void show_time()
{
    char time_str[20];

    clear_lcd();

    snprintf(time_str, 20, "%s", nrf_cal_get_time_string(false));

    nrf_gfx_point_t time_point = NRF_GFX_POINT(8, 20);
    APP_ERROR_CHECK(nrf_gfx_print(p_lcd, &time_point, 0, time_str, &carson_50ptFontInfo, true));

    show_seconds();

    nrf_gfx_display(p_lcd);
    nrf_delay_ms(1000);
}

void show_distance(int ang, int tc_ang)
{
    char distance_str[20];

    snprintf(distance_str, 20, "%d %d", ang, tc_ang);
    nrf_gfx_point_t distance_point = NRF_GFX_POINT(45, 128 - 20);
    APP_ERROR_CHECK(nrf_gfx_print(p_lcd, &distance_point, 0, distance_str, &roboto_12ptFontInfo, true));
    nrf_gfx_display(p_lcd);
}

void show_arrow(void)
{
    double heading = 0;
    double ang;

    ang = get_heading(&heading);
    if(failed > 0)
        select_frame(p_lcd, 90);
    else 
        select_frame(p_lcd, 360 - heading);

    show_distance((int)round(ang), (int)round(heading));
}

void lcd_flush()
{
    nrf_gfx_display(p_lcd);
}

void lcd_print(int y, char *st)
{
    clear_lcd();
    nrf_gfx_point_t wk = NRF_GFX_POINT(10, y);
    APP_ERROR_CHECK(nrf_gfx_print(p_lcd, &wk, 0, st, &roboto_12ptFontInfo, true));
    nrf_gfx_display(p_lcd);
}
