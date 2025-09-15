// taivoite 1p


#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>


static const struct gpio_dt_spec red   = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec blue  = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);


// 0 = punainen, 1 = keltainen, 2 = vihreä
volatile int led_state = 0;


#define STACKSIZE 500
#define PRIORITY 5
void red_task(void *, void *, void *);
void yellow_task(void *, void *, void *);
void green_task(void *, void *, void *);

K_THREAD_DEFINE(red_thread, STACKSIZE, red_task, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(yellow_thread, STACKSIZE, yellow_task, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(green_thread, STACKSIZE, green_task, NULL, NULL, NULL, PRIORITY, 0, 0);

int init_leds(void) {
    int ret;
    if (!gpio_is_ready_dt(&red) || !gpio_is_ready_dt(&green) || !gpio_is_ready_dt(&blue)) {
        printk("Error: LED devices not ready\n");
        return -1;
    }
    ret = gpio_pin_configure_dt(&red, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) return ret;
    ret = gpio_pin_configure_dt(&green, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) return ret;
    ret = gpio_pin_configure_dt(&blue, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) return ret;

    printk("LEDs initialized\n");
    return 0;
}

void red_task(void *, void *, void *) {
    while (true) {
        if (led_state == 0) {
            gpio_pin_set_dt(&red, 1);
            k_sleep(K_SECONDS(1));
            gpio_pin_set_dt(&red, 0);
            led_state = 1; 
        } else {
            k_msleep(50); 
        }
    }
}

void yellow_task(void *, void *, void *) {
    while (true) {
        if (led_state == 1) {
            gpio_pin_set_dt(&red, 1);
            gpio_pin_set_dt(&green, 1);
            k_sleep(K_SECONDS(1));
            gpio_pin_set_dt(&red, 0);
            gpio_pin_set_dt(&green, 0);
            led_state = 2; 
        } else {
            k_msleep(50);
        }
    }
}

void green_task(void *, void *, void *) {
    while (true) {
        if (led_state == 2) {
            gpio_pin_set_dt(&green, 1);
            k_sleep(K_SECONDS(1));
            gpio_pin_set_dt(&green, 0);
            led_state = 0; 
        } else {
            k_msleep(50);
        }
    }
}

int main(void) {
    init_leds();
    printk("Traffic light FSM running...\n");

    while (true) {
        k_msleep(1000);
    }
    return 0;
}
