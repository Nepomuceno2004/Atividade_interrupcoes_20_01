#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

#define IS_RGBW false
#define NUM_PIXELS 25
#define WS2812_PIN 7
#define tempo 100
#define pinLed 13
#define button_A 5
#define button_B 6

static volatile uint32_t last_time = 0; // Armazena o tempo do último evento (em microssegundos)

// Variável global para armazenar a cor (Entre 0 e 255 para intensidade)
uint8_t led_r = 25; // Intensidade do vermelho
uint8_t led_g = 0; // Intensidade do verde
uint8_t led_b = 25; // Intensidade do azul

int numero_exibido = 0;

// Buffer para armazenar quais LEDs estão ligados matriz 5x5
bool numero_0[NUM_PIXELS] = {
    1, 1, 1, 1, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
    1, 1, 1, 1, 1};

bool numero_1[NUM_PIXELS] = {
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0,
    0, 0, 1, 0, 0};

bool numero_2[NUM_PIXELS] = {
    1, 1, 1, 1, 1,
    1, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
    0, 0, 0, 0, 1,
    1, 1, 1, 1, 1};

bool numero_3[NUM_PIXELS] = {
    1, 1, 1, 1, 1,
    0, 0, 0, 0, 1,
    1, 1, 1, 1, 1,
    0, 0, 0, 0, 1,
    1, 1, 1, 1, 1};

bool numero_4[NUM_PIXELS] = {
    1, 0, 0, 0, 0,
    0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 
    1, 0, 0, 0, 1,
    1, 0, 0, 0, 1,
};

bool numero_5[NUM_PIXELS] = {
    1, 1, 1, 1, 1,
    0, 0, 0, 0, 1,
    1, 1, 1, 1, 1,
    1, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
};

bool numero_6[NUM_PIXELS] = {
    1, 1, 1, 1, 1,
    1, 0, 0, 0, 1,
    1, 1, 1, 1, 1,
    1, 0, 0, 0, 0,
    1, 1, 1, 1, 1,
};

bool numero_7[NUM_PIXELS] = {
    0, 0, 0, 1, 0,
    0, 0, 1, 0, 0,
    0, 1, 0, 0, 0,
    0, 0, 0, 0, 1,
    1, 1, 1, 1, 1,
};

bool numero_8[NUM_PIXELS] = {
    1, 1, 1, 1, 1,
    1, 0, 0, 0, 1,
    1, 1, 1, 1, 1,
    1, 0, 0, 0, 1,
    1, 1, 1, 1, 1,
};

bool numero_9[NUM_PIXELS] = {
    1, 1, 1, 1, 1,
    0, 0, 0, 0, 1,
    1, 1, 1, 1, 1,
    1, 0, 0, 0, 1,
    1, 1, 1, 1, 1,
};

//Envia a cor de um LED para a fila de transmissão.
static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

//Converte valores RGB para o formato de 32 bits utilizado pelos LEDs WS2812.
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

//Atualiza os LEDs da matriz de acordo com o número a ser exibido.
void set_one_led(uint8_t r, uint8_t g, uint8_t b, bool led_buffer[])
{
    // Define a cor com base nos parâmetros fornecidos
    uint32_t color = urgb_u32(r, g, b);

    // Define todos os LEDs com a cor especificada
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        if (led_buffer[i])
        {
            put_pixel(color); // Liga o LED com um no buffer
        }
        else
        {
            put_pixel(0); // Desliga os LEDs com zero no buffer
        }
    }
}

//Retorna a matriz correspondente ao número desejado.
bool *escolha_desenho(int num)
{
    switch (num)
    {
    case 0:
        return numero_0;
    case 1:
        return numero_1;
    case 2:
        return numero_2;
    case 3:
        return numero_3;
    case 4:
        return numero_4;
    case 5:
        return numero_5;
    case 6:
        return numero_6;
    case 7:
        return numero_7;
    case 8:
        return numero_8;
    case 9:
        return numero_9;
    default:
        return NULL;
    }
}

void gpio_irq_handler(uint gpio, uint32_t events)
{
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    if (current_time - last_time > 200000) // 200ms de debounce
    {
        last_time = current_time;

        if (gpio == button_A && numero_exibido < 9)
        {
            numero_exibido++;
        }
        else if (gpio == button_B && numero_exibido > 0)
        {
            numero_exibido--;
        }

        set_one_led(led_r, led_g, led_b, escolha_desenho(numero_exibido));
    }
}

int main(){
    stdio_init_all();

    gpio_init(pinLed);
    gpio_set_dir(pinLed, GPIO_OUT);

    gpio_init(button_A);
    gpio_set_dir(button_A, GPIO_IN);
    gpio_pull_up(button_A);

    gpio_init(button_B);
    gpio_set_dir(button_B, GPIO_IN);
    gpio_pull_up(button_B);

    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    set_one_led(led_r, led_g, led_b, escolha_desenho(numero_exibido));
    
    gpio_set_irq_enabled_with_callback(button_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(button_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    while (1)
    {
        gpio_put(pinLed, true);
        sleep_ms(tempo); //100ms

        gpio_put(pinLed, false);
        sleep_ms(tempo); //100ms

        //1 ciclo, ligar e desligar, a cada 200ms, ou seja, 5 ciclos a cada 1s
    }

    return 0;
}