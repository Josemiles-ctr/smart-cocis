* [ ] 

# Room Node

The Room Node is one of the two ESP-IDF applications in the **CoCIS Smart Control and Monitoring System** repository. It detects occupancy and controls the light in a lecture room.

Two rooms are in scope: **`LLT_1A`** and **`LLT_1B`**. Both run the same `room_node` firmware. In the target design they are distinguished by a build-time room ID, so each room gets its own build output, Wokwi config and MQTT identity.

The canonical system design is in the [root README](../Readme.md), and the build/development guide is [docs/GETTING_STARTED.md](../docs/GETTING_STARTED.md).

## Current state

- `include/constants.h` documents the GPIO assignment used by both room simulations.
- The room diagrams include the entrance and exit relays, granted/denied LEDs, alarm button and buzzer, LCD1602, and 74HC595 LCD shift register.
- `main.c` and `sample_led.c` are implemented. The occupancy, lighting and communications modules remain scaffolding and are not yet listed in `main/CMakeLists.txt`.
- The build-time `COCIS_ROOM_ID` option and `include/room_config.h` are implemented. The current task logs the selected room at boot.

## Room hardware constants

`include/constants.h` is the single source of truth for the room simulation pin names:

| Constant | GPIO | Connected hardware |
| --- | ---: | --- |
| `ENTRANCE_DOOR_RELAY_PIN` | 23 | Top relay / entrance door |
| `ENTRANCE_GRANTED_LED_PIN` | 22 | Top green LED / entrance granted |
| `ENTRANCE_DENIED_LED_PIN` | 21 | Top red LED / entrance denied |
| `EXIT_DOOR_RELAY_PIN` | 25 | Bottom relay / exit door |
| `EXIT_GRANTED_LED_PIN` | 14 | Bottom green LED / exit granted |
| `EXIT_DENIED_LED_PIN` | 27 | Bottom red LED / exit denied |
| `ALARM_PUSH_BUTTON_PIN` | 13 | Alarm push button |
| `ALARM_BUZZER_PIN` | 26 | Alarm buzzer |
| `LCD_RS_PIN` | 19 | LCD1602 RS |
| `LCD_RW_PIN` | 18 | LCD1602 RW |
| `LCD_ENABLE_PIN` | 5 | LCD1602 E |
| `LCD_SHIFT_REGISTER_DATA_PIN` | 15 | 74HC595 DS |
| `LCD_SHIFT_REGISTER_LATCH_PIN` | 2 | 74HC595 STCP |
| `LCD_SHIFT_REGISTER_CLOCK_PIN` | 4 | 74HC595 SHCP |

The LCD light-status messages are retained as `LCD_MESSAGE_LIGHTS_ON_HEX` and `LCD_MESSAGE_LIGHTS_OFF_HEX`. Both `LLT_1A/diagram.json` and `LLT_1B/diagram.json` use this same mapping.

## Room identity

| Room       | `COCIS_ROOM_ID` | Build directory   | Wokwi config          |
| ---------- | ----------------- | ----------------- | --------------------- |
| `LLT_1A` | `1`             | `build/LLT_1A/` | `LLT_1A/wokwi.toml` |
| `LLT_1B` | `2`             | `build/LLT_1B/` | `LLT_1B/wokwi.toml` |

The planned shared header `include/room_config.h` will map `COCIS_ROOM_ID` to:

- the room name (`LLT_1A` or `LLT_1B`),
- the MQTT client ID,
- topics such as `cocis/LLT_1A/status` and `cocis/LLT_1B/status`.

It is not implemented yet; do not add room-specific values directly to individual source files.

## Files and includes

| File                      | Includes                                                        | Responsibility                                        |
| ------------------------- | --------------------------------------------------------------- | ----------------------------------------------------- |
| `idf_ext.py`            | No project headers                                              | Dispatches default `idf.py build` to both rooms     |
| `include/room_config.h` | No project headers                                              | Maps the build-time room ID to room identity and pins |
| `main/main.c`           | `room_config.h`, `occupancy.h`, `lighting.h`, `comms.h` | Starts the room tasks and supplies the room identity  |
| `main/occupancy.c`      | `occupancy.h`, `room_config.h`                              | PIR input, hold-off timer and occupancy state         |
| `main/lighting.c`       | `lighting.h`, `room_config.h`                               | Local light control                                   |
| `main/comms.c`          | `comms.h`, `room_config.h`                                  | MQTT status/config/time messaging                     |
| `main/sample_led.c`     | `sample_led.h` and ESP-IDF headers                            | Current starter task                                    |

Each `.c` file includes its own header. No source file includes another `.c` file; CMake compiles the sources separately and links them together.

## Build

Build both rooms from this directory:

```bash
cd room_node
idf.py build
```

With no room selected, `idf.py build` runs both room builds. To build one room only:

```bash
idf.py -B build/LLT_1A -DCOCIS_ROOM_ID=1 build
idf.py -B build/LLT_1B -DCOCIS_ROOM_ID=2 build
```

Each room produces `embedded-course-work.bin` and `embedded-course-work.elf` inside its own build directory:

```text
build/LLT_1A/embedded-course-work.bin
build/LLT_1A/embedded-course-work.elf
build/LLT_1B/embedded-course-work.bin
build/LLT_1B/embedded-course-work.elf
```

The default build dispatching is implemented in `idf_ext.py`; the CMake room option and shared `include/room_config.h` are implemented.

To flash a specific room build:

```bash
idf.py -B build/LLT_1A -p /dev/ttyUSB0 flash monitor
```

## Run in Wokwi

Each room has its own diagram and Wokwi config:

```text
LLT_1A/
|-- diagram.json
`-- wokwi.toml

LLT_1B/
|-- diagram.json
`-- wokwi.toml
```

The firmware and ELF paths in a room's `wokwi.toml` are relative to that file. They should point at the matching room build, for example:

```toml
[wokwi]
version = 1
firmware = "../build/LLT_1A/flasher_args.json"
elf = "../build/LLT_1A/embedded-course-work.elf"
```

Open the repository root or `room_node` in VS Code, then use **Wokwi: Select Config File** (`F1`) to choose `LLT_1A/wokwi.toml` or `LLT_1B/wokwi.toml` before starting the simulator. Using `flasher_args.json` is preferred for ESP-IDF because it also loads the bootloader and partition table.

Online Wokwi project: [Shared room simulation](https://wokwi.com/projects/478673965757941761)

## Adding the target modules

Add each new `.c` file to `main/CMakeLists.txt`; ESP-IDF does not pick up sources automatically. Headers placed in `include/` need no CMake change. When the room modules are implemented, include `room_config.h` instead of hard-coding the room name or MQTT topics, and update the matching room's `diagram.json` when the PIR sensor or light wiring changes.
