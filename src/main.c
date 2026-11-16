/*#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>

#define SLEEP_TIME_VERDE 4000
#define SLEEP_TIME_VERMELHO 4000
#define STACK_SIZE       1024
#define PRIORITY         0

// Define o LED usando Device Tree
#define LED2_NODE DT_ALIAS(led2)
#define LED0_NODE DT_ALIAS(led0)

K_MUTEX_DEFINE(mutexPedestre);
K_MUTEX_DEFINE(mutexCarro);

static const struct gpio_dt_spec ledVermelho = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec ledVerde = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec sinal       = GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), sinal_gpios);
static const struct device *uart_dev = DEVICE_DT_GET(DT_NODELABEL(uart0));

void threadPedestreVerde(void *arg1, void *arg2, void *arg3)
{
    while (1) {
        int retVerde;
        retVerde = gpio_pin_configure_dt(&ledVerde, GPIO_OUTPUT_ACTIVE);

        k_mutex_lock(&mutexPedestre, K_FOREVER);

        gpio_pin_set_dt(&ledVerde, 0);

        k_msleep(SLEEP_TIME_VERDE);

        gpio_pin_toggle_dt(&ledVerde);

        k_msleep(SLEEP_TIME_VERDE);

        gpio_pin_toggle_dt(&ledVerde);

        k_mutex_unlock(&mutexPedestre);
    }
}

void threadPedestreVermelho(void *arg1, void *arg2, void *arg3)
{
    while (1) {
        int retVermelho;

        retVermelho = gpio_pin_configure_dt(&ledVermelho, GPIO_OUTPUT_ACTIVE);
       
        k_mutex_lock(&mutexPedestre, K_FOREVER);

        gpio_pin_set_dt(&ledVermelho, 1);

        k_msleep(SLEEP_TIME_VERMELHO);

        gpio_pin_toggle_dt(&ledVermelho);

        k_msleep(SLEEP_TIME_VERMELHO);

        gpio_pin_toggle_dt(&ledVermelho);

        k_mutex_unlock(&mutexPedestre);
    }
}

void threadCarroVermelho(void *arg1, void *arg2, void *arg3)
{
    while (1) {
        k_mutex_lock(&mutexCarro, K_FOREVER);

        gpio_pin_set_dt(&sinal, 0);

        k_msleep(SLEEP_TIME_VERMELHO);

        gpio_pin_toggle_dt(&ledVermelho);

        k_msleep(SLEEP_TIME_VERMELHO);

        gpio_pin_toggle_dt(&ledVermelho);

        k_mutex_unlock(&mutexCarro);
    }
}

void threadCarroVerde(void *arg1, void *arg2, void *arg3)
{
    while (1) {
        k_mutex_lock(&mutex, K_FOREVER);

        gpio_pin_set_dt(&ledVerde, 0);

        k_msleep(SLEEP_TIME_VERDE);

        gpio_pin_toggle_dt(&ledVerde);

        k_msleep(SLEEP_TIME_VERDE);

        gpio_pin_toggle_dt(&ledVerde);

        k_mutex_unlock(&mutex);
    }
}

void threadCarroAmarelo(void *arg1, void *arg2, void *arg3)
{
    while (1) {
        k_mutex_lock(&mutex, K_FOREVER);

        gpio_pin_set_dt(&ledVerde, 0);

        k_msleep(SLEEP_TIME_VERDE);

        gpio_pin_toggle_dt(&ledVerde);

        k_msleep(SLEEP_TIME_VERDE);

        gpio_pin_toggle_dt(&ledVerde);

        k_mutex_unlock(&mutex);
    }
}

K_THREAD_DEFINE(threadVerde, STACK_SIZE, threadPedestreVerde,
                NULL, NULL, NULL,
                PRIORITY, 0, 0);

K_THREAD_DEFINE(threadVermelho, STACK_SIZE, threadPedestreVermelho,
                NULL, NULL, NULL,
                PRIORITY, 0, 0);

K_THREAD_DEFINE(threadCarroVermelho, STACK_SIZE, threadCarroVermelho,
                NULL, NULL, NULL,
                PRIORITY, 0, 0);

K_THREAD_DEFINE(threadCarroVerde, STACK_SIZE, threadCarroVerde,
                NULL, NULL, NULL,
                PRIORITY, 0, 0);

K_THREAD_DEFINE(threadCarroAmarelo, STACK_SIZE, threadCarroAmarelo,
                NULL, NULL, NULL,
                PRIORITY, 0, 0);
*/

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>


#define TEMPO_VERDE     3000
#define TEMPO_AMARELO   1000
#define TEMPO_VERMELHO  4000


static const struct gpio_dt_spec ledVerde    = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec ledVermelho = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
static const struct gpio_dt_spec sinal       = GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), sinal_gpios);


static void set_leds(int vermelho, int verde)
{
    gpio_pin_set_dt(&ledVermelho, vermelho);
    gpio_pin_set_dt(&ledVerde, verde);
}


int main(void)
{
    if (!gpio_is_ready_dt(&ledVerde) ||
        !gpio_is_ready_dt(&ledVermelho) ||
        !gpio_is_ready_dt(&sinal)) {
        return 0;
    }


    gpio_pin_configure_dt(&ledVerde, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&ledVermelho, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&sinal, GPIO_OUTPUT_INACTIVE);


    while (1) {
        /* Veículo verde (3 s), pedestre vermelho */
        gpio_pin_set_dt(&sinal, 0);
        set_leds(0, 1);
        k_msleep(TEMPO_VERDE);


        /* Veículo amarelo (1 s), pedestre ainda vermelho */
        set_leds(1, 1);
        k_msleep(TEMPO_AMARELO);


        /* Veículo vermelho (4 s), pedestre verde */
        set_leds(1, 0);
        gpio_pin_set_dt(&sinal, 1);
        k_msleep(TEMPO_VERMELHO);
    }
}
