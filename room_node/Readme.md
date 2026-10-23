* [ ] 

# Room Node

The Room Node is one of the two ESP-IDF applications in the **CoCIS Smart Control and Monitoring System** repository. It detects occupancy and controls the light in a lecture room.

Two rooms are in scope: **`LLT_1A`** and **`LLT_1B`**. Both run the same `room_node` firmware. In the target design they are distinguished by a build-time room ID, so each room gets its own build output, Wokwi config and MQTT identity.

The canonical system design is in the [root README](../Readme.md), and the build/development guide is [docs/GETTING_STARTED.md](../docs/GETTING_STARTED.md).

## Current state

- The project builds and runs the starter button/LED example: button on **GPIO 14**, LED on **GPIO 18**.
- `main.c`, `sample_led.c` and `include/sample_led.h` are implemented.
- `occupancy.c`, `lighting.c`, `comms.c` and their headers are empty scaffolding. They are not yet listed in `main/CMakeLists.txt`.
- `LLT_1A/diagram.json` and `LLT_1B/diagram.json` currently contain only the starter button and LED, not the target PIR sensor and room light.
- The build-time `COCIS_ROOM_ID` option and `include/room_config.h` are implemented. The current starter task logs the selected room at boot; occupancy, lighting and communications remain scaffolding.

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
| `main/sample_led.c`     | `sample_led.h` and ESP-IDF headers                            | Current starter button/LED example                    |

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
