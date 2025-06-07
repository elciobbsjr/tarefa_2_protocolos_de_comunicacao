// oled_display.h
#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <stdbool.h>
#include "ssd1306.h"

extern ssd1306_t display;

void oled_inicializar();
void oled_mensagem(const char *linha1, const char *linha2);
void atualizar_oled(bool led, float temp);

#endif
