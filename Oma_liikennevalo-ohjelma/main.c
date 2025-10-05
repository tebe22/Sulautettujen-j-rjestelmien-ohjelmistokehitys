// tavoite 1p

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <ctype.h>
#include <string.h>

#define UART_NODE DT_CHOSEN(zephyr_console)
static const struct device *const uart_dev = DEVICE_DT_GET(UART_NODE);

static const struct gpio_dt_spec red =
    GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0});

enum {
    TP_ERR_NULL    = -1,
    TP_ERR_LEN     = -2,
    TP_ERR_NAN     = -3,
    TP_ERR_RANGE_H = -4,
    TP_ERR_RANGE_M = -5,
    TP_ERR_RANGE_S = -6
};

static inline int to_two(const char* p) { return (p[0]-'0')*10 + (p[1]-'0'); }

int time_parse(const char* t) {
    if (!t) return TP_ERR_NULL;
    if (strlen(t) != 6) return TP_ERR_LEN;
    for (int i = 0; i < 6; i++) if (!isdigit((unsigned char)t[i])) return TP_ERR_NAN;
    int h = to_two(t + 0), m = to_two(t + 2), s = to_two(t + 4);
    if (h < 0 || h > 23) return TP_ERR_RANGE_H;
    if (m < 0 || m > 59) return TP_ERR_RANGE_M;
    if (s < 0 || s > 59) return TP_ERR_RANGE_S;
    return m * 60 + s;
}

static void led_work_handler(struct k_work *work) {
    if (!device_is_ready(red.port)) return;
    gpio_pin_set_dt(&red, 1);
    k_sleep(K_SECONDS(1));
    gpio_pin_set_dt(&red, 0);
}
K_WORK_DEFINE(led_work, led_work_handler);

static void timer_handler(struct k_timer *t) {
    k_work_submit(&led_work);
}
K_TIMER_DEFINE(my_timer, timer_handler, NULL);

int main(void) {
    if (!device_is_ready(uart_dev)) {
        printk("Console UART not ready\n");
    }
    if (!device_is_ready(red.port)) {
        printk("LED device not ready\n");
    } else {
        gpio_pin_configure_dt(&red, GPIO_OUTPUT_INACTIVE);
    }

    printk("Syota HHMMSS (6 numeroa). Esim 000005 -> LED syttyy 5 s paasta 1 s ajaksi.\n");

    char buf[7] = {0};
    int idx = 0;

    for (;;) {
        unsigned char c;
        if (uart_poll_in(uart_dev, (char *)&c) == 0) {
            if (c >= '0' && c <= '9') {
                buf[idx++] = (char)c;
                if (idx == 6) {
                    buf[6] = '\0';
                    int sec = time_parse(buf);
                    if (sec >= 0) {
                        printk("Timer start in %d s\n", sec);
                        k_timer_start(&my_timer, K_SECONDS(sec), K_NO_WAIT);
                    } else {
                        printk("Parse error %d for '%s'\n", sec, buf);
                    }
                    idx = 0;
                    memset(buf, 0, sizeof(buf));
                }
            } else {
                idx = 0;
                memset(buf, 0, sizeof(buf));
            }
        } else {
            k_msleep(10);
        }
    }
}
