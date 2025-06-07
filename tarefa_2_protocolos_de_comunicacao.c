#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"
#include "dhcpserver.h"
#include "dnsserver.h"
#include "oled_display.h"
#include "http_server.h"

int main() {
    stdio_init_all();
    sleep_ms(3000);

    adc_init();
    adc_set_temp_sensor_enabled(true);
    oled_inicializar();

    oled_mensagem("Iniciando Wi-Fi...", "");

    cyw43_arch_init();
    cyw43_arch_enable_ap_mode("picow_test", "password", CYW43_AUTH_WPA2_AES_PSK);

    ip4_addr_t ip, mask;
    IP4_ADDR(&ip, 192, 168, 4, 1);
    IP4_ADDR(&mask, 255, 255, 255, 0);

    dhcp_server_t dhcp_server;
    dhcp_server_init(&dhcp_server, &ip, &mask);

    dns_server_t dns_server;
    dns_server_init(&dns_server, &ip);

    iniciar_http_server();

    oled_mensagem("Acesse:", "192.168.4.1");
    printf("[INFO] Acesse: http://192.168.4.1\n");

    while (true) {
        cyw43_arch_poll();
        sleep_ms(10);
    }
}
