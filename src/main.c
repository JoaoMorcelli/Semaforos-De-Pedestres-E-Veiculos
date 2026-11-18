// Programa do microcontrolador Mestre
// Controla o LED interno (RGB) e os 5 LEDs externos (3 de veículo + 2 de pedestre)

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>

#define ModoNormal  1
#define ModoNoturno 2
#define ModoBotao 3

#ifndef ModoAtual
#define ModoAtual ModoNoturno
#endif

#if ModoAtual == ModoNormal

#define SLEEP_TIME_VERDE    3000
#define SLEEP_TIME_VERMELHO 4000
#define SLEEP_TIME_AMARELO  1000

#elif ModoAtual == ModoNoturno

// Amarelo piscando: 1000 ms aceso, 1000 ms apagado
#define Pisca 1000

#endif

#define STACK_SIZE 1024
#define PRIORITY   0

// Define o LED usando Device Tree
#define LED2_NODE DT_ALIAS(led2)
#define LED0_NODE DT_ALIAS(led0)
#define BUTTON_NODE DT_ALIAS(sw0)



static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);
static struct gpio_callback button_cb_data;
static int64_t last_button_time;


// LED interno (como no código original)
static const struct gpio_dt_spec ledVermelho = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec ledVerde    = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec sinal       = GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), sinal_gpios);

// LEDs externos, definidos no overlay (boards/frdm_kl25z.overlay)
static const struct gpio_dt_spec extVeicVermelho = GPIO_DT_SPEC_GET(DT_NODELABEL(veic_vermelho), gpios);
static const struct gpio_dt_spec extVeicAmarelo  = GPIO_DT_SPEC_GET(DT_NODELABEL(veic_amarelo),  gpios);
static const struct gpio_dt_spec extVeicVerde    = GPIO_DT_SPEC_GET(DT_NODELABEL(veic_verde),    gpios);
static const struct gpio_dt_spec extPedVermelho  = GPIO_DT_SPEC_GET(DT_NODELABEL(ped_vermelho),  gpios);
static const struct gpio_dt_spec extPedVerde     = GPIO_DT_SPEC_GET(DT_NODELABEL(ped_verde),     gpios);

// Define o estado dos 5 LEDs externos de uma vez
static void setExternos(int veicVermelho, int veicAmarelo, int veicVerde,
                        int pedVermelho, int pedVerde)
{
    gpio_pin_set_dt(&extVeicVermelho, veicVermelho);
    gpio_pin_set_dt(&extVeicAmarelo,  veicAmarelo);
    gpio_pin_set_dt(&extVeicVerde,    veicVerde);
    gpio_pin_set_dt(&extPedVermelho,  pedVermelho);
    gpio_pin_set_dt(&extPedVerde,     pedVerde);
}

// Define o LED interno (vermelho, verde) e, a partir dele, os LEDs externos.
// Este é o ÚNICO lugar onde está a relação interno -> externos.
static void setLeds(int vermelho, int verde)
{
    gpio_pin_set_dt(&ledVermelho, vermelho);
    gpio_pin_set_dt(&ledVerde,    verde);

    if (vermelho && verde) {
        // Interno AMARELO  -> veículo vermelho, pedestre vermelho
        setExternos(1, 0, 0,   0, 1);
    } else if (verde) {
        // Interno VERDE    -> veículo vermelho, pedestre vermelho
        setExternos(1, 0, 0,   0, 1);
    } else if (vermelho) {
        // Interno VERMELHO -> veículo verde, pedestre verde
        setExternos(0, 0, 1,   1, 0);
    } else {
        // Interno apagado -> externos apagados
        setExternos(0, 0, 0,   0, 0);
    }
}


#if ModoAtual == ModoNormal

K_MUTEX_DEFINE(mutexPedestre);
K_MUTEX_DEFINE(mutexCarro);

K_SEM_DEFINE(semCarroVerde,       0, 1);
K_SEM_DEFINE(semCarroAmarelo,     0, 1);
K_SEM_DEFINE(semCarroVermelho,    0, 1);
K_SEM_DEFINE(semPedestreVerde,    0, 1);
K_SEM_DEFINE(semPedestreVermelho, 0, 1);

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

        setLeds(1, 0);

        k_sem_give(&semPedestreVerde);
        k_msleep(3000);
        setExternos(0, 1, 0,   1, 0);
        k_msleep(1000);
        setExternos(0, 0, 0,   1, 0);
        k_sem_give(&semCarroVerde);
    }
}

void threadCarroVerde(void *arg1, void *arg2, void *arg3)
{
    while (1) {
        k_sem_take(&semCarroVerde, K_FOREVER);

        setLeds(0, 1);

        k_sem_give(&semPedestreVermelho);
        k_msleep(SLEEP_TIME_VERDE);

        k_sem_give(&semCarroAmarelo);
    }
}

