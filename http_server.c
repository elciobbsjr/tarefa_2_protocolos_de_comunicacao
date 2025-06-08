// http_server.c
#include <stdio.h>
#include <string.h>
#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"
#include "oled_display.h"
#include "temperatura.h"
#include "hardware/gpio.h"

#define TCP_PORT 80
#define LED_GPIO 13

// Protótipos internos
static err_t servidor_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
static err_t servidor_aceitar(void *arg, struct tcp_pcb *newpcb, err_t err);

// Gera a página HTML de resposta dinâmica com status do LED e temperatura
int gerar_html(char *buffer, size_t max_len, bool estado_led, float temp) {
    return snprintf(buffer, max_len,
        "<!DOCTYPE html>"
        "<html lang='pt-BR'>"
        "<head><meta charset='UTF-8'><title>Controle Pico W</title>"
        "<style>"
            "body { font-family:Arial; text-align:center; margin-top:50px; background-color:black; color:white; }"
            ".botao { padding:10px 20px; font-size:16px; border:none; border-radius:5px; color:#fff; margin:10px; }"
            ".ligar { background-color:#28a745; }"
            ".desligar { background-color:#dc3545; }"
            ".card { padding:15px; border-radius:10px; box-shadow:0 2px 5px rgba(0,0,0,0.3); display:inline-block; background-color:#333; }"
        "</style>"
        "<script>"
        "function atualizarTemperatura() { fetch('/temp').then(resp => resp.text()).then(temp => { document.getElementById('temp').innerText = temp; }); }"
        "setInterval(atualizarTemperatura, 2000);"
        "window.onload = atualizarTemperatura;"
        "</script></head>"
        "<body>"
        "<div class='card'>"
            "<h3>Status do LED: %s</h3>"
            "<h3>Temperatura: <span id='temp'>%.2f</span> &deg;C</h3>"
            "<a href='/?led=on'><button class='botao ligar'>Ligar LED</button></a>"
            "<a href='/?led=off'><button class='botao desligar'>Desligar LED</button></a>"
        "</div>"
        "</body></html>",
        estado_led ? "Ligado" : "Desligado", temp);
}

// Processa requisições recebidas
static err_t servidor_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    if (!p) return tcp_close(pcb);

    char req[512] = {0}, resp[1024];
    pbuf_copy_partial(p, req, sizeof(req) - 1, 0);

    bool led_ativo = gpio_get(LED_GPIO);

    // Requisição para ler temperatura (AJAX)
    if (strstr(req, "GET /temp")) {
        float temp = ler_temperatura();
        printf("Temperatura atualizada: %.2f °C\n", temp); // Exibe temperatura no terminal
        atualizar_oled(led_ativo, temp);

        char resposta_temp[32];
        snprintf(resposta_temp, sizeof(resposta_temp), "%.2f", temp);

        char cabecalho[128];
        snprintf(cabecalho, sizeof(cabecalho),
                 "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n",
                 (int)strlen(resposta_temp));

        tcp_write(pcb, cabecalho, strlen(cabecalho), 0);
        tcp_write(pcb, resposta_temp, strlen(resposta_temp), 0);
        tcp_output(pcb);

        tcp_recved(pcb, p->tot_len);
        pbuf_free(p);
        return ERR_OK;
    }

    // Requisição para alterar estado do LED
    if (strstr(req, "GET /?led=on")) {
        gpio_put(LED_GPIO, 1);
        led_ativo = true;
        printf("LED ligado\n");
    } else if (strstr(req, "GET /?led=off")) {
        gpio_put(LED_GPIO, 0);
        led_ativo = false;
        printf("LED desligado\n");
    }

    // Atualiza temperatura e OLED
    float temp = ler_temperatura();
    printf("Temperatura atualizada: %.2f °C\n", temp); // Exibe temperatura ao carregar a página
    atualizar_oled(led_ativo, temp);

    // Gera HTML dinâmico
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

// Callback para aceitar novas conexões TCP
static err_t servidor_aceitar(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, servidor_recv);
    return ERR_OK;
}

// Inicializa o servidor HTTP e configura pino do LED
void iniciar_http_server() {
    gpio_init(LED_GPIO);
    gpio_set_dir(LED_GPIO, GPIO_OUT);

    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    tcp_bind(pcb, IP_ANY_TYPE, TCP_PORT);
    pcb = tcp_listen_with_backlog(pcb, 1);
    tcp_accept(pcb, servidor_aceitar);
}
