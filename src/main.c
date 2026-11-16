#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#define SLEEP_TIME_VERDE 4000
#define SLEEP_TIME_VERMELHO 4000
#define STACK_SIZE       1024
#define PRIORITY         0

// Define o LED usando Device Tree
#define LED2_NODE DT_ALIAS(led2)
#define LED0_NODE DT_ALIAS(led0)

K_MUTEX_DEFINE(mutex);

static const struct gpio_dt_spec ledVermelho = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec ledVerde = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

void threadLedVerde(void *arg1, void *arg2, void *arg3)
{
    while (1) {
        int retVerde;
        retVerde = gpio_pin_configure_dt(&ledVerde, GPIO_OUTPUT_ACTIVE);
        k_mutex_lock(&mutex, K_FOREVER);
        gpio_pin_set_dt(&ledVerde, 0);
        k_msleep(SLEEP_TIME_VERDE);
        gpio_pin_toggle_dt(&ledVerde);
        k_msleep(SLEEP_TIME_VERDE);
        gpio_pin_toggle_dt(&ledVerde);
        k_mutex_unlock(&mutex);
    }
}

void threadLedVermelho(void *arg1, void *arg2, void *arg3)
{
    while (1) {
        int retVermelho;
        retVermelho = gpio_pin_configure_dt(&ledVermelho, GPIO_OUTPUT_ACTIVE);
        k_mutex_lock(&mutex, K_FOREVER);
        gpio_pin_set_dt(&ledVermelho, 1);
        k_msleep(SLEEP_TIME_VERMELHO);
        gpio_pin_toggle_dt(&ledVermelho);
        k_msleep(SLEEP_TIME_VERMELHO);
        gpio_pin_toggle_dt(&ledVermelho);
        k_mutex_unlock(&mutex);
    }
}

K_THREAD_DEFINE(threadVerde, STACK_SIZE, threadLedVerde,
                NULL, NULL, NULL,
                PRIORITY, 0, 0);

K_THREAD_DEFINE(threadVermelho, STACK_SIZE, threadLedVermelho,
                NULL, NULL, NULL,
                PRIORITY, 0, 0);