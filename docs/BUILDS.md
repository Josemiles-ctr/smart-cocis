# Firmware Builds

* [ ] The repository contains two ESP-IDF projects:

- `entrance_node`: entrance access-control firmware.
- `room_node`: shared room firmware used by `LLT_1A` and `LLT_1B`.

Run these commands from the repository root after exporting ESP-IDF:

```bash
. "$HOME/.espressif/v6.1/esp-idf/export.sh"
```

## 1. Build the entrance node

```bash
cd entrance_node
idf.py build
```

Output:

```text
entrance_node/build/embedded-course-work.bin
entrance_node/build/embedded-course-work.elf
```

## 2. Build one room

The room project uses the same source files for both rooms. Select the room
with `COCIS_ROOM_ID`; the value is passed to the compiler and mapped by
`room_node/include/room_config.h`.

Build `LLT_1A`:

```bash
cd room_node
idf.py -B build/LLT_1A -DCOCIS_ROOM_ID=1 build
```

Build `LLT_1B`:

```bash
cd room_node
idf.py -B build/LLT_1B -DCOCIS_ROOM_ID=2 build
```

The outputs are written to the selected directory:

```text
room_node/build/LLT_1A/
room_node/build/LLT_1B/
```

Each directory contains its own ELF, binary, bootloader, partition table and
`flasher_args.json`.

## 3. Build both rooms

Run the room project without a room ID:

```bash
cd room_node
idf.py build
```

`room_node/idf_ext.py` dispatches two independent ESP-IDF builds:

```text
build/LLT_1A  with COCIS_ROOM_ID=1
build/LLT_1B  with COCIS_ROOM_ID=2
```

Both images are compiled from the same firmware sources. Only the build-time
room ID changes, allowing `room_config.h` to provide the correct room name and
room-specific identity for each image.
