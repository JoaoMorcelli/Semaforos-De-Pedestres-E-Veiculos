#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>

#define SLEEP_TIME_VERDE 3000
#define SLEEP_TIME_VERMELHO 4000
#define SLEEP_TIME_AMARELO 1000

#define STACK_SIZE       1024
#define PRIORITY         0

// Define o LED usando Device Tree
#define LED2_NODE DT_ALIAS(led2)
#define LED0_NODE DT_ALIAS(led0)

K_MUTEX_DEFINE(mutexPedestre);
K_MUTEX_DEFINE(mutexCarro);

K_SEM_DEFINE(semCarroVerde,       0, 1);
K_SEM_DEFINE(semCarroAmarelo,     0, 1);
K_SEM_DEFINE(semCarroVermelho,    0, 1);
K_SEM_DEFINE(semPedestreVerde,    0, 1);
K_SEM_DEFINE(semPedestreVermelho, 0, 1);

static const struct gpio_dt_spec ledVermelho = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec ledVerde = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec sinal       = GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), sinal_gpios);

///Conectar os pinos PTD2 das duas placas
void threadPedestreVerde(void *arg1, void *arg2, void *arg3)
{
    while (1) {
        k_sem_take(&semPedestreVerde, K_FOREVER);
		gpio_pin_set_dt(&sinal, 1);
    }
}

void threadPedestreVermelho(void *arg1, void *arg2, void *arg3)
{
    while (1) {
        k_sem_take(&semPedestreVermelho, K_FOREVER);
		gpio_pin_set_dt(&sinal, 0);
    }
}

void threadCarroVermelho(void *arg1, void *arg2, void *arg3)
{
    while (1) {
        k_sem_take(&semCarroVermelho, K_FOREVER);

		gpio_pin_set_dt(&ledVermelho, 1);
		gpio_pin_set_dt(&ledVerde,    0);

		k_sem_give(&semPedestreVerde);
		k_msleep(SLEEP_TIME_VERMELHO);

		k_sem_give(&semCarroVerde);
    }
}

void threadCarroVerde(void *arg1, void *arg2, void *arg3)
{
    while (1) {
        k_sem_take(&semCarroVerde, K_FOREVER);

		gpio_pin_set_dt(&ledVermelho, 0);
		gpio_pin_set_dt(&ledVerde,    1);

		k_sem_give(&semPedestreVermelho);
		k_msleep(SLEEP_TIME_VERDE);

		k_sem_give(&semCarroAmarelo);
    }
}

void threadCarroAmarelo(void *arg1, void *arg2, void *arg3)
{
    while (1) {
        k_sem_take(&semCarroAmarelo, K_FOREVER);

		gpio_pin_set_dt(&ledVermelho, 1);
		gpio_pin_set_dt(&ledVerde,    1);

		k_sem_give(&semPedestreVermelho);
		k_msleep(SLEEP_TIME_AMARELO);

		k_sem_give(&semCarroVermelho);
    }
}

K_THREAD_DEFINE(tdPedestreVerde, STACK_SIZE, threadPedestreVerde,
                NULL, NULL, NULL,
                PRIORITY, 0, 0);

K_THREAD_DEFINE(tdPedestreVermelho, STACK_SIZE, threadPedestreVermelho,
                NULL, NULL, NULL,
                PRIORITY, 0, 0);

K_THREAD_DEFINE(tdCarroVermelho, STACK_SIZE, threadCarroVermelho,
                NULL, NULL, NULL,
                PRIORITY, 0, 0);

K_THREAD_DEFINE(tdCarroVerde, STACK_SIZE, threadCarroVerde,
                NULL, NULL, NULL,
                PRIORITY, 0, 0);

K_THREAD_DEFINE(tdCarroAmarelo, STACK_SIZE, threadCarroAmarelo,
                NULL, NULL, NULL,
                PRIORITY, 0, 0);

int main(void)
{
	if (!gpio_is_ready_dt(&ledVermelho) ||
	    !gpio_is_ready_dt(&ledVerde) ||
	    !gpio_is_ready_dt(&sinal)) {
		return 0;
	}

	gpio_pin_configure_dt(&ledVermelho, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&ledVerde,    GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&sinal,       GPIO_OUTPUT_INACTIVE);

	k_sem_give(&semCarroVerde);

	return 0;
}