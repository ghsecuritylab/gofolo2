/*
 * GoFolo BLE Protocol v1.0:
 * 
 * PKT FORMAT: { CMD.1 [---------PAYLOAD----------] }
 *             ^----------- PKT_LEN ----------------^
 * 
 * The PKT_LEN can be 1 if the CMD doesn't have a PAYLOAD.
 * 
 * CMDs:
 *         0: Clock mode:           [ NO-PAYLOAD ]
 *         1: Navigation mode:      [ NO-PAYLOAD ]
 *         2: Detailed mode:        [ NO-PAYLOAD ]
 *         3: Configure the device: [ DATE_AND_TIME.4 | UNIT.1 ]
 * 
 *                                    DATE_AND_TIME - seconds since 1970-01-01 00:00:00 UTC
 *                                        NEXT_TURN - 0: RIGHT, 1: LEFT, 2: OFF
 *                                             UNIT - 0: in meters, 1: in feet
 * 
 *         4: Start navigation:     [ NO-PAYLOAD ] 
 *         5: Stop navigation:      [ NO-PAYLOAD ] 
 *         6: Navigation data:      [ DIRECTION.2 | NEXT_TURN.1 | METERING.4 | DISTANCE.4 | COVERED.4 ]
 * 
 *                                    DIRECTION - Deviation from the North: 0-360
 *                                    NEXT_TURN - 0: OFF, 1: LEFT, 2: RIGHT
 *                                     METERING - in UNITs
 *                                     DISTANCE - in UNITs
 *                                      COVERED - in UNITs
 * 
 *         7: Upgrade the firmware: [ LEN.4 | Firmware data ]
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "nrf_calendar.h"
#include "lcd.h"

uint32_t ntohl(uint32_t const net) 
{
    uint8_t data[4] = { 0 };
    memcpy(&data, &net, sizeof(data));

    return ((uint32_t) data[3] << 0)
        | ((uint32_t) data[2] << 8)
        | ((uint32_t) data[1] << 16)
        | ((uint32_t) data[0] << 24);
}

typedef struct __attribute__((__packed__)) cfg_s {
    uint32_t date;
    uint8_t unit;
} cfg_t;

typedef struct __attribute__((__packed__)) firm_s {
    uint32_t len;
    uint8_t data[0];
} firm_t;

typedef struct __attribute__((__packed__)) nav_s {
    uint16_t dir;
    uint8_t next;
    uint32_t met;
    uint32_t dist;
    uint32_t cov;
} nav_t;

typedef struct __attribute__((__packed__)) packet_s {
    uint8_t cmd;
    union {
        cfg_t cfg;
        nav_t nav;
        firm_t firm;
    } p;
} packet_t;

enum {
    CMD_CLOCK_MDOE,
    CMD_NAV_MODE,
    CMD_DETAILED,
    CMD_CFG,
    CMD_START,
    CMD_STOP,
    CMD_NAV,
    CMD_UPGRADE,
};

extern int st;

void data_handler(int len, const uint8_t *p)
{
#if 0
    char buff[64];
    sprintf(buff, "%08x", ntohl(pkt->p.cfg.date));
    lcd_print(10, buff);
#endif
    packet_t *pkt = (packet_t *)p;

    switch(pkt->cmd) {
        case CMD_CLOCK_MDOE:
            st = 0;
            break;
        case CMD_NAV_MODE:
            st = 1;
            break;
        case CMD_DETAILED:
            st = 1;
            break;
        case CMD_CFG:
            nrf_cal_set_time_raw(ntohl(pkt->p.cfg.date));
            break;
        case CMD_START:
            break;
        case CMD_STOP:
            break;
        case CMD_NAV:
            break;
        case CMD_UPGRADE:
            break;
        default:
            break;
    }

    return;
}
