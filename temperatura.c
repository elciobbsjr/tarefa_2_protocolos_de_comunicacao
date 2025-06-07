// temperatura.c
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "temperatura.h"

float ler_temperatura() {
    adc_select_input(4);
    float soma = 0.0f;

    for (int i = 0; i < 100; i++) {
        uint16_t adc = adc_read();
        float voltagem = adc * 3.3f / (1 << 12);
        float temp = 27.0f - (voltagem - 0.706f) / 0.001721f;
        soma += temp;
        sleep_ms(1);
    }

    return soma / 100.0f;
}
