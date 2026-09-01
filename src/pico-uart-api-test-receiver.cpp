#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "UartComms.hpp"

#define UART_RECEIVE_TASK_PRIORITY (tskIDLE_PRIORITY + 2UL)

namespace uart_config {
    inline uart_inst_t* const UART_NUM = uart0;
    inline constexpr uint BAUD = 115200;
    inline constexpr uint TX = 0;
    inline constexpr uint RX = 1;
}

void uart_receive_task(void* params) {
    UartComms* pUartComms = static_cast<UartComms*>(params);

    while (true) {
        std::string message;

        while (pUartComms->receive(message)) {
            printf("received: '%s'\n", message.c_str());
        }

        vTaskDelay(pdMS_TO_TICKS(25));
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

    xTaskCreate(uart_receive_task, "UartReceiveTask", 512, (void*)&uartComms, UART_RECEIVE_TASK_PRIORITY, nullptr);

    vTaskStartScheduler();

    return 0;
}