void threadCarroAmarelo(void *arg1, void *arg2, void *arg3)
{
    while (1) {
        k_sem_take(&semCarroAmarelo, K_FOREVER);

        setLeds(1, 1);

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

#elif ModoAtual == ModoNoturno

void threadModoNoturno(void *arg1, void *arg2, void *arg3)
{
    while (1) {
        setLeds(1, 1);
        setExternos(0, 1, 0,   1, 0);
        k_msleep(Pisca);
        setExternos(0, 0, 0,   0, 0);
        setLeds(0, 0);
        k_msleep(Pisca);
    }
}

K_THREAD_DEFINE(tdModoNoturno, STACK_SIZE, threadModoNoturno,
                NULL, NULL, NULL,
                PRIORITY, 0, 0);


#elif ModoAtual == ModoBotao

K_SEM_DEFINE(semBotao, 0, 1);
#define TEMPO_DE_TRAVESSIA 3000

// Botao deve ser conectado à porta PTA16 e no terra da placa mestre
void button_isr(const struct device *dev,
                struct gpio_callback *cb,
                uint32_t pins)
{
    int64_t now = k_uptime_get();

    if ((pins & BIT(button.pin)) && (now - last_button_time >= 200)) {
        last_button_time = now;
        k_sem_give(&semBotao);
    }
}

void threadBotao(void *arg1, void *arg2, void *arg3)
{
	while (1) {
		k_sem_take(&semBotao, K_FOREVER);

		setLeds(1, 1);

		k_msleep(1000);

		setLeds(1, 0);
		gpio_pin_set_dt(&sinal, 1);

		k_msleep(TEMPO_DE_TRAVESSIA);

		setLeds(0, 1);
		gpio_pin_set_dt(&sinal, 0);
	}
}

K_THREAD_DEFINE(tdBotao, STACK_SIZE, threadBotao,
				NULL, NULL, NULL,
				PRIORITY, 0, 0);

#endif
int main(void)
{
    if (!gpio_is_ready_dt(&ledVermelho)     ||
        !gpio_is_ready_dt(&ledVerde)        ||
        !gpio_is_ready_dt(&sinal)           ||
        !gpio_is_ready_dt(&extVeicVermelho) ||
        !gpio_is_ready_dt(&extVeicAmarelo)  ||
        !gpio_is_ready_dt(&extVeicVerde)    ||
        !gpio_is_ready_dt(&extPedVermelho)  ||
        !gpio_is_ready_dt(&extPedVerde)) {
        return 0;
    }

    gpio_pin_configure_dt(&ledVermelho,     GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&ledVerde,        GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&sinal,           GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&extVeicVermelho, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&extVeicAmarelo,  GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&extVeicVerde,    GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&extPedVermelho,  GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&extPedVerde,     GPIO_OUTPUT_INACTIVE);

#if ModoAtual == ModoBotao
	if (!gpio_is_ready_dt(&button)) {
		return 0;
	}

	gpio_pin_configure_dt(&button, GPIO_INPUT);
	// Ordem segura: inicializa o callback antes de adicioná-lo
	gpio_init_callback(&button_cb_data, button_isr, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb_data);
	gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);

	setLeds(0, 1);
	gpio_pin_set_dt(&sinal, 0);
#endif

#if ModoAtual == ModoNormal
    k_sem_give(&semCarroVerde);
#endif

    return 0;
}

/*
//Programa do microcontrolador Escravo
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#define ModoNormal  1
#define ModoNoturno 2

#ifndef ModoAtual
#define ModoAtual ModoNormal
#endif

// Barra a compilacao se o valor do modo for invalido 
#if (ModoAtual != ModoNormal) && (ModoAtual != ModoNoturno)
    #error "ModoAtual invalido! Use ModoNormal ou ModoNoturno."
#endif

static const struct gpio_dt_spec ledVerde    = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec ledVermelho = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
static const struct gpio_dt_spec sinal       = GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), sinal_gpios);

#if ModoAtual == ModoNoturno
#define Pisca 1000
#endif

int main(void)
{
    if (!gpio_is_ready_dt(&ledVerde) ||
        !gpio_is_ready_dt(&ledVermelho) ||
        !gpio_is_ready_dt(&sinal)) {
        return 0;
    }

    gpio_pin_configure_dt(&ledVerde,    GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&ledVermelho, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&sinal,       GPIO_INPUT);
    
#if ModoAtual == ModoNormal

    while (1) {
        if (gpio_pin_get_dt(&sinal) > 0) {
            // Mestre vermelho: pedestre verde 
            gpio_pin_set_dt(&ledVermelho, 0);
            gpio_pin_set_dt(&ledVerde, 1);
        } else {
            // Mestre verde/amarelo: pedestre vermelho 
            gpio_pin_set_dt(&ledVerde, 0);
            gpio_pin_set_dt(&ledVermelho, 1);
        }

        int v = gpio_pin_get_dt(&sinal);
        printk("sinal = %d\n", v);

        k_msleep(10);
    }

#elif ModoAtual == ModoNoturno

    while (1) {
        gpio_pin_set_dt(&ledVermelho, 1);
        k_msleep(Pisca);
        gpio_pin_set_dt(&ledVermelho, 0);
        k_msleep(Pisca);
    }

#endif

    return 0;
}
*/