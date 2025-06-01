cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configurações da extensão VSCode Pico
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Importa o SDK Pico
include(pico_sdk_import.cmake)

# Nome do projeto
project(tarefa_2_protocolos_de_comunicacao C CXX ASM)

# Inicializa o SDK do Pico
pico_sdk_init()

# === Adiciona os arquivos fonte ===
add_executable(tarefa_2_protocolos_de_comunicacao
    tarefa_2_protocolos_de_comunicacao.c
    dhcpserver/dhcpserver.c
    dnsserver/dnsserver.c
    inc/ssd1306_i2c.c
)

# === Inclui os diretórios de cabeçalhos ===
target_include_directories(tarefa_2_protocolos_de_comunicacao PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/dhcpserver
    ${CMAKE_CURRENT_LIST_DIR}/dnsserver
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

# === Habilita saída USB CDC ===
pico_enable_stdio_usb(tarefa_2_protocolos_de_comunicacao 1)
pico_enable_stdio_uart(tarefa_2_protocolos_de_comunicacao 0)

# === Vincula as bibliotecas necessárias ===
target_link_libraries(tarefa_2_protocolos_de_comunicacao
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    hardware_i2c
    hardware_adc
)

# === Geração dos arquivos UF2, BIN, etc ===
pico_add_extra_outputs(tarefa_2_protocolos_de_comunicacao)
