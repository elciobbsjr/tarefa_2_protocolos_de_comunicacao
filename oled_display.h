// oled_display.h
#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <stdbool.h>
#include "ssd1306.h"

// Estrutura global do display OLED
extern ssd1306_t display;

// Inicializa o display OLED via I2C
void oled_inicializar();

// Exibe duas linhas de texto no display
void oled_mensagem(const char *linha1, const char *linha2);

// Atualiza o display com status do LED e temperatura
void atualizar_oled(bool led, float temp);

#endif
