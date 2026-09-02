#include "UartComms.hpp"

UartComms* UartComms::uart0Instance_ = nullptr;
UartComms* UartComms::uart1Instance_ = nullptr;

UartComms::UartComms(
    uart_inst_t* uart,
    uint baud,
    uint txPin,
    uint rxPin,
    char messageEndMarker
)
    : uart_(uart),
      baud_(baud),
      txPin_(txPin),
      rxPin_(rxPin),
      messageEndMarker_(messageEndMarker) {
}

bool UartComms::init() {
    uart_init(uart_, baud_);

    gpio_set_function(txPin_, GPIO_FUNC_UART);
    gpio_set_function(rxPin_, GPIO_FUNC_UART);

    if (uart_ == uart0) {
        uart0Instance_ = this;
        irq_set_exclusive_handler(UART0_IRQ, uart0IrqHandler);
        irq_set_enabled(UART0_IRQ, true);
    } else if (uart_ == uart1) {
        uart1Instance_ = this;
        irq_set_exclusive_handler(UART1_IRQ, uart1IrqHandler);
        irq_set_enabled(UART1_IRQ, true);
    } else {
        return false;
    }

    uart_set_irq_enables(uart_, true, false);

    return true;
}

void UartComms::send(const std::string& message) {
    uart_write_blocking(
        uart_,
        reinterpret_cast<const uint8_t*>(message.data()),
        message.size()
    );

    uart_putc_raw(uart_, messageEndMarker_);
}

bool UartComms::receive(std::string& message) {
    uint8_t byte;

    while (readByte(byte)) {
        if (byte == '\0') {
            continue;
        }

        if (byte == static_cast<uint8_t>(messageEndMarker_)) {
            if (messageIndex_ == 0) {
                continue;
            }

            message.assign(messageBuffer_, messageIndex_);
            messageIndex_ = 0;

            return true;
        }

        if (messageIndex_ < MESSAGE_BUFFER_SIZE - 1) {
            messageBuffer_[messageIndex_++] = static_cast<char>(byte);
        } else {
            messageIndex_ = 0;
        }
    }

    return false;
}

void UartComms::uart0IrqHandler() {
    if (uart0Instance_ != nullptr) {
        uart0Instance_->onRxInterrupt();
    }
}

void UartComms::uart1IrqHandler() {
    if (uart1Instance_ != nullptr) {
        uart1Instance_->onRxInterrupt();
    }
}

void UartComms::onRxInterrupt() {
    while (uart_is_readable(uart_)) {
        receiveByte(static_cast<uint8_t>(uart_getc(uart_)));
    }
}

void UartComms::receiveByte(uint8_t byte) {
    uint16_t nextIndex = (writeIndex_ + 1) % UART_BUFFER_SIZE;

    if (nextIndex == readIndex_) {
        // Ring buffer full. Drop the incoming byte.
        return;
    }

    ringBuffer_[writeIndex_] = byte;
    writeIndex_ = nextIndex;
}

bool UartComms::readByte(uint8_t& byte) {
    if (writeIndex_ == readIndex_) {
        return false;
    }

    byte = ringBuffer_[readIndex_];
    readIndex_ = (readIndex_ + 1) % UART_BUFFER_SIZE;

    return true;
}