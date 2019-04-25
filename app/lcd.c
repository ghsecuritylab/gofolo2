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
#include "proto.h"

#define A 1
#define W 127
#define T 4

#define BLACK 0
#define WHITE 1

uint8_t color_cfg = BLACK;

#define LCD_PRINT(x, y, font, ...) \
    do { \
        char s[32]; \
        sprintf(s, ##__VA_ARGS__); \
        nrf_gfx_point_t p = NRF_GFX_POINT((x), (y)); \
        APP_ERROR_CHECK(nrf_gfx_print(p_lcd, &p, color_cfg, s, (font), true)); \
    } while(0)

extern nav_t nav;
extern int failed;
extern const nrf_lcd_t nrf_lcd_sharp;
int select_frame(const nrf_lcd_t * p_lcd, int ang);

/* Font data for Motor Oil 1937 M54 14pt */
extern const nrf_gfx_font_desc_t motorOil1937M54_14ptFontInfo;
extern const nrf_gfx_font_desc_t carson_50ptFontInfo;
extern const nrf_gfx_font_desc_t roboto_12ptFontInfo;
extern const nrf_gfx_font_desc_t orkney_8ptFontInfo;

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
        APP_ERROR_CHECK(nrf_gfx_line_draw(p_lcd, &a, color_cfg));
    } else if(t->tm_sec <= 30) {
        nrf_gfx_line_t a = NRF_GFX_LINE(A, A, W - A, A, T);
        nrf_gfx_line_t b = NRF_GFX_LINE(W - T, A, W - T, (t->tm_sec - 15) * 8, T);
        APP_ERROR_CHECK(nrf_gfx_line_draw(p_lcd, &a, color_cfg));
        APP_ERROR_CHECK(nrf_gfx_line_draw(p_lcd, &b, color_cfg));
    } else if(t->tm_sec <= 45) {
        nrf_gfx_line_t a = NRF_GFX_LINE(A, A, W - A, A, T);
        nrf_gfx_line_t b = NRF_GFX_LINE(W - T, A, W - T, W - A, T);
        nrf_gfx_line_t c = NRF_GFX_LINE(W - (t->tm_sec - 30) * 8, W - T, W - A, W - T, T);
        APP_ERROR_CHECK(nrf_gfx_line_draw(p_lcd, &a, color_cfg));
        APP_ERROR_CHECK(nrf_gfx_line_draw(p_lcd, &b, color_cfg));
        APP_ERROR_CHECK(nrf_gfx_line_draw(p_lcd, &c, color_cfg));
    } else if(t->tm_sec <= 60) {
        nrf_gfx_line_t a = NRF_GFX_LINE(A, A, W - A, A, T);
        nrf_gfx_line_t b = NRF_GFX_LINE(W - T, A, W - T, W - A, T);
        nrf_gfx_line_t c = NRF_GFX_LINE(A, W - T, W - A, W - T, T);
        nrf_gfx_line_t d = NRF_GFX_LINE(A, W - (t->tm_sec - 45) * 8, A, W - T, T);

        APP_ERROR_CHECK(nrf_gfx_line_draw(p_lcd, &a, color_cfg));
        APP_ERROR_CHECK(nrf_gfx_line_draw(p_lcd, &b, color_cfg));
        APP_ERROR_CHECK(nrf_gfx_line_draw(p_lcd, &c, color_cfg));
        APP_ERROR_CHECK(nrf_gfx_line_draw(p_lcd, &d, color_cfg));
    }

    strftime(s_str, 80, "%D", t);
    nrf_gfx_point_t s = NRF_GFX_POINT(30, 105);
    APP_ERROR_CHECK(nrf_gfx_print(p_lcd, &s, color_cfg, s_str, &roboto_12ptFontInfo, true));

    strftime(s_str, 80, "%a", t);
    nrf_gfx_point_t wk = NRF_GFX_POINT(91, 10);
    APP_ERROR_CHECK(nrf_gfx_print(p_lcd, &wk, color_cfg, s_str, &roboto_12ptFontInfo, true));
}

void show_time()
{
    char time_str[20];

    clear_lcd();

    snprintf(time_str, 20, "%s", nrf_cal_get_time_string(false));

    nrf_gfx_point_t time_point = NRF_GFX_POINT(8, 20);
    APP_ERROR_CHECK(nrf_gfx_print(p_lcd, &time_point, color_cfg, time_str, &carson_50ptFontInfo, true));

    show_seconds();

    nrf_gfx_display(p_lcd);
    nrf_delay_ms(1000);
}

