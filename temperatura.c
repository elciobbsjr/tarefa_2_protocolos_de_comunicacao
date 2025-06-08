// temperatura.c
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "temperatura.h"

// Função para ler e calcular a temperatura média usando o sensor interno
float ler_temperatura() {
    // Seleciona o canal ADC 4 (sensor interno)
    adc_select_input(4);

    float soma = 0.0f;

    // Realiza 100 leituras para obter uma média mais estável
    for (int i = 0; i < 100; i++) {
        uint16_t adc = adc_read();

        // Converte o valor lido do ADC para tensão (3.3V ref, 12 bits)
        float voltagem = adc * 3.3f / (1 << 12);

        // Calcula a temperatura em graus Celsius (fórmula do RP2040 datasheet)
        float temp = 27.0f - (voltagem - 0.706f) / 0.001721f;

        soma += temp;
        sleep_ms(1);  // Pequeno atraso para estabilidade
    }

    // Retorna a média das 100 leituras
    return soma / 100.0f;
}
