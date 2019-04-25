typedef struct __attribute__((__packed__)) cfg_s {
    uint32_t date;
    uint8_t unit;
    uint8_t color;
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
    CMD_CALIBRATE,
};

void data_handler(int len, const uint8_t *pkt);
