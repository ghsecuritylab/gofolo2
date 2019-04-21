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
#include "proto.h"

uint8_t nav_next = 0;
uint16_t nav_dir = 0;

nav_t nav = { 0 };

uint16_t ntohs(uint16_t const net) 
{
    uint8_t data[2] = { 0 };
    memcpy(&data, &net, sizeof(data));

    return ((uint16_t) data[1] << 0)
        | ((uint16_t) data[0] << 8);
}

uint32_t ntohl(uint32_t const net) 
{
    uint8_t data[4] = { 0 };
    memcpy(&data, &net, sizeof(data));

    return ((uint32_t) data[3] << 0)
        | ((uint32_t) data[2] << 8)
        | ((uint32_t) data[1] << 16)
        | ((uint32_t) data[0] << 24);
}

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
            st = 2;
            break;
        case CMD_CFG:
            nrf_cal_set_time_raw(ntohl(pkt->p.cfg.date));
            break;
        case CMD_START:
            break;
        case CMD_STOP:
            break;
        case CMD_NAV:
            nav.next = pkt->p.nav.next;
            nav.dir = pkt->p.nav.dir;
            nav.dist = pkt->p.nav.dist;
            nav.met = pkt->p.nav.met;
            nav.cov = pkt->p.nav.cov;
            break;
        case CMD_UPGRADE:
            break;
        case CMD_CALIBRATE:
            st = 3;
            break;
        default:
            break;
    }

    return;
}
