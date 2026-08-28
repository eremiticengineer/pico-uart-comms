#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define UART_RECEIVE_TASK_PRIORITY (tskIDLE_PRIORITY + 2UL)

#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define UART_BUFFER_SIZE 256
#define MESSAGE_BUFFER_SIZE 256

SemaphoreHandle_t uart_mutex;

volatile uint8_t ring_buffer[UART_BUFFER_SIZE];
unsigned char uart_buffer[UART_BUFFER_SIZE];
volatile uint16_t write_index = 0;
volatile uint16_t read_index = 0;
uint16_t message_index = 0;
uint8_t message_buffer[MESSAGE_BUFFER_SIZE];

// Call this from UART RX interrupt or polling when a byte arrives
void uart_receive_byte(uint8_t byte) {
  ring_buffer[write_index] = byte;
  write_index = (write_index + 1) % UART_BUFFER_SIZE;
}

void on_uart_rx() {
  while (uart_is_readable(UART_ID)) {
      char c = uart_getc(UART_ID);
      uart_receive_byte(c);
  }
}

// Check if there is data available
uint16_t uart_available() {
  if (write_index >= read_index)
    return write_index - read_index;
  else
    return UART_BUFFER_SIZE - (read_index - write_index);
}

// Read one byte from buffer if available
int uart_read_byte(uint8_t* byte) {
    if (uart_available() == 0) return 0; // no data

    *byte = ring_buffer[read_index];
    read_index = (read_index + 1) % UART_BUFFER_SIZE;
    return 1; // success
}

void init_uart() {
  uart_init(UART_ID, BAUD_RATE);
  gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

  // Enable UART RX interrupt
  uart_set_irq_enables(UART_ID, true, false);  // RX only
  irq_set_exclusive_handler(UART0_IRQ, on_uart_rx);
  irq_set_enabled(UART0_IRQ, true);
}

void process_uart() {
  uint8_t byte;
  while (uart_read_byte(&byte)) {
    printf("received: %s\n", message_buffer);
    // printf("String length: %d\n", message_index);
    // minicom doesn't send \n 0x0A, it send \r 0x0D
    //if (byte == '\n' || byte == '\r') {
    if (byte == '!') {
      message_buffer[message_index] = '\0';
      message_index = 0;
      printf("received: %s\n", uart_buffer);
    }
    else {
      if (message_index < MESSAGE_BUFFER_SIZE - 1) {
          message_buffer[message_index++] = byte;
      }
      else {
          // Buffer overflow - reset message
          message_index = 0;
      }
    }
  }
}

void uart_receive_task(__unused void *params) {
    while (true) {
        process_uart();
        vTaskDelay(pdMS_TO_TICKS(25)); // 25ms
    }
}

int main( void )
{
    stdio_init_all();

    init_uart();

    uart_mutex = xSemaphoreCreateMutex();

    xTaskCreate(uart_receive_task, "UartReceiveTask", 512, nullptr, UART_RECEIVE_TASK_PRIORITY, nullptr);

    vTaskStartScheduler();

    return 0;
}
