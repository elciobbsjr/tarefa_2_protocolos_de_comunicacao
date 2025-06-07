// oled_display.c
#include <stdio.h>
#include <string.h>
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "ssd1306.h"
#include "ssd1306_i2c.h"
#include "oled_display.h"

#define I2C_PORT i2c1
#define SDA_PIN 14
#define SCL_PIN 15
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

ssd1306_t display;

void oled_inicializar() {
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    ssd1306_init_bm(&display, OLED_WIDTH, OLED_HEIGHT, false, ssd1306_i2c_address, I2C_PORT);
    ssd1306_config(&display);
    ssd1306_init();
}

void oled_mensagem(const char *linha1, const char *linha2) {
    memset(display.ram_buffer + 1, 0, display.bufsize - 1);
    ssd1306_draw_string(display.ram_buffer + 1, 0, 0, linha1);
    ssd1306_draw_string(display.ram_buffer + 1, 0, 16, linha2);

    ssd1306_command(&display, ssd1306_set_column_address);
    ssd1306_command(&display, 0);
    ssd1306_command(&display, OLED_WIDTH - 1);
    ssd1306_command(&display, ssd1306_set_page_address);
    ssd1306_command(&display, 0);
    ssd1306_command(&display, (OLED_HEIGHT / 8) - 1);
    ssd1306_send_data(&display);
}

void atualizar_oled(bool led, float temp) {
    char linha1[22], linha2[22];
    snprintf(linha1, sizeof(linha1), "LED: %s", led ? "Ligado" : "Desligado");
    snprintf(linha2, sizeof(linha2), "Temp: %.2f C", temp);
    oled_mensagem(linha1, linha2);
}