extern int heading;
void show_arrow(void)
{
    int ang = roundf(get_direction());
    if(failed > 0)
        select_frame(p_lcd, 90);
    else 
        select_frame(p_lcd, 360 - ang);

    if (nav.met / 1000) {
        LCD_PRINT(45, 128 - 20, &motorOil1937M54_14ptFontInfo, "%lu.%lukm", (nav.met / 1000), ((nav.met % 1000) / 100));
    } else {
        LCD_PRINT(45, 128 - 20, &motorOil1937M54_14ptFontInfo, "%lum", nav.met);
    }
#if 0
    LCD_PRINT(0, 128 - 20, &orkney_8ptFontInfo, "Dr:%d", nav.dir);
    LCD_PRINT(0, 128 - 40, &orkney_8ptFontInfo, "M:%lu", nav.met);
    LCD_PRINT(45, 128 - 20, &orkney_8ptFontInfo, "N:%d", heading);
    LCD_PRINT(90, 128 - 20, &orkney_8ptFontInfo, "D:%lu", nav.dist);
    LCD_PRINT(90, 128 - 40, &orkney_8ptFontInfo, "C:%lu", nav.cov);
#endif

    nrf_gfx_display(p_lcd);
}

#define COL1 1
#define COL2 83

void show_detail(void)
{
    uint32_t left;

    clear_lcd();

    left = nav.dist - nav.cov;

    if (nav.dist / 1000) {
        LCD_PRINT(COL1, 10, &roboto_12ptFontInfo, "DISTANCE");    LCD_PRINT(COL2, 10, &roboto_12ptFontInfo, "%lu.%lukm", (nav.dist / 1000), ((nav.dist % 1000) / 100));
    } else {
        LCD_PRINT(COL1, 10, &roboto_12ptFontInfo, "DISTANCE");    LCD_PRINT(COL2, 10, &roboto_12ptFontInfo, "%lum", nav.dist);
    }

    if (left / 1000) {
        LCD_PRINT(COL1, 40, &roboto_12ptFontInfo, "LEFT");        LCD_PRINT(COL2, 40, &roboto_12ptFontInfo, "%lu.%lukm", (left / 1000), ((left % 1000) / 100));
    } else {
        LCD_PRINT(COL1, 40, &roboto_12ptFontInfo, "LEFT");        LCD_PRINT(COL2, 40, &roboto_12ptFontInfo, "%lum", left);
    }

    if (nav.cov / 1000) {
        LCD_PRINT(COL1, 95, &roboto_12ptFontInfo, "COVERED");     LCD_PRINT(COL2, 95, &roboto_12ptFontInfo, "%lu.%lukm", (nav.cov / 1000), ((nav.cov % 1000) / 100));
    } else {
        LCD_PRINT(COL1, 95, &roboto_12ptFontInfo, "COVERED");     LCD_PRINT(COL2, 95, &roboto_12ptFontInfo, "%lum", nav.cov);
    }

    nrf_gfx_display(p_lcd);
}

void print_calibration(int16_t m[], int16_t n[], int cal)
{
    clear_lcd();

    if(cal)
        LCD_PRINT(20, 30, &orkney_8ptFontInfo, "CALIBRATION...");
    else
        LCD_PRINT(20, 30, &orkney_8ptFontInfo, "CALIBRATED");

    LCD_PRINT(5, 128 - 60, &orkney_8ptFontInfo, "X:%d", m[0]);
    LCD_PRINT(5, 128 - 40, &orkney_8ptFontInfo, "Y:%d", m[1]);
    LCD_PRINT(5, 128 - 20, &orkney_8ptFontInfo, "Z:%d", m[2]);

    LCD_PRINT(65, 128 - 60, &orkney_8ptFontInfo, "X:%d", n[0]);
    LCD_PRINT(65, 128 - 40, &orkney_8ptFontInfo, "Y:%d", n[1]);
    LCD_PRINT(65, 128 - 20, &orkney_8ptFontInfo, "Z:%d", n[2]);

    nrf_gfx_display(p_lcd);
}

void lcd_flush()
{
    nrf_gfx_display(p_lcd);
}

void lcd_print(int y, char *st)
{
    clear_lcd();
    nrf_gfx_point_t wk = NRF_GFX_POINT(10, y);
    APP_ERROR_CHECK(nrf_gfx_print(p_lcd, &wk, color_cfg, st, &roboto_12ptFontInfo, true));
    nrf_gfx_display(p_lcd);
}
