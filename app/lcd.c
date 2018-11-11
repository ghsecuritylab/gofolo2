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

#define BLACK           0
#define WHITE           1

extern const nrf_lcd_t nrf_lcd_sharp;

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

#define A 1
#define W 127
#define T 4

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

void show_distance(int ang)
{
    char distance_str[20];

    snprintf(distance_str, 20, "%dm", ang);
    nrf_gfx_point_t distance_point = NRF_GFX_POINT(45, 128 - 20);
    APP_ERROR_CHECK(nrf_gfx_print(p_lcd, &distance_point, 0, distance_str, &motorOil1937M54_14ptFontInfo, true));
    nrf_gfx_display(p_lcd);
}

int select_frame(const nrf_lcd_t * p_lcd, int ang);

#define M_PI 3.14159265358979323846264338327950288
int get_heading()
{
    int16_t d[3];
    uint8_t val[6];
    double heading = 0;

    i2c_read(0x28, &val[0], 6);

    d[0] = val[1]; d[0] <<= 8;
    d[1] = val[3]; d[1] <<= 8;
    d[2] = val[5]; d[2] <<= 8;

    d[0] |= val[0];
    d[1] |= val[2];
    d[2] |= val[4];

    d[0] = -d[0];
    //d[1] = -d[1];

    heading = 180 * atan2(d[1], d[0])/M_PI;
    if(heading < 0)
        heading += 360;

    return (int)heading;
}

void show_arrow(void)
{
    int ang = get_heading();
    select_frame(p_lcd, ang);
    show_distance(ang);
    nrf_delay_ms(200);
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
