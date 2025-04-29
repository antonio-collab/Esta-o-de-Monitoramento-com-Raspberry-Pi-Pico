#include <stdio.h>
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"

// Pinos
const int vRx = 26;
const int vRy = 27;
const int SW = 22;
const int led_r = 13;
const int led_g = 11;
const int led_b = 12;
const int buzzer = 21;

// Canais ADC
const int ADC_CHANNEL_0 = 0;
const int ADC_CHANNEL_1 = 1;

// Estados
volatile uint16_t flag_estado = 0;

// Limiares
const uint16_t LIMIAR_BAIXO = 1000;
const uint16_t LIMIAR_MODERADO = 2000;
const uint16_t LIMIAR_ALTO = 3000;

#define BUZZER_FREQUENCY 2000  // 2kHz

// ------ SUAS FUNÇÕES: PWM e BEEP ------
void pwm_init_buzzer(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (BUZZER_FREQUENCY * 4096));
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(pin, 0); // Começa desligado
}

void beep(uint pin, uint duration_ms) {
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_set_gpio_level(pin, 2048); // 50% de 4096
    sleep_ms(duration_ms);
    pwm_set_gpio_level(pin, 0);
    sleep_ms(100);
}
// --------------------------------------

// Configura joystick
void setup_joystick() {
    adc_init();
    adc_gpio_init(vRx);
    adc_gpio_init(vRy);
    gpio_init(SW);
    gpio_set_dir(SW, GPIO_IN);
    gpio_pull_up(SW);
}

// Configura LEDs e buzzer
void setup_saida() {
    gpio_init(led_r);
    gpio_init(led_g);
    gpio_init(led_b);
    gpio_set_dir(led_r, GPIO_OUT);
    gpio_set_dir(led_g, GPIO_OUT);
    gpio_set_dir(led_b, GPIO_OUT);
    pwm_init_buzzer(buzzer);
}

void setup() {
    stdio_init_all();
    setup_joystick();
    setup_saida();
}

// Lê joystick
void joystick_read_axis(uint16_t *eixo_x, uint16_t *eixo_y) {
    adc_select_input(ADC_CHANNEL_0);
    sleep_us(2);
    *eixo_x = adc_read();
    adc_select_input(ADC_CHANNEL_1);
    sleep_us(2);
    *eixo_y = adc_read();
}

// Função core 1
void core1_entry() {
    uint32_t comando_recebido = 0;
    while (1) {
        comando_recebido = multicore_fifo_pop_blocking();
        if (comando_recebido > LIMIAR_ALTO) {
            gpio_put(led_r, 1);
            gpio_put(led_g, 0);
            gpio_put(led_b, 0);
            beep(buzzer, 1000); // Alarme longo
        } else if (comando_recebido > LIMIAR_MODERADO) {
            gpio_put(led_r, 0);
            gpio_put(led_g, 0);
            gpio_put(led_b, 1);
        } else {
            gpio_put(led_r, 0);
            gpio_put(led_g, 1);
            gpio_put(led_b, 0);
        }
    }
}

// Timer
bool alarme_callback(repeating_timer_t *rt) {
    multicore_fifo_push_blocking(flag_estado);
    return true;
}

// Main
int main() {
    setup();
    printf("Estacao de Monitoramento Iniciada!\n");
    multicore_launch_core1(core1_entry);
    repeating_timer_t timer;
    add_repeating_timer_ms(2000, alarme_callback, NULL, &timer);

    uint16_t valor_x, valor_y;
    while (1) {
        joystick_read_axis(&valor_x, &valor_y);
        flag_estado = valor_x;
        printf("Eixo X: %d, Eixo Y: %d, Estado Atual: %d\n", valor_x, valor_y, flag_estado);
        sleep_ms(200);
    }
}