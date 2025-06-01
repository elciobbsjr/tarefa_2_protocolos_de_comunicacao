#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"
#include "dhcpserver.h"
#include "dnsserver.h"
#include "ssd1306.h"
#include "ssd1306_i2c.h"
#include "ssd1306_font.h"

#define TCP_PORT 80
#define LED_GPIO 13

#define I2C_PORT i2c1
#define SDA_PIN 14
#define SCL_PIN 15
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

ssd1306_t display;

// Função que converte a leitura ADC para temperatura em Celsius
float ler_temperatura() {
    adc_select_input(4);
    uint16_t adc = adc_read();
    float voltagem = adc * 3.3f / (1 << 12);
    return 27.0f - (voltagem - 0.706f) / 0.001721f;
}

// Exibe no display o status do LED e temperatura
void atualizar_oled(bool led, float temp) {
    char linha1[22], linha2[22];
    snprintf(linha1, sizeof(linha1), "LED: %s", led ? "Ligado" : "Desligado");
    snprintf(linha2, sizeof(linha2), "Temp: %.2f C", temp);

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

// Gera a página HTML
int gerar_html(char *buffer, size_t max_len, bool estado_led, float temp) {
    return snprintf(buffer, max_len,
        "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Controle Pico W</title></head>"
        "<body style='text-align:center;font-family:Arial;'>"
        "<h1>Servidor Pico W</h1>"
        "<p>Status do LED: <strong>%s</strong></p>"
        "<p>Temperatura: <strong>%.2f &deg;C</strong></p>"
        "<a href='/?led=on'><button style='padding:15px;'>Ligar LED</button></a><br><br>"
        "<a href='/?led=off'><button style='padding:15px;'>Desligar LED</button></a>"
        "</body></html>",
        estado_led ? "Ligado" : "Desligado", temp);
}

// Callback HTTP
err_t servidor_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    if (!p) return tcp_close(pcb);

    char req[512] = {0}, resp[1024];
    pbuf_copy_partial(p, req, sizeof(req) - 1, 0);

    bool led_ativo = gpio_get(LED_GPIO);
    if (strstr(req, "GET /?led=on")) {
        gpio_put(LED_GPIO, 1);
        led_ativo = true;
        printf("[HTTP] LED ligado\n");
    } else if (strstr(req, "GET /?led=off")) {
        gpio_put(LED_GPIO, 0);
        led_ativo = false;
        printf("[HTTP] LED desligado\n");
    }

    float temp = ler_temperatura();
    printf("[HTTP] Temperatura atual: %.2f C\n", temp);
    atualizar_oled(led_ativo, temp);

    int len = gerar_html(resp, sizeof(resp), led_ativo, temp);
    char cabecalho[128];
    snprintf(cabecalho, sizeof(cabecalho),
             "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n", len);

    tcp_write(pcb, cabecalho, strlen(cabecalho), 0);
    tcp_write(pcb, resp, len, 0);
    tcp_output(pcb);

    tcp_recved(pcb, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}

err_t servidor_aceitar(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, servidor_recv);
    printf("[HTTP] Nova conexão aceita.\n");
    return ERR_OK;
}

int main() {
    stdio_init_all();
    sleep_ms(3000);

    // Inicializa GPIO e ADC
    gpio_init(LED_GPIO);
    gpio_set_dir(LED_GPIO, GPIO_OUT);
    adc_init();
    adc_set_temp_sensor_enabled(true);

    // Inicializa barramento I2C e display
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    ssd1306_init_bm(&display, OLED_WIDTH, OLED_HEIGHT, false, ssd1306_i2c_address, I2C_PORT);
    ssd1306_config(&display);
    ssd1306_init();

    // Mensagem inicial no OLED
    memset(display.ram_buffer + 1, 0, display.bufsize - 1);
    ssd1306_draw_string(display.ram_buffer + 1, 0, 0, "Iniciando Wi-Fi...");
    ssd1306_command(&display, ssd1306_set_column_address);
    ssd1306_command(&display, 0);
    ssd1306_command(&display, OLED_WIDTH - 1);
    ssd1306_command(&display, ssd1306_set_page_address);
    ssd1306_command(&display, 0);
    ssd1306_command(&display, (OLED_HEIGHT / 8) - 1);
    ssd1306_send_data(&display);

    // Inicializa Wi-Fi como AP
    if (cyw43_arch_init()) return 1;
    cyw43_arch_enable_ap_mode("picow_test", "password", CYW43_AUTH_WPA2_AES_PSK);

    ip4_addr_t ip, mask;
    IP4_ADDR(&ip, 192, 168, 4, 1);
    IP4_ADDR(&mask, 255, 255, 255, 0);

    dhcp_server_t dhcp_server;
    dhcp_server_init(&dhcp_server, &ip, &mask);

    dns_server_t dns_server;
    dns_server_init(&dns_server, &ip);

    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb || tcp_bind(pcb, IP_ANY_TYPE, TCP_PORT) != ERR_OK) {
        printf("[ERRO] Falha ao iniciar servidor.\n");
        return 1;
    }

    pcb = tcp_listen_with_backlog(pcb, 1);
    tcp_accept(pcb, servidor_aceitar);

    printf("[INFO] Acesse: http://192.168.4.1\n");

    // Mensagem final no OLED
    memset(display.ram_buffer + 1, 0, display.bufsize - 1);
    ssd1306_draw_string(display.ram_buffer + 1, 0, 0, "Acesse:");
    ssd1306_draw_string(display.ram_buffer + 1, 0, 16, "192.168.4.1");
    ssd1306_command(&display, ssd1306_set_column_address);
    ssd1306_command(&display, 0);
    ssd1306_command(&display, OLED_WIDTH - 1);
    ssd1306_command(&display, ssd1306_set_page_address);
    ssd1306_command(&display, 0);
    ssd1306_command(&display, (OLED_HEIGHT / 8) - 1);
    ssd1306_send_data(&display);

    while (true) {
        cyw43_arch_poll();
        sleep_ms(10);
    }
}
