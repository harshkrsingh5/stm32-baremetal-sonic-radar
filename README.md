# stm32-baremetal-sonic-radar
Bare-metal C firmware for an ARM Cortex-M3 proximity radar using raw registers for UART and SPI
# Bare-Metal ARM Cortex-M3 Telemetry & Proximity System

A highly optimized, register-level embedded firmware application written from scratch in bare-metal C for the STM32 (ARM Cortex-M3) architecture. This system integrates multiple communication protocols (**UART** and **SPI**) alongside a time-of-flight ultrasonic sensor to establish a real-time proximity radar and visual bar graph alarm, operating entirely without Vendor HAL or third-party libraries.

## 🛠️ System Architecture & Hardware Topology

The firmware maps custom pointers directly to the microcontroller's memory-mapped registers to govern peripheral state engines across multiple internal clock gates:

* **Processor Core:** ARM Cortex-M3 (Operating @ 8 MHz)
* **Outbound Telemetry (UART1):** Configured via Alternate Function Push-Pull pins running at a 9600 baud timing grid (`USART1_BRR = 0x341`).
* **Visual Display Interface (SPI1):** Configured as an SPI Master running at an $f/256$ clock division mode to achieve absolute breadboard capacitance immunity, driving an external **74HC595 Shift Register Stage**.
* **Sensing Layer:** An **HC-SR04 Ultrasonic Sensor** interfaced via General Purpose Output Push-Pull (TRIG) and Floating Input (ECHO) tracks on Port A.

## 🚀 Key Engineering Highlights

### 1. Full-Duplex Synchronous Bus Synchronization
Unlike simple bit-banged approaches, the SPI driver handles data movement using strict hardware flag tracking. Outbound streams block until the internal `TXE` buffer flag clears, while incoming bytes are caught via `RXNE` spinlocks, ensuring 100% full-duplex transactional integrity.

### 2. Empirical Software Loop Calibration
To compute distances accurately without a hardware input-capture timer initialized, a software delay loop was explicitly calibrated to the underlying CPU instruction clock cycles. By injecting raw `NOP` (No Operation) instructions to match loop management overhead, the function achieves microsecond-precision timing windows ($\approx 8\text{ clock cycles} = 1.0\,\mu\text{s}$).

### 3. Footprint Optimization & Memory Hygiene
Standard formatting functions like `sprintf` introduce significant code bloat inside tiny flash structures. This firmware implements a highly lightweight, custom integer-to-ASCII conversion sub-routine leveraging recursive modulo and division logic, minimizing memory footprint and maintaining zero compiler alerts (`-Wunused`).

## 📁 Source Code Structure

* `main.c`: Contains the unified embedded lifecycle architecture, hardware clock gating configurations, protocol initializations, and the core tracking loops.

## ⚙️ How To Run & Wire
1. Route STM32 pins `PA4`, `PA5`, and `PA7` directly to the `RCLK`, `SRCLK`, and `SER` tracks of the 74HC595 Shift Register.
2. Link the HC-SR04 `TRIG` and `ECHO` lines to `PA1` and `PA2`.
3. Bridge STM32 `PA9` (TX) over to an external USB-to-TTL pass-through converter (e.g., an MCU board held in hard reset) to intercept the scrolling serial stream on your computer console at 9600 baud.
