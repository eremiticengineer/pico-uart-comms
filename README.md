# Pico UART Comms Test

This is a simple project to send data from one pico to another over UART. It's a handy way to send data to another pico that can broadcast the data over LoRa for example.

## Cloning the project

Clone the project with FreeRTOS submodules to get the pico functionality:

```
git clone --recurse-submodules https://github.com/eremiticengineer/pico-uart-comms
```

If you cloned without recursing submodules:

```
git submodule update --init --recursive
```

## Building for different chips

pico2:

```
./build_project

or

./build_project pico2
```

Seeed Xaio rp2040:

```
./build_project seeed_xiao_rp2040
```

## Wiring the pico UARTs

The UART configurations are in:

```
src/pico-uart-api-test-sender.cpp uart_config
src/pico-uart-api-test-sender.cpp uart_config
```

adjust to suit. Wire thus:

```
sender pico TX -> receiver pico RX
sender pico RX -> receiver pico TX
sender pico GND -> receiver pico GND
```

## Setting up the sender and receiver

First build both the sender and receiver:
```
./build_project
```

then hold in BOOTSEL on the sender pico, plug it in and flash it:

```
cp build/pico_uart_api_test_sender.uf2 /media/pi/RP2350
```

and unplug the sender pico.

Hold in BOOTSEL on the receiver pico, plug it in and flash it:

```
cp build/pico_uart_api_test_receiver.uf2 /media/pi/RP2350
```

and unplug the receiver pico.

## Monitoring the UART comms between the two picos

Plug in the receiver pico and monitor:

```
picocom /dev/ttyACM0 -b 115200
```

then plug in the sender pico and monitor:

```
picocom /dev/ttyACM1 -b 115200

wrote 'comms data from pico#', length=21
```

The receiver monitor will show:

```
received: comms data from pico
```

## FreeRTOS-Kernal setup for new projects

When creating a FreeRTOS project from scratch, clone the main branch into the project. The main branch at the moment has the necessary pico functionality:

```
git init
git submodule add https://github.com/FreeRTOS/FreeRTOS-Kernel.git lib/FreeRTOS-Kernel
git submodule update --init --recursive
git add .gitmodules lib/FreeRTOS-Kernel
```

## FreeRTOSConfig.h

This file customises FreeRTOS for your project. The file:

```
include/FreeRTOSConfig.h
```

is this one from the pico-examples:

```
pico-examples/freertos/FreeRTOSConfig_examples_common.h
```

## References

* [Task priorites](https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/01-Tasks-and-co-routines/03-Task-priorities)
* [uxTaskGetStackHighWaterMark](https://www.freertos.org/Documentation/02-Kernel/04-API-references/03-Task-utilities/04-uxTaskGetStackHighWaterMark)
