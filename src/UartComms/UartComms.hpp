#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "pico/stdlib.h"
#include "hardware/uart.h"

class UartComms {
public:
    UartComms(
        uart_inst_t* uart,
        uint baud,
        uint txPin,
        uint rxPin,
        char messageEndMarker = '#'
    );

    bool init();
    void send(const std::string& message);
    bool receive(std::string& message);

private:
    static constexpr size_t UART_BUFFER_SIZE = 256;
    static constexpr size_t MESSAGE_BUFFER_SIZE = 256;

    uart_inst_t* uart_;
    uint baud_;
    uint txPin_;
    uint rxPin_;
    char messageEndMarker_;

    volatile uint8_t ringBuffer_[UART_BUFFER_SIZE]{};
    volatile uint16_t writeIndex_ = 0;
    volatile uint16_t readIndex_ = 0;

    char messageBuffer_[MESSAGE_BUFFER_SIZE]{};
    uint16_t messageIndex_ = 0;

    static UartComms* uart0Instance_;
    static UartComms* uart1Instance_;

    static void uart0IrqHandler();
    static void uart1IrqHandler();

    void onRxInterrupt();
    void receiveByte(uint8_t byte);
    bool readByte(uint8_t& byte);
};