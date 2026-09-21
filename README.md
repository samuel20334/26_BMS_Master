# 26 BMS Master

![MCU](https://img.shields.io/badge/MCU-STM32H563RITx-03234B?logo=stmicroelectronics&logoColor=white)
![RTOS](https://img.shields.io/badge/RTOS-FreeRTOS-green)
![C](https://img.shields.io/badge/C-Embedded-A8B9CC?logo=c&logoColor=white)
![GUI](https://img.shields.io/badge/GUI-PyQt6-41CD52?logo=qt&logoColor=white)

Master-node firmware for a segmented lithium-ion battery pack's **Battery Management System (BMS)**, running on an **STM32H563** under **FreeRTOS**. The master aggregates cell data from multiple `LTC6813` battery stack monitors, enforces safety limits, estimates state of charge, and reports pack status over dual CAN (FDCAN) buses — with a companion desktop GUI for live monitoring.

## Core responsibilities

- **Cell monitoring** — reads per-cell voltages and temperatures across all segments via `LTC6813` battery monitor ICs (`read_cell_voltages`, `read_cell_temps`)
- **Fault detection** — checks every cell against under/over-voltage and under/over-temperature thresholds and raises a fault bitmask (`check_uv_ov_fault`, `check_ut_ot_fault`) transmitted over CAN
- **Cell balancing** — passive balancing toward a target voltage (`balance_cells`)
- **State of charge estimation** — combines open-circuit-voltage lookup (`soc_ocv`) with coulomb counting (`soc_cc`) using an on-chip OCV curve
- **CAN telemetry (FDCAN1)** — broadcasts pack voltage, per-segment min/max voltage & temperature, and fault codes on dedicated message IDs, backed by a ring-buffer TX queue (`CAN_TX_Enqueue` / `CAN_TX_Process`) so transmission never blocks the control loop
- **Charger interface (FDCAN2)** — implements the CAN handshake to start/stop charging with an Elcon-protocol charger, gated on pack safety state
- **Onboard data logging** — writes timestamped, CRC-16-checked voltage/temperature records to an `M95P32` SPI EEPROM in a circular log, with recovery/replay on boot (`EEPROM_Write`, `EEPROM_FindStart`, `EEPROM_TransmitAll`)
- **Live monitoring GUI** — a PyQt6 desktop app (`gui.py`) that reads the master's UART stream and renders per-cell voltage/temperature tables and fault indicators in real time

## Hardware & tech stack

| Component | Role |
|---|---|
| STM32H563RITx | Master MCU (Cortex-M33) |
| FreeRTOS | Task scheduling |
| LTC6813-1 | Multi-cell battery stack monitor (daisy-chained via SPI) |
| M95P32 | SPI EEPROM for black-box data logging |
| Dual FDCAN | Segment/pack telemetry (FDCAN1) + charger comms (FDCAN2) |
| PyQt6 + PySerial | Desktop monitoring GUI (`gui.py`) |

## Repository structure

```
26_BMS_Master/
├── Core/
│   ├── Inc/               # bms_functions.h, FreeRTOS config, HAL config
│   └── Src/                # main.c, bms_functions.c, RTOS app entry
├── Drivers/
│   ├── LTC6813/            # battery monitor IC driver
│   ├── Elcon/               # charger CAN protocol driver
│   └── M95P32/              # EEPROM driver
├── Middlewares/Third_Party/FreeRTOS/
├── gui.py                  # PyQt6 UART monitoring GUI
└── 26-BMS_Master.ioc        # STM32CubeMX project configuration
```

## Building

This is an **STM32CubeIDE** project.

1. Import into STM32CubeIDE (`File → Import → Existing Projects into Workspace`)
2. Build with the default `Debug` configuration, or regenerate peripheral init code from `26-BMS_Master.ioc` via STM32CubeMX if pin/clock config changes
3. Flash to the target board over SWD

## Running the monitoring GUI

```bash
pip install pyqt6 pyserial
python gui.py
```

Connect to the board's UART port from the GUI to view live per-segment cell voltages, temperatures, and fault status.

## Suggested next step

The `Debug/`, `log/`, and STM32CubeIDE build artefacts are currently tracked in the repo. Adding a `.gitignore` for these (and the compiler crash logs) would keep the history focused on source changes — happy to generate one if useful.
