# SmartBag – RFID-Based Smart Bag

## Project Overview

SmartBag is an embedded systems project developed using the STM32F103C8T6 microcontroller. It uses RFID technology to identify and track essential items inside a school bag.

The system helps users check whether all required books or items have been packed before leaving.

## Hardware Components

- STM32F103C8T6 (Blue Pill)
- MFRC522 RFID Reader
- RFID Tags
- SSD1306 OLED Display
- Buzzer
- Connecting wires

## Features

- RFID-based item identification
- Tracks scanned and missing items
- Displays information on an OLED screen
- Buzzer indication for missing items

## Working Principle

1. Each book or item is assigned an RFID tag.
2. The RFID reader scans the tags.
3. The STM32 microcontroller identifies the scanned items.
4. The OLED display shows the scanning status.
5. The system indicates missing items using a buzzer.

## Software and Tools

- Embedded C
- STM32CubeIDE
- STM32CubeMX

## Microcontroller

STM32F103C8T6 – Blue Pill

