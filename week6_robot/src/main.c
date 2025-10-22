// tavoite 1P

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>
#include <string.h>
#include <ctype.h>

enum {
    TP_ERR_NULL  = -1,
    TP_ERR_LEN   = -2,
    TP_ERR_NAN   = -3,
    TP_ERR_RANGE_H = -4,
    TP_ERR_RANGE_M = -5,
    TP_ERR_RANGE_S = -6,
};

static inline int to_two(const char *p) {
    return (p[0]-'0')*10 + (p[1]-'0');
}

int time_parse(const char *t) {
    if (!t) return TP_ERR_NULL;
    if (strlen(t) != 6) return TP_ERR_LEN;
    for (int i = 0; i < 6; i++) {
        if (!isdigit((unsigned char)t[i])) return TP_ERR_NAN;
    }
    int h = to_two(t);
    int m = to_two(t+2);
    int s = to_two(t+4);
    if (h < 0 || h > 23) return TP_ERR_RANGE_H;
    if (m < 0 || m > 59) return TP_ERR_RANGE_M;
    if (s < 0 || s > 59) return TP_ERR_RANGE_S;
    return h*3600 + m*60 + s;
}

static const struct device *uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

static void strip_crlf(char *s) {
    size_t n = strlen(s);
    while (n && (s[n-1] == '\r' || s[n-1] == '\n')) { s[--n] = '\0'; }
}

void main(void)
{
    if (!device_is_ready(uart_dev)) {
        for (;;) { k_sleep(K_FOREVER); }
    }

    char buf[64];
    size_t idx = 0;

    for (;;) {
        unsigned char c;
        int rc = uart_poll_in(uart_dev, &c);
        if (rc == 0) {
            if (c == '\n' || c == '\r') {
                if (idx > 0) {
                    buf[idx] = '\0';
                    strip_crlf(buf);
                    int res = time_parse(buf);
                    printk("%d\n", res);  
                    idx = 0;
                }
            } else {
                if (idx < sizeof(buf) - 1) {
                    buf[idx++] = (char)c;
                } else {
                    idx = 0;
                    printk("%d\n", TP_ERR_LEN);
                }
            }
        } else {
            k_sleep(K_MSEC(10));
        }
    }
}
