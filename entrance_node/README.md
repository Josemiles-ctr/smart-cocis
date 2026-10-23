* [ ] 

# Entrance Node

The entrance node is the independent ESP-IDF project for the building entrance.
Its target responsibilities are RFID entry and exit handling, door control,
access scheduling, user registry, logging and communication with the room
nodes.

The current firmware is the starter button/LED example. The entrance-specific
modules and headers are scaffolded for later implementation.

## Build

From this directory, with ESP-IDF exported:

```bash
cd entrance_node
idf.py build
```

The generated firmware is written to:

```text
build/embedded-course-work.bin
build/embedded-course-work.elf
```

The build directory also contains the bootloader, partition table and
`flasher_args.json`.

## Flash

```bash
idf.py -p /dev/ttyUSB0 flash monitor
```

Replace `/dev/ttyUSB0` with the serial port for the ESP32 board.

## Simulation

The entrance Wokwi files are located at the project root:

```text
entrance_node/diagram.json
entrance_node/wokwi.toml
```

Build the firmware before starting Wokwi. The current simulation uses the
starter circuit: a push button on GPIO 14 controls an LED on GPIO 18.

Online Wokwi project: [Entrance node simulation](https://wokwi.com/projects/478673857962207233)

## Source layout

| Path                      | Responsibility                       |
| ------------------------- | ------------------------------------ |
| `main/main.c`           | ESP-IDF application entry point      |
| `main/sample_led.c`     | Current starter button/LED task      |
| `main/rfid_sim.c`       | Planned serial RFID input            |
| `main/access_control.c` | Planned access decision logic        |
| `main/schedule.c`       | Planned access schedule              |
| `main/registry.c`       | Planned user registry                |
| `main/logger.c`         | Planned event logging                |
| `main/cli.c`            | Planned serial command interface     |
| `main/auth.c`           | Planned administrator authentication |
| `main/comms.c`          | Planned room-node communication      |
