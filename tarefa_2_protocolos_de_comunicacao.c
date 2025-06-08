#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"
#include "dhcpserver.h"
#include "dnsserver.h"
#include "oled_display.h"
#include "http_server.h"

int main() {
    // Inicializa sistema de I/O padrão e aguarda para garantir que tudo está pronto
    stdio_init_all();
    sleep_ms(3000);

    // Inicializa ADC e sensor de temperatura
    adc_init();
    adc_set_temp_sensor_enabled(true);

    // Inicializa o display OLED e exibe status
    oled_inicializar();
    oled_mensagem("Iniciando Wi-Fi...", "");

    // Inicializa o chip Wi-Fi em modo Access Point
    cyw43_arch_init();
    cyw43_arch_enable_ap_mode("picow_test", "password", CYW43_AUTH_WPA2_AES_PSK);

    // Configura IP e máscara de sub-rede
    ip4_addr_t ip, mask;
    IP4_ADDR(&ip, 192, 168, 4, 1);
    IP4_ADDR(&mask, 255, 255, 255, 0);

    // Inicializa servidor DHCP para fornecer IPs aos clientes
    dhcp_server_t dhcp_server;
    dhcp_server_init(&dhcp_server, &ip, &mask);

    // Inicializa servidor DNS
    dns_server_t dns_server;
    dns_server_init(&dns_server, &ip);

    // Inicializa servidor HTTP
    iniciar_http_server();

    // Exibe instruções no display e no console
    oled_mensagem("Acesse:", "192.168.4.1");
    printf("[INFO] Acesse: http://192.168.4.1\n");

    // Loop principal para manter o Access Point ativo
    while (true) {
        cyw43_arch_poll();
        sleep_ms(10);
    }
}
