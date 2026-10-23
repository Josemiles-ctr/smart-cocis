# CoCIS Smart Control and Monitoring System

A distributed access control, occupancy monitoring and lighting automation system for the College of Computing and Information Sciences (CoCIS), Makerere University. The system is built on ESP32 microcontrollers using ESP-IDF and is simulated on Wokwi.

## Table of Contents

- [Description](#description)
- [Implementation Status](#implementation-status)
- [Features](#features)
- [System Architecture](#system-architecture)
- [Hardware Components](#hardware-components)
- [RFID Simulation](#rfid-simulation)
- [Software Breakdown](#software-breakdown)
- [Access Decision Logic](#access-decision-logic)
- [Serial Interface](#serial-interface)
- [Data Storage](#data-storage)
- [Synchronization](#synchronization)
- [Project Structure](#project-structure)
- [Build and Run](#build-and-run)
- [Testing](#testing)
- [Limitations](#limitations)

## Description

The system secures the main entrance of the building, tracks who is inside at any time, and switches lights off in unoccupied lecture rooms to reduce electricity use. It is designed as a pilot covering the main entrance and two lecture rooms, **LLT_1A** and **LLT_1B**.

The main entrance has two sections, one for entry and one for exit. Each person is identified by an RFID tag that must be scanned at both sections. Each lecture room has its own terminal that detects occupancy and controls the lights.

## Implementation Status

The repository scaffolding is already in place: both ESP-IDF projects, their component layout, `sdkconfig`, Wokwi configuration, SD-card data files and the local VS Code IntelliSense setup. The firmware currently runs the starter button/LED example on both nodes (button on GPIO 14, LED on GPIO 18).

The modules, headers and behaviour described below are the target design and are still being implemented. In particular, the two-room build configuration (`COCIS_ROOM_ID`) and the planned `include/room_config.h` are documented here but are not implemented yet.

- Implemented: `main.c`, `sample_led.c`, `include/sample_led.h` and the starter Wokwi circuit in each node.
- Room simulation scaffolding: `room_node/LLT_1A/` and `room_node/LLT_1B/`, each with its own `diagram.json` and `wokwi.toml`.
- Scaffolded but not yet wired into the build: the remaining module sources and their headers.
- See [docs/GETTING_STARTED.md](docs/GETTING_STARTED.md) for the build, simulation and development workflow.

## Features

The capabilities below describe the target system.

- RFID based identification at separate entry and exit sections
- Door control for valid scans
- Record of entry and exit events, and a live list of people inside the building
- Anti-passback: no repeated entry without an exit, and no exit without a prior entry
- Configurable building access period, enforced for both entry and exit
- Authenticated administration: only logged in administrators can change settings
- Automatic lighting control based on room occupancy
- Three cooperating ESP32 terminals
- Persistent storage of logs on an SD card
- Serial command interface for monitoring and configuration
- Synchronization of shared resources between tasks and between terminals

## System Architecture

The system has three terminals.

| Terminal      | Responsibility                                                                                                           |
| ------------- | ------------------------------------------------------------------------------------------------------------------------ |
| Entrance Node | RFID scans (entry and exit), door control, access schedule, user registry, access log, occupancy count, serial interface |
| LLT_1A Node   | Occupancy detection and lighting control for lecture room LLT_1A                                                         |
| LLT_1B Node   | Occupancy detection and lighting control for lecture room LLT_1B                                                         |

```
                 +----------------------------+
                 |        Entrance Node       |
                 |  Entry reader / Exit reader|
                 |  Door control, SD card     |
                 |  Serial interface          |
                 +-------------+--------------+
                               |
                      Inter-node messaging
                               |
              +----------------+----------------+
              |                                 |
   +----------+-----------+         +-----------+----------+
   |     LLT_1A Node      |         |     LLT_1B Node      |
   |  Occupancy sensor    |         |  Occupancy sensor    |
   |  Light control       |         |  Light control       |
   +----------------------+         +----------------------+
```

The Entrance Node holds the authoritative copy of shared state (registry, schedule, access log, occupancy). Room nodes report their status to it and receive configuration from it. Room nodes control their own lights locally so lighting continues if communication is lost.

### Inter-node Communication

Nodes exchange messages over MQTT on the Wokwi `Wokwi-GUEST` network. Topics are namespaced as follows.

```
cocis/entrance/schedule     Schedule updates from the Entrance Node
cocis/system/time           Time reference from the Entrance Node
cocis/LLT_1A/status         Occupancy and light state of LLT_1A
cocis/LLT_1B/status         Occupancy and light state of LLT_1B
```

The communication layer is isolated behind its own module interface so the transport can be changed without affecting the rest of the firmware.

## Hardware Components

Pin numbers below are the target assignment for the full system. Update `diagram.json` to match as each module is implemented; the current simulation only contains the starter button and LED.

### Entrance Node

| Component                 | Purpose       | GPIO                           |
| ------------------------- | ------------- | ------------------------------ |
| ESP32 DevKit              | Controller    | n/a                            |
| microSD card module (SPI) | Log storage   | SCK 18, MISO 19, MOSI 23, CS 5 |
| Servo or relay            | Entry door    | 25                             |
| Servo or relay            | Exit door     | 26                             |
| Green LED                 | Entry granted | 2                              |
| Blue LED                  | Exit granted  | 4                              |
| Red LED                   | Access denied | 15                             |

### Room Nodes (LLT_1A and LLT_1B)

| Component           | Purpose             | GPIO |
| ------------------- | ------------------- | ---- |
| ESP32 DevKit        | Controller          | n/a  |
| PIR motion sensor   | Occupancy detection | 27   |
| Relay module or LED | Room light          | 26   |

## RFID Simulation

Wokwi has no RFID reader part, so the reader is simulated through the serial interface. Two logical readers are represented by a prefix.

```
IN:<UID>     Tag scanned at the entry section
OUT:<UID>    Tag scanned at the exit section
```

Example:

```
IN:A1B2C3D4
OUT:A1B2C3D4
```

The scan source is decoupled from the access logic by a queue of scan events.

```c
typedef struct {
    bool     is_entry;
    char     uid[16];
    uint32_t timestamp;
} scan_event_t;
```

The serial parser produces scan events and the access control task consumes them. A hardware reader driver, such as an MFRC522 over SPI, could replace the parser without changes to the access logic.

## Software Breakdown

### Entrance Node

| Task                    | Responsibility                                                                            |
| ----------------------- | ----------------------------------------------------------------------------------------- |
| `rfid_input_task`     | Parses `IN:` and `OUT:` input and queues scan events                                  |
| `access_control_task` | Validates tags, applies schedule and anti-passback rules, drives doors, updates occupancy |
| `logger_task`         | Writes access and system events to the SD card                                            |
| `cli_task`            | Serial menu, login and command handling                                                   |
| `comms_task`          | Sends and receives inter-node messages                                                    |

### Room Nodes (LLT_1A and LLT_1B)

| Task               | Responsibility                                       |
| ------------------ | ---------------------------------------------------- |
| `occupancy_task` | Reads the PIR sensor and applies a hold-off timer    |
| `light_task`     | Switches the light according to occupancy            |
| `comms_task`     | Publishes status and receives time and configuration |

### Lighting Logic

- A room is occupied when motion has been detected within the hold-off period.
- The light turns on when the room becomes occupied.
- The light turns off when the hold-off period passes with no motion.
- The hold-off period is configurable.

## Access Decision Logic

Each scan is evaluated in this order:

1. The tag is looked up in the registry. Unknown or disabled tags are denied.
2. The current time is checked against the configured access period. Scans outside the period are denied at both sections.
3. Anti-passback is checked. Entry is denied if the user is already inside. Exit is denied if the user is not inside.
4. If all checks pass, the matching door is activated, the user state and occupancy count are updated, and the event is logged.

Denied scans are logged with the reason.

### Access Schedule

The access period is defined by a start time and an end time and stored persistently. It applies to both entry and exit. Only an authenticated administrator can change it, and the change takes effect on the next scan.

### Authentication

- Administrator credentials are stored as salted hashes.
- Restricted commands require an active login session.
- Repeated failed logins cause a temporary lockout.
- Login attempts and configuration changes are logged.

## Serial Interface

### General Commands

| Command                     | Description                               |
| --------------------------- | ----------------------------------------- |
| `help`                    | List commands                             |
| `status`                  | Time, schedule, occupancy and node status |
| `inside`                  | Users currently inside the building       |
| `count`                   | Number of people inside                   |
| `log [n]`                 | Last n access records                     |
| `rooms`                   | Occupancy and light state of both rooms   |
| `schedule`                | Show the access period                    |
| `nodes`                   | Connection state of room nodes            |
| `login <user> <password>` | Authenticate as administrator             |
| `logout`                  | End the session                           |

### Administrator Commands

| Command                          | Description                       |
| -------------------------------- | --------------------------------- |
| `schedule set <HH:MM> <HH:MM>` | Set access start and end time     |
| `user add <UID> <name>`        | Register a tag                    |
| `user remove <UID>`            | Remove a tag                      |
| `user disable <UID>`           | Disable a tag                     |
| `user list`                    | List registered users             |
| `holdoff <seconds>`            | Set the light hold-off time       |
| `passwd`                       | Change the administrator password |
| `export`                       | Print stored logs over serial     |

### Example Output Format

```
> login admin ********
Login successful.
> schedule set 07:00 20:00
Access period updated: 07:00 to 20:00
> IN:A1B2C3D4
[2026-10-26 08:12:03] ENTRY granted: Alice (A1B2C3D4) | inside=1
> OUT:A1B2C3D4
[2026-10-26 20:05:41] EXIT denied: outside access period
```

This output shows the intended format and is not captured from a running build.

## Data Storage

Large and frequently growing data is stored on the SD card.

| File                   | Content                                                                  |
| ---------------------- | ------------------------------------------------------------------------ |
| `/sdcard/access.csv` | Timestamp, UID, name, direction, result, reason                          |
| `/sdcard/system.log` | Boot, login attempts, configuration changes, node connect and disconnect |
| `/sdcard/room.csv`   | Room occupancy and light state changes                                   |

Small configuration data (schedule, hold-off time, administrator credential hash) is kept in NVS flash so the system can operate if the SD card is absent.

Access record format:

```
timestamp,uid,name,direction,result,reason
2026-10-26 08:12:03,A1B2C3D4,Alice,IN,GRANTED,
2026-10-26 20:05:41,A1B2C3D4,Alice,OUT,DENIED,outside_access_period
```

## Synchronization

| Shared resource                   | Risk                                   | Mechanism                                                                         |
| --------------------------------- | -------------------------------------- | --------------------------------------------------------------------------------- |
| SD card and log files             | Concurrent writes corrupting records   | Single `logger_task` fed by a queue, with a mutex around file operations        |
| User registry and occupancy count | Concurrent scan and CLI updates        | Mutex around registry and counter                                                 |
| Access schedule                   | Read during a scan while being changed | Mutex, with the schedule copied under lock before evaluation                      |
| Cross-node state                  | Duplicate or out of order messages     | Entrance Node as single authority, messages carry a sequence number and timestamp |
| Scan events                       | Lost or reordered scans                | FreeRTOS queue between input and access control                                   |

Reliability measures:

- Room nodes keep controlling lights locally if the Entrance Node is unreachable.
- Each room node sends a periodic heartbeat so the Entrance Node can detect a missing node.
- Storage errors are reported on the serial interface and logged.
- The task watchdog is enabled on all nodes.

## Project Structure

```
smart-cocis/
|-- Readme.md
|-- docs/
|   `-- GETTING_STARTED.md
|-- entrance_node/
|   |-- CMakeLists.txt
|   |-- include/
|   |   |-- sample_led.h
|   |   |-- rfid_sim.h
|   |   |-- access_control.h
|   |   |-- schedule.h
|   |   |-- registry.h
|   |   |-- logger.h
|   |   |-- cli.h
|   |   |-- auth.h
|   |   `-- comms.h
|   |-- main/
|   |   |-- CMakeLists.txt
|   |   |-- main.c
|   |   |-- sample_led.c
|   |   |-- rfid_sim.c
|   |   |-- access_control.c
|   |   |-- schedule.c
|   |   |-- registry.c
|   |   |-- logger.c
|   |   |-- cli.c
|   |   |-- auth.c
|   |   `-- comms.c
|   |-- simulations/
|   |   `-- sample_led.json
|   |-- diagram.json
|   |-- sdkconfig
|   `-- wokwi.toml
|-- room_node/
|   |-- CMakeLists.txt
|   |-- Readme.md
|   |-- GETTING_STARTED.md
|   |-- LLT_1A/
|   |   |-- diagram.json
|   |   `-- wokwi.toml
|   |-- LLT_1B/
|   |   |-- diagram.json
|   |   `-- wokwi.toml
|   |-- include/
|   |   |-- sample_led.h
|   |   |-- room_config.h        # planned room identity/configuration header
|   |   |-- occupancy.h
|   |   |-- lighting.h
|   |   `-- comms.h
|   |-- main/
|   |   |-- CMakeLists.txt
|   |   |-- main.c
|   |   |-- sample_led.c
|   |   |-- occupancy.c
|   |   |-- lighting.c
|   |   `-- comms.c
|   |-- simulations/
|   |   `-- sample_led.json
|   |-- sdkconfig
|   `-- .gitignore
|-- sdcard/
|   |-- access.csv
|   |-- room.csv
|   `-- system.log
`-- .vscode/                  # local IntelliSense helpers (gitignored)
```

`entrance_node` and `room_node` are independent ESP-IDF projects with their own `build/` and `sdkconfig` directories. The room firmware is built twice: `room_node/build/LLT_1A/` and `room_node/build/LLT_1B/`.

Only `main.c` and `sample_led.c` are currently registered in `main/CMakeLists.txt`; add the other module sources there as they are implemented. See [docs/GETTING_STARTED.md](docs/GETTING_STARTED.md) for details.

### Room Builds and Includes

`LLT_1A` and `LLT_1B` use the same `room_node` source files. The room is selected at build time with `COCIS_ROOM_ID`:

| Room    | ID | Build directory          |
| ------- | -- | ------------------------ |
| `LLT_1A` | 1  | `room_node/build/LLT_1A` |
| `LLT_1B` | 2  | `room_node/build/LLT_1B` |

The planned shared configuration header is `room_node/include/room_config.h`. It is not implemented yet. When added, it will map `COCIS_ROOM_ID` to the room slug (`LLT_1A` / `LLT_1B`), MQTT client ID and topics such as `cocis/LLT_1A/status`.

Planned include relationships:

| Source file       | Includes                                                    | Responsibility                                      |
| ----------------- | ----------------------------------------------------------- | --------------------------------------------------- |
| `main.c`          | `room_config.h`, `occupancy.h`, `lighting.h`, `comms.h`     | Starts the room tasks and passes the room identity  |
| `occupancy.c`     | `occupancy.h`, `room_config.h`                              | PIR input, hold-off timer and occupancy state       |
| `lighting.c`      | `lighting.h`, `room_config.h`                               | Local light control                                 |
| `comms.c`         | `comms.h`, `room_config.h`                                  | MQTT status/config/time messaging                   |
| `sample_led.c`    | `sample_led.h` and ESP-IDF headers                          | Current starter button/LED example                  |

`room_config.h` does not include any project `.c` file. Each module includes its own header, and source files are compiled separately by CMake and linked together. Do not `#include` one `.c` file from another.

## Build and Run

### Prerequisites

- ESP-IDF installed and exported in the shell
- A Wokwi account, or the Wokwi extension for VS Code

### Build

The ESP32 target is already recorded in each project's `sdkconfig` (`CONFIG_IDF_TARGET="esp32"`), so `set-target` is only needed on a fresh checkout or when switching chips.

Build the entrance node:

```bash
cd entrance_node
idf.py build
```

The room firmware is shared, but it is built once per room. Each room uses its own build directory and passes its room ID to CMake:

```bash
cd room_node
idf.py -B build/LLT_1A -DCOCIS_ROOM_ID=1 build
idf.py -B build/LLT_1B -DCOCIS_ROOM_ID=2 build
```

| Room     | `COCIS_ROOM_ID` | Build output                  |
| -------- | --------------- | ----------------------------- |
| `LLT_1A` | `1`             | `build/LLT_1A/`               |
| `LLT_1B` | `2`             | `build/LLT_1B/`               |

The `COCIS_ROOM_ID` CMake option is part of the planned two-room build and is not implemented yet. Until it is, the existing `idf.py build` command still produces the starter firmware.

Each room build produces `embedded-course-work.bin` and `embedded-course-work.elf` inside its own build directory. See [docs/GETTING_STARTED.md](docs/GETTING_STARTED.md) for flashing, cleaning and development details.

### Wokwi Configuration

Wokwi requires `wokwi.toml` and `diagram.json` in the same folder. The entrance node keeps its Wokwi files at the project root:

```text
entrance_node/
|-- diagram.json
`-- wokwi.toml
```

The room simulations are separate. Each room has its own diagram and config:

| Room     | Wokwi config                   | Firmware path                                      | ELF path                                              |
| -------- | ------------------------------ | -------------------------------------------------- | ----------------------------------------------------- |
| `LLT_1A` | `room_node/LLT_1A/wokwi.toml` | `../build/LLT_1A/flasher_args.json`                | `../build/LLT_1A/embedded-course-work.elf`            |
| `LLT_1B` | `room_node/LLT_1B/wokwi.toml` | `../build/LLT_1B/flasher_args.json`                | `../build/LLT_1B/embedded-course-work.elf`            |

The paths in a room's `wokwi.toml` are relative to that file, not to the repository root. Each room config must point at its own build directory so Wokwi loads the firmware with the correct `COCIS_ROOM_ID`. ESP-IDF's `flasher_args.json` is preferred over the application `.bin` because it also loads the bootloader and partition table.

Because the room config files are in subfolders, open the repository root or `room_node` in VS Code and use **Wokwi: Select Config File** (`F1`) to choose `LLT_1A` or `LLT_1B` before starting the simulator.

If `project(...)` is renamed in a node's root `CMakeLists.txt`, update the matching `wokwi.toml` paths as well.

### Run

Build the node or room first, open the repository root or the relevant project folder in VS Code, select the Wokwi config, then start the simulator:

- Entrance node: `entrance_node/` and its root `wokwi.toml`.
- `LLT_1A`: `room_node/LLT_1A/wokwi.toml`.
- `LLT_1B`: `room_node/LLT_1B/wokwi.toml`.

The current starter firmware reads the button on GPIO 14 and drives the LED on GPIO 18. The RFID commands and room modules described above are part of the target design and are not implemented yet.

## Testing

| Scenario                                              | Expected result                                         |
| ----------------------------------------------------- | ------------------------------------------------------- |
| `IN:` with a registered tag within the access period  | Entry granted, occupancy incremented, event logged      |
| `IN:` with an unregistered tag                        | Denied and logged                                       |
| `IN:` twice without `OUT:`                            | Second scan denied                                      |
| `OUT:` for a user not inside                          | Denied                                                  |
| `IN:` outside the access period                       | Denied                                                  |
| `OUT:` outside the access period                      | Denied                                                  |
| Schedule change without login                         | Rejected                                                |
| Schedule change after login                           | Applied on the next scan                                |
| Repeated wrong password                               | Lockout and log entry                                   |
| Motion in a room, then none past the hold-off         | Light turns on, then off                                |
| `LLT_1A` or `LLT_1B` disconnected                     | Reported by the Entrance Node, local lighting continues |
| CLI command during a scan                             | No corrupted record or inconsistent count               |

## Limitations

- RFID reading is simulated through the serial interface.
- The current firmware is still the starter button/LED application.
- Time source (SNTP over simulated WiFi or an RTC part) is not implemented yet.
- Inter-node communication depends on what Wokwi supports for multiple ESP32 boards.
- Simulation timing does not match real-world performance exactly.
