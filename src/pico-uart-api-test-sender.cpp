#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

#include <string>
#include <cstring>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define UART_SEND_TASK_PRIORITY (tskIDLE_PRIORITY + 2UL)

SemaphoreHandle_t uart_mutex;

namespace uart_config {
    inline uart_inst_t* const UART_NUM = uart1;
    inline constexpr uint BAUD = 115200;
    inline constexpr uint TX = 4;
    inline constexpr uint RX = 5;
}

void uart_send_task(__unused void *params) {
    std::string commsString = "comms data from pico#";
    while (true) {
        if (xSemaphoreTake(uart_mutex, pdMS_TO_TICKS(100))) {
            uart_write_blocking(
                uart_config::UART_NUM,
                reinterpret_cast<const uint8_t *>(commsString.data()),
                commsString.size()
            );
            xSemaphoreGive(uart_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int main( void )
{
    stdio_init_all();

    uart_init(uart_config::UART_NUM, uart_config::BAUD);
    gpio_set_function(uart_config::TX, GPIO_FUNC_UART);
    gpio_set_function(uart_config::RX, GPIO_FUNC_UART);

    uart_mutex = xSemaphoreCreateMutex();

    xTaskCreate(uart_send_task, "UartSendTask", 512, nullptr, UART_SEND_TASK_PRIORITY, nullptr);

    vTaskStartScheduler();

    return 0;
}
