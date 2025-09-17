#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>

#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)
static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

#define STACKSIZE 500
#define PRIORITY 5

static const struct gpio_dt_spec red   = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec blue  = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);

K_FIFO_DEFINE(dispatcher_fifo);
K_MUTEX_DEFINE(cv_mutex);
K_CONDVAR_DEFINE(red_cv);
K_CONDVAR_DEFINE(yellow_cv);
K_CONDVAR_DEFINE(green_cv);
K_CONDVAR_DEFINE(release_cv);

struct data_t {
    void *fifo_reserved;
    char color;
};

int init_leds(void) {
    int ret;
    if (!gpio_is_ready_dt(&red) || !gpio_is_ready_dt(&green) || !gpio_is_ready_dt(&blue)) {
        return -1;
    }
    ret = gpio_pin_configure_dt(&red, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) return ret;
    ret = gpio_pin_configure_dt(&green, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) return ret;
    ret = gpio_pin_configure_dt(&blue, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) return ret;
    return 0;
}

int init_uart(void) {
    if (!device_is_ready(uart_dev)) {
        return -1;
    }
    return 0;
}

void uart_task(void *, void *, void *) {
    char rc = 0;
    while (1) {
        if (uart_poll_in(uart_dev, &rc) == 0) {
            if (rc == 'R' || rc == 'Y' || rc == 'G') {
                struct data_t *buf = k_malloc(sizeof(struct data_t));
                if (buf == NULL) continue;
                buf->color = rc;
                k_fifo_put(&dispatcher_fifo, buf);
            }
        }
        k_msleep(10);
    }
}

void dispatcher_task(void *, void *, void *) {
    while (1) {
        struct data_t *item = k_fifo_get(&dispatcher_fifo, K_FOREVER);
        char color = item->color;
        k_free(item);
        k_mutex_lock(&cv_mutex, K_FOREVER);
        if (color == 'R') {
            k_condvar_signal(&red_cv);
        } else if (color == 'Y') {
            k_condvar_signal(&yellow_cv);
        } else if (color == 'G') {
            k_condvar_signal(&green_cv);
        }
        k_condvar_wait(&release_cv, &cv_mutex, K_FOREVER);
        k_mutex_unlock(&cv_mutex);
    }
}

void red_task(void *, void *, void *) {
    while (1) {
        k_mutex_lock(&cv_mutex, K_FOREVER);
        k_condvar_wait(&red_cv, &cv_mutex, K_FOREVER);
        k_mutex_unlock(&cv_mutex);
        gpio_pin_set_dt(&red, 1);
        k_sleep(K_SECONDS(1));
        gpio_pin_set_dt(&red, 0);
        k_mutex_lock(&cv_mutex, K_FOREVER);
        k_condvar_signal(&release_cv);
        k_mutex_unlock(&cv_mutex);
    }
}

void yellow_task(void *, void *, void *) {
    while (1) {
        k_mutex_lock(&cv_mutex, K_FOREVER);
        k_condvar_wait(&yellow_cv, &cv_mutex, K_FOREVER);
        k_mutex_unlock(&cv_mutex);
        gpio_pin_set_dt(&red, 1);
        gpio_pin_set_dt(&green, 1);
        k_sleep(K_SECONDS(1));
        gpio_pin_set_dt(&red, 0);
        gpio_pin_set_dt(&green, 0);
        k_mutex_lock(&cv_mutex, K_FOREVER);
        k_condvar_signal(&release_cv);
        k_mutex_unlock(&cv_mutex);
    }
}

void green_task(void *, void *, void *) {
    while (1) {
        k_mutex_lock(&cv_mutex, K_FOREVER);
        k_condvar_wait(&green_cv, &cv_mutex, K_FOREVER);
        k_mutex_unlock(&cv_mutex);
        gpio_pin_set_dt(&green, 1);
        k_sleep(K_SECONDS(1));
        gpio_pin_set_dt(&green, 0);
        k_mutex_lock(&cv_mutex, K_FOREVER);
        k_condvar_signal(&release_cv);
        k_mutex_unlock(&cv_mutex);
    }
}

K_THREAD_DEFINE(uart_thread, STACKSIZE, uart_task, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(dis_thread, STACKSIZE, dispatcher_task, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(red_thread, STACKSIZE, red_task, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(yellow_thread, STACKSIZE, yellow_task, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(green_thread, STACKSIZE, green_task, NULL, NULL, NULL, PRIORITY, 0, 0);

int main(void) {
    init_leds();
    init_uart();
    while (1) {
        k_msleep(1000);
    }
    return 0;
}
