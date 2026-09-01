#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

#include <string>
#include <cstring>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "UartComms.hpp"

#define UART_SEND_TASK_PRIORITY (tskIDLE_PRIORITY + 2UL)

SemaphoreHandle_t uart_mutex;

#ifdef DEVICE_seeed_xiao_rp2040
// Seeed Xaio rp2040
namespace uart_config {
    inline uart_inst_t* const UART_NUM = uart0;
    inline constexpr uint BAUD = 115200;
    inline constexpr uint TX = 0;
    inline constexpr uint RX = 1;
}
#elif defined(DEVICE_pico2)
namespace uart_config {
    inline uart_inst_t* const UART_NUM = uart0;
    inline constexpr uint BAUD = 115200;
    inline constexpr uint TX = 0;
    inline constexpr uint RX = 1;
}
#else
#error "No supported DEVICE defined"
#endif

void uart_send_task(void* params) {
    UartComms *pUartComms = static_cast<UartComms *>(params);

    std::string message = "comms data from pico";

    while (true) {
        if (xSemaphoreTake(uart_mutex, pdMS_TO_TICKS(100))) {
            pUartComms->send(message);

            printf("wrote '%s', length=%zu\n", message.c_str(), message.size());

            xSemaphoreGive(uart_mutex);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int main( void )
{
    stdio_init_all();

    sleep_ms(2000);

    UartComms uartComms(
        uart_config::UART_NUM,
        uart_config::BAUD,
        uart_config::TX,
        uart_config::RX
    );
    uartComms.init();

    uart_mutex = xSemaphoreCreateMutex();

    xTaskCreate(uart_send_task, "UartSendTask", 512, (void*)&uartComms, UART_SEND_TASK_PRIORITY, nullptr);

    vTaskStartScheduler();

    return 0;
}
