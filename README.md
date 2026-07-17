# LLD Executor (Universal Frequency Sensor)

![MCU](https://img.shields.io/badge/MCU-STM32F103C8T6-03234B)
![RTOS](https://img.shields.io/badge/RTOS-FreeRTOS%20CMSIS--V2-0B6E4F)
![CAN](https://img.shields.io/badge/CAN-1%20Mbit%2Fs-2E6FBB)
![Detector](https://img.shields.io/badge/Detector-TLC555CDR-8B0000)

Firmware for the **LLD (Liquid Level Detection) Executor** — a universal
dual-channel frequency sensor based on STM32F103 and TLC555CDR. Measures
frequency shift between two oscillator channels (needle and reference) to detect
liquid presence, with application-agnostic CAN interface.

## Current Status

| Area | Status |
| --- | --- |
| STM32CubeIDE project | Generated, CubeMX baseline configured |
| TIM3 dual input capture (CH1+CH2) | Configured (Prescaler=0, 64 MHz, ICFilter=4) |
| CAN transport | Configured (1 Mbit/s), service layer active |
| FreeRTOS task baseline | 4 tasks: CAN handler, Dispatcher, LLD Controller, Watchdog |
| App-layer integration | Complete (Stage 3-4, build passes clean) |
| LLD core driver | TIM3 capture + differential filter + state machine |
| Watchdog supervisor | IWDG + heartbeat (CAN/Dispatcher/LLD) |

## Hardware Baseline

Target MCU:

- `STM32F103C8T6`
- `SYSCLK = 64 MHz` (HSI + PLL)
- FreeRTOS CMSIS-V2
- bxCAN at `1 Mbit/s`

Detection hardware:

- `TLC555CDR` — dual-channel astable oscillator
- Channel 1 (needle): `PA6 (TIM3_CH1)` — input capture direct
- Channel 2 (reference): `PA7 (TIM3_CH2)` — input capture direct
- Frequency shift between channels indicates liquid presence

## Firmware Architecture

Main application layers:

```text
Core/
  STM32Cube HAL startup, interrupts, generated RTOS object creation

App/inc, App/src
  common executor config, queues, flash config, CAN protocol helpers,
  lld_timer_driver, lld_filter, lld_controller

App/src/tasks
  task_can_handler      bxCAN RX/TX transport owner
  task_dispatcher       service/domain routing and ACK/NACK policy
  task_lld_controller   LLD domain task, measurement lifecycle
  task_watchdog         heartbeat supervisor + IWDG refresh
```

RTOS objects:

- tasks: `task_can_handle`, `task_dispatcher`, `task_lld_contro`, `task_watchdog`
- queues: `can_rx_queue`, `can_tx_queue`, `dispatcher_queue`, `lld_queue`

Data flow:

```text
TLC555 CH1 (PA6) -> TIM3 capture IRQ -> period buffer -> task_lld_controller
TLC555 CH2 (PA7) -> TIM3 capture IRQ -> period buffer -> task_lld_controller
                                                             |
                                                             v
                                                      adaptive filter
                                                             |
                                                             v
                                                      CAN_SendDataFragmented
                                                             |
                                                             v
                                                      can_tx_queue
                                                             |
                                                             v
                                                      task_can_handler -> CAN bus
```

## CAN Executor Contract

Executor CAN profile:

- 29-bit Extended ID only
- strict `DLC = 8`
- Conductor address: `0x10`
- LLD default NodeID: `0x61`
- LLD device type: `0x06`

Service commands implemented:

| Command | Code | Status |
| --- | --- | --- |
| `GET_DEVICE_INFO` | `0xF001` | pending |
| `REBOOT` | `0xF002` | pending |
| `FLASH_COMMIT` | `0xF003` | pending |
| `GET_UID` | `0xF004` | pending |
| `SET_NODE_ID` | `0xF005` | pending |
| `FACTORY_RESET` | `0xF006` | pending |
| `GET_STATUS` | `0xF007` | pending |

LLD domain commands:

| Command | Code | Current behavior |
| --- | --- | --- |
| `LLD_ARM` | `0xE701` | pending |
| `LLD_GET_STATUS` | `0xE702` | pending |
| `LLD_SET_THRESHOLD` | `0xE703` | pending |

Normal response pattern:

```text
COMMAND -> ACK -> DATA... -> DONE
COMMAND -> ACK -> NACK
```

## Build

The project is generated for STM32CubeIDE 1.19.x.

From STM32CubeIDE:

1. Import the project.
2. Select the Debug configuration.
3. Build `STM32F103_lld`.

Headless local build:

```bash
make -C Debug all -j4
```

## Repository Layout

```text
App/
  inc/                 application headers
  src/                 application modules
  src/tasks/           FreeRTOS task entry implementations

Core/
  STM32Cube generated startup, main, interrupts

Drivers/
  STM32 HAL and CMSIS

Middlewares/
  FreeRTOS

readme/
  local workspace documentation pointers
```

## Documentation

This firmware repository contains the publishable firmware project. During local
development, detailed project documentation is maintained in the workspace-level
`DDS-240_readme` folder (DDS-240 ecosystem workspace). The local pointer file is:

```text
readme/README.md
```
