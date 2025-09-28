#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/timing/timing.h>

#define STACKSIZE 500
#define PRIORITY 5

static const struct gpio_dt_spec red   = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec blue  = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);

static bool debug_enabled = true;
#define DBG_PRINTK(...) do { if (debug_enabled) { printk(__VA_ARGS__); } } while (0)

static uint64_t red_last_us;
static uint64_t yellow_last_us;
static uint64_t green_last_us;

K_MUTEX_DEFINE(cv_mutex);
K_CONDVAR_DEFINE(red_cv);
K_CONDVAR_DEFINE(yellow_cv);
K_CONDVAR_DEFINE(green_cv);
K_CONDVAR_DEFINE(release_cv);

static int init_leds(void) {
    int ret;
    ret = gpio_pin_configure_dt(&red, GPIO_OUTPUT_INACTIVE);   if (ret < 0) return ret;
    ret = gpio_pin_configure_dt(&green, GPIO_OUTPUT_INACTIVE); if (ret < 0) return ret;
    ret = gpio_pin_configure_dt(&blue, GPIO_OUTPUT_INACTIVE);  if (ret < 0) return ret;
    return 0;
}

static void red_task(void *, void *, void *) {
    while (1) {
        k_mutex_lock(&cv_mutex, K_FOREVER);
        k_condvar_wait(&red_cv, &cv_mutex, K_FOREVER);
        k_mutex_unlock(&cv_mutex);
        timing_start();
        timing_t t0 = timing_counter_get();
        DBG_PRINTK("R on\n");
        gpio_pin_set_dt(&red, 1);
        k_sleep(K_SECONDS(1));
        gpio_pin_set_dt(&red, 0);
        DBG_PRINTK("R off\n");
        timing_t t1 = timing_counter_get();
        timing_stop();
        red_last_us = timing_cycles_to_ns(timing_cycles_get(&t0, &t1)) / 1000ULL;
        k_mutex_lock(&cv_mutex, K_FOREVER);
        k_condvar_signal(&release_cv);
        k_mutex_unlock(&cv_mutex);
    }
}

static void yellow_task(void *, void *, void *) {
    while (1) {
        k_mutex_lock(&cv_mutex, K_FOREVER);
        k_condvar_wait(&yellow_cv, &cv_mutex, K_FOREVER);
        k_mutex_unlock(&cv_mutex);
        timing_start();
        timing_t t0 = timing_counter_get();
        DBG_PRINTK("Y on\n");
        gpio_pin_set_dt(&red, 1);
        gpio_pin_set_dt(&green, 1);
        k_sleep(K_SECONDS(1));
        gpio_pin_set_dt(&red, 0);
        gpio_pin_set_dt(&green, 0);
        DBG_PRINTK("Y off\n");
        timing_t t1 = timing_counter_get();
        timing_stop();
        yellow_last_us = timing_cycles_to_ns(timing_cycles_get(&t0, &t1)) / 1000ULL;
        k_mutex_lock(&cv_mutex, K_FOREVER);
        k_condvar_signal(&release_cv);
        k_mutex_unlock(&cv_mutex);
    }
}

static void green_task(void *, void *, void *) {
    while (1) {
        k_mutex_lock(&cv_mutex, K_FOREVER);
        k_condvar_wait(&green_cv, &cv_mutex, K_FOREVER);
        k_mutex_unlock(&cv_mutex);
        timing_start();
        timing_t t0 = timing_counter_get();
        DBG_PRINTK("G on\n");
        gpio_pin_set_dt(&green, 1);
        k_sleep(K_SECONDS(1));
        gpio_pin_set_dt(&green, 0);
        DBG_PRINTK("G off\n");
        timing_t t1 = timing_counter_get();
        timing_stop();
        green_last_us = timing_cycles_to_ns(timing_cycles_get(&t0, &t1)) / 1000ULL;
        k_mutex_lock(&cv_mutex, K_FOREVER);
        k_condvar_signal(&release_cv);
        k_mutex_unlock(&cv_mutex);
    }
}

static void run_sequence(const char *label) {
    k_mutex_lock(&cv_mutex, K_FOREVER);
    k_condvar_signal(&red_cv);
    k_condvar_wait(&release_cv, &cv_mutex, K_FOREVER);
    k_mutex_unlock(&cv_mutex);

    k_mutex_lock(&cv_mutex, K_FOREVER);
    k_condvar_signal(&yellow_cv);
    k_condvar_wait(&release_cv, &cv_mutex, K_FOREVER);
    k_mutex_unlock(&cv_mutex);

    k_mutex_lock(&cv_mutex, K_FOREVER);
    k_condvar_signal(&green_cv);
    k_condvar_wait(&release_cv, &cv_mutex, K_FOREVER);
    k_mutex_unlock(&cv_mutex);

    uint64_t total = red_last_us + yellow_last_us + green_last_us;
    printk("[%s] R=%llu Y=%llu G=%llu Total=%llu\n", label, red_last_us, yellow_last_us, green_last_us, total);
}

static void seq_task(void *, void *, void *) {
    k_sleep(K_MSEC(200));
    debug_enabled = true;
    run_sequence("DEBUG=ON");
    k_sleep(K_MSEC(200));
    debug_enabled = false;
    run_sequence("DEBUG=OFF");
    while (1) {
        k_msleep(1000);
    }
}

K_THREAD_DEFINE(red_thread, STACKSIZE, red_task, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(yel_thread, STACKSIZE, yellow_task, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(grn_thread, STACKSIZE, green_task, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(seq_thread, STACKSIZE, seq_task, NULL, NULL, NULL, PRIORITY, 0, 0);

int main(void) {
    timing_init();
    init_leds();
    while (1) {
        k_msleep(1000);
    }
    return 0;
}
