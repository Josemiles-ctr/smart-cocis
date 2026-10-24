# Getting Started

The repository is a **multi-project ESP-IDF workspace**. It contains two independent applications:

| Directory         | Target role                                                                              | Current firmware           |
| ----------------- | ---------------------------------------------------------------------------------------- | -------------------------- |
| `entrance_node` | RFID entry/exit, door control, access schedule, user registry, logs and serial interface | Starter button/LED example |
| `room_node`     | Occupancy detection and local lighting control for `LLT_1A` and `LLT_1B`                | Starter button/LED example |

In the target system, `LLT_1A` and `LLT_1B` run the same `room_node` firmware and are distinguished by a build-time room ID. The system design is complete, but most application modules are still empty scaffolding. The sections below describe the repository as it exists today and how to extend it safely.

---

## 1. Prerequisites

| Requirement        | Notes                                                                                                               |
| ------------------ | ------------------------------------------------------------------------------------------------------------------- |
| ESP-IDF            | **v6.1** was used to generate the project's `sdkconfig` files. Other v5.x/v6.x may work but are not tested. |
| Python 3           | Required by `idf.py` and the ESP-IDF build system.                                                                |
| CMake              | **3.22 or newer** (enforced by each node's root `CMakeLists.txt`).                                          |
| Ninja              | Default ESP-IDF build tool.                                                                                         |
| USB driver         | CP210x/CH340 driver if you flash a real ESP32 DevKit.                                                               |
| VS Code (optional) | Recommended with the **ESP-IDF**, **C/C++** and **Wokwi** extensions.                             |

Install ESP-IDF using the official guide: [https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/).

For the installation used in this workspace, load the environment in every new shell:

```bash
# Adjust this path if ESP-IDF is installed elsewhere.
. "$HOME/.espressif/v6.1/esp-idf/export.sh"
```

Verify the tools:

```bash
idf.py --version
echo "$IDF_PATH"
```

> Each node's root `CMakeLists.txt` reads `$IDF_PATH` at configure time. If the variable is empty, CMake fails with an error such as `Could not find the ESP-IDF...`. Run `export.sh` in the same shell that invokes `idf.py`.

---

## 2. Repository layout

```
smart-cocis/
├── Readme.md                     # System design and target behaviour
├── docs/
│   └── GETTING_STARTED.md        # This guide
├── entrance_node/                # ESP-IDF application (project)
│   ├── CMakeLists.txt            # Root project definition
│   ├── main/
│   │   ├── CMakeLists.txt        # Registers the `main` component
│   │   ├── main.c                # app_main()
│   │   ├── sample_led.c          # Starter button/LED driver
│   │   └── ...                   # Empty scaffolding for system modules
│   ├── include/
│   │   ├── sample_led.h          # Public interface for sample_led.c
│   │   └── ...                   # Empty scaffolding for module headers
│   ├── simulations/sample_led.json  # Wokwi diagram copy used by the starter example
│   ├── diagram.json              # Active Wokwi circuit
│   ├── sdkconfig                 # Target and Kconfig options
│   └── wokwi.toml                # Firmware/ELF paths for Wokwi
├── room_node/                    # ESP-IDF application (project)
│   ├── CMakeLists.txt            # Root project definition
│   ├── Readme.md                 # Room-node-specific notes
│   ├── idf_ext.py                # Dispatches a default build to both rooms
│   ├── LLT_1A/                   # Wokwi project for lecture room LLT_1A
│   │   ├── diagram.json
│   │   └── wokwi.toml
│   ├── LLT_1B/                   # Wokwi project for lecture room LLT_1B
│   │   ├── diagram.json
│   │   └── wokwi.toml
│   ├── main/
│   │   ├── CMakeLists.txt        # Registers the `main` component
│   │   ├── main.c                # app_main()
│   │   ├── sample_led.c          # Starter button/LED driver
│   │   └── ...                   # Empty scaffolding for room modules
│   ├── include/
│   │   ├── sample_led.h          # Public interface for sample_led.c
│   │   ├── room_config.h         # Planned room identity/configuration header
│   │   └── ...                   # Empty scaffolding for module headers
│   ├── simulations/sample_led.json
│   ├── sdkconfig                 # Target and Kconfig options
│   └── .gitignore                # Ignores room build output
├── sdcard/                       # Files intended for the simulated SD card
│   ├── access.csv
│   ├── room.csv
│   └── system.log
└── .vscode/                      # Local IntelliSense helpers (gitignored)
```

Both node directories are **separate ESP-IDF projects**, not components of one project. Each has its own `sdkconfig` and build output. The room project is built twice into `room_node/build/LLT_1A/` and `room_node/build/LLT_1B/`; each room also has its own Wokwi `diagram.json` and `wokwi.toml`.

### Room identity and include layout

`LLT_1A` and `LLT_1B` share the same `room_node` sources. The room is selected at build time with `COCIS_ROOM_ID`:

| Room     | `COCIS_ROOM_ID` | Build directory          |
| -------- | --------------- | ------------------------ |
| `LLT_1A` | `1`             | `room_node/build/LLT_1A` |
| `LLT_1B` | `2`             | `room_node/build/LLT_1B` |

`room_node/include/room_config.h` maps the build-time ID to the room name, MQTT client ID and MQTT topics such as `cocis/LLT_1A/status` and `cocis/LLT_1B/status`. `room_node/include/constants.h` contains the implemented room-simulation GPIO assignments and LCD light-status messages.

Planned include relationships:

| File           | Includes                                                | Responsibility                                     |
| -------------- | ------------------------------------------------------- | -------------------------------------------------- |
| `main.c`       | `room_config.h`, `occupancy.h`, `lighting.h`, `comms.h` | Starts the room tasks and supplies the room identity |
| `occupancy.c`  | `occupancy.h`, `room_config.h`                          | PIR input, hold-off timer and occupancy state      |
| `lighting.c`   | `lighting.h`, `room_config.h`                           | Local light control                                |
| `comms.c`      | `comms.h`, `room_config.h`                              | MQTT status/config/time messaging                  |
| `sample_led.c` | `sample_led.h` and ESP-IDF headers                      | Current starter task                                |

Each `.c` file includes its own header. No source file includes another `.c` file; CMake compiles the sources separately and links them together.

### File responsibilities

| File / directory            | Responsibility                                                                                                   |
| --------------------------- | ---------------------------------------------------------------------------------------------------------------- |
| `Readme.md`               | Target system architecture, behaviour, CLI, storage and testing specification.                                   |
| `docs/GETTING_STARTED.md` | Toolchain setup, repository layout, build/simulation steps and development conventions.                          |
| `room_node/idf_ext.py`    | Builds both `LLT_1A` and `LLT_1B` when `idf.py build` is run without a room selection.                           |
| `*/CMakeLists.txt`        | Declares the ESP-IDF project and its output name (`embedded-course-work`).                                     |
| `*/main/CMakeLists.txt`   | Lists the compiled `.c` files and exports the node's `include/` directory.                                   |
| `*/main/main.c`           | Defines `app_main()`, the ESP-IDF entry point. Currently starts the LED task.                                  |
| `*/main/sample_led.c`     | Starter hardware driver: GPIO initialisation, FreeRTOS task and the public `exported_run_led_task()` function. |
| `*/include/`              | Public headers for the node. Skipped headers are scaffolding for future modules.                                 |
| `room_node/include/room_config.h` | Shared room identity header. Maps `COCIS_ROOM_ID` to room name, MQTT client ID, topics and room GPIOs. |
| `room_node/LLT_1A/`       | Wokwi `diagram.json` and `wokwi.toml` for lecture room `LLT_1A`.                                                |
| `room_node/LLT_1B/`       | Wokwi `diagram.json` and `wokwi.toml` for lecture room `LLT_1B`.                                                |
| `room_node/build/LLT_1A/` | Generated `LLT_1A` build output. Ignored by Git.                                                               |
| `room_node/build/LLT_1B/` | Generated `LLT_1B` build output. Ignored by Git.                                                               |
| `*/sdkconfig`             | Generated configuration. Commit changes when you deliberately change Kconfig options.                            |
| `entrance_node/diagram.json` | Wokwi circuit for the entrance node. It currently contains only the starter button and LED.                 |
| `entrance_node/wokwi.toml`   | Tells Wokwi which entrance-node firmware and ELF to load.                                                   |
| `sdcard/`                 | Data files for the target SD-card logging feature.                                                               |
| `.vscode/`                | Local editor/inclusion configuration. Machine-specific and gitignored.                                           |

---

## 3. Build, flash and simulate

### Build a node

Build the entrance node:

```bash
cd entrance_node
idf.py build
```

Build each room from the shared room project. With no room selected, the project extension builds both rooms:

```bash
cd room_node
idf.py build
```

To build only one room, select it explicitly:

```bash
idf.py -B build/LLT_1A -DCOCIS_ROOM_ID=1 build
idf.py -B build/LLT_1B -DCOCIS_ROOM_ID=2 build
```

| Room     | `COCIS_ROOM_ID` | Build output      |
| -------- | --------------- | ----------------- |
| `LLT_1A` | `1`             | `build/LLT_1A/`   |
| `LLT_1B` | `2`             | `build/LLT_1B/`   |

The default `idf.py build` dispatches through `room_node/idf_ext.py`. A single-room build must provide its room ID with `-DCOCIS_ROOM_ID=1` or `-DCOCIS_ROOM_ID=2`.

The target is already set in each `sdkconfig` (`CONFIG_IDF_TARGET="esp32"`). Run `idf.py set-target esp32` only on a fresh checkout, after deleting `sdkconfig`, or when switching to a different chip.

Both projects currently use the CMake project name `embedded-course-work`, so each build directory produces the same file names:

```
<build-dir>/embedded-course-work.elf   # ELF with debug symbols
<build-dir>/embedded-course-work.bin   # flashable application image
<build-dir>/bootloader/bootloader.bin
<build-dir>/partition_table/partition-table.bin
<build-dir>/compile_commands.json      # used by IntelliSense / clangd
```

For the entrance node, `<build-dir>` is `entrance_node/build/`. For the rooms, it is `room_node/build/LLT_1A/` or `room_node/build/LLT_1B/`.

### Flash and monitor a real board

```bash
idf.py -p /dev/ttyUSB0 flash monitor
```

To flash a specific room build, keep its build directory:

```bash
cd room_node
idf.py -B build/LLT_1A -p /dev/ttyUSB0 flash monitor
```

Exit the monitor with `Ctrl+]`.

### Run in Wokwi

1. Build the entrance node or the room you want to simulate.
2. Open the repository root or the relevant project folder in VS Code.
3. Use **Wokwi: Select Config File** (`F1`) to choose the config, because the room configs are in subfolders.
4. Run **Wokwi: Start Simulator** from the command palette (`Ctrl+Shift+P`).

| Simulation | Wokwi config                   | Firmware path                         | ELF path                                     |
| ---------- | ------------------------------ | ------------------------------------- | -------------------------------------------- |
| Entrance   | `entrance_node/wokwi.toml`     | `build/embedded-course-work.bin`      | `build/embedded-course-work.elf`             |
| `LLT_1A`   | `room_node/LLT_1A/wokwi.toml` | `../build/LLT_1A/flasher_args.json`   | `../build/LLT_1A/embedded-course-work.elf`   |
| `LLT_1B`   | `room_node/LLT_1B/wokwi.toml` | `../build/LLT_1B/flasher_args.json`   | `../build/LLT_1B/embedded-course-work.elf`   |

The paths in a room's `wokwi.toml` are relative to that file, not to the repository root. Each room config points at its own build directory so Wokwi loads the firmware built with the matching `COCIS_ROOM_ID`. ESP-IDF's `flasher_args.json` is preferred over the application `.bin` because it also specifies the bootloader and partition table.

The entrance node and the two rooms use separate Wokwi configs. Run one Wokwi session per config, and select `LLT_1A` or `LLT_1B` deliberately before starting the room simulation.

If you rename `project(...)` in a node's root `CMakeLists.txt`, update the matching `wokwi.toml` paths too.

The room simulations now contain the full room wiring. The GPIO names and LCD light-status message bytes are documented in `room_node/include/constants.h`; keep that header and both room `diagram.json` files synchronized when wiring changes. The RFID serial commands and room application modules are still not implemented.

---

## 4. How each project is built

ESP-IDF is a component-based build system on top of CMake. The flow for one node is:

```
CMakeLists.txt (project root)
  │  include($ENV{IDF_PATH}/tools/cmake/project.cmake)
  │  project(embedded-course-work)
  │
  ├─ ESP-IDF scans the project directory for component subdirectories
  │    └─ finds main/ (a directory containing CMakeLists.txt)
  │
  ▼
main/CMakeLists.txt
  │  idf_component_register(SRCS "sample_led.c" "main.c"
  │                          INCLUDE_DIRS "../include")
  │
  ├─ compiles main/sample_led.c  ──┐
  ├─ compiles main/main.c        ──┤
  └─ adds -I<node>/include          │
                                    ▼
                     <build-dir>/embedded-course-work.elf / .bin
```

The room project uses the same flow, but the output directory is selected per room with `idf.py -B build/LLT_1A -DCOCIS_ROOM_ID=1` or `idf.py -B build/LLT_1B -DCOCIS_ROOM_ID=2`.

### The project root `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.22)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(embedded-course-work)
```

- `include($ENV{IDF_PATH}/tools/cmake/project.cmake)` loads the ESP-IDF build logic.
- `project(embedded-course-work)` discovers components, generates configuration headers, builds the bootloader and partition table, and creates the final image.
- The `project()` argument is also the output name. See the note above about `wokwi.toml`.

### The `main` component

Every directory containing a `CMakeLists.txt` that calls `idf_component_register()` is a component. `main/` is the only component in each node today.

```cmake
idf_component_register(SRCS "sample_led.c" "main.c"
                    INCLUDE_DIRS "../include")
```

- `SRCS` lists every `.c` file to compile. **ESP-IDF does not glob sources**, so each new `.c` file must be added here. The empty module files in `main/` are not compiled until they are listed.
- `INCLUDE_DIRS` exports `../include` to this component and to anything that depends on it. Paths are relative to `main/`.
- New headers in `include/` do not need a CMake change. New `.c` files do.
- `main` automatically requires all other components, which is why `sample_led.c` can include `driver/gpio.h` and `freertos/FreeRTOS.h` without an explicit `REQUIRES` list.

### Entry point and starter behaviour

ESP-IDF creates the main FreeRTOS task and calls:

```c
void app_main(void)
{
    exported_run_led_task();
}
```

`app_main()` must keep exactly that name and signature. Do not define a normal `main()`.

`sample_led.c` remains the starter hardware task. The complete room simulation pin mapping is centralized in `room_node/include/constants.h` and must stay in sync with both room `diagram.json` files.

---

## 5. Adding or changing code

### Add a source file to a node

1. Create `main/my_module.c` and, if it needs a public interface, `include/my_module.h`.
2. Add the source to that node's `main/CMakeLists.txt`:

   ```cmake
   idf_component_register(SRCS "sample_led.c" "main.c" "my_module.c"
                       INCLUDE_DIRS "../include")
   ```
3. Include the header where needed:

   ```c
   #include "my_module.h"
   ```
4. Rebuild. CMake reconfigures automatically when `CMakeLists.txt` changes.

When implementing one of the scaffolded modules (`rfid_sim.c`, `access_control.c`, `schedule.c`, `registry.c`, `logger.c`, `cli.c`, `auth.c`, `comms.c`, `occupancy.c`, `lighting.c`), register it in the corresponding node's `main/CMakeLists.txt` as part of the same change.

### Add a header

Place it in the node's `include/` directory. No CMake change is required because `../include` is already exported.

### Add a reusable component

For code shared by several modules, create a component directory inside the node:

```
<node>/components/led/
├── CMakeLists.txt
├── include/led.h
└── led.c
```

```cmake
idf_component_register(SRCS "led.c"
                       INCLUDE_DIRS "include"
                       REQUIRES driver)
```

ESP-IDF discovers components in the project directory automatically. Unlike `main`, an ordinary component must list its dependencies in `REQUIRES` (for example `driver` for `driver/gpio.h`).

### Change configuration

```bash
idf.py menuconfig
```

This rewrites `sdkconfig` and backs up the previous version to `sdkconfig.old`. Review `sdkconfig` changes before committing. Do **not** edit `build/config/sdkconfig.h`; it is generated.

### Clean rebuilds

```bash
idf.py reconfigure   # re-run CMake only
idf.py fullclean     # delete build/ (needed after large CMake or toolchain changes)
```

---

## 6. VS Code and IntelliSense

The C/C++ extension needs ESP-IDF include paths and compiler flags. This workspace uses the `compile_commands.json` generated by each node's build.

- Each node has a `.vscode/c_cpp_properties.json`. The entrance node points at `entrance_node/build/compile_commands.json`; for the room node, point it at `room_node/build/LLT_1A/compile_commands.json` or `room_node/build/LLT_1B/compile_commands.json`.
- The repository root has `.vscode/c_cpp_properties.json`, which points at `.vscode/compile_commands.json`. That file merges the node databases so IntelliSense works when the repository root is open. With the two-room build, include the entrance build and both room build directories in the merge.
- `.vscode/merge_compile_commands.py` regenerates the merged database.
- `.vscode/tasks.json` exposes **ESP-IDF: Update IntelliSense compile commands**, which runs the merge script.
- These files are machine-specific and are ignored by Git. `.vscode/settings.json` records the local ESP-IDF path and C/C++ defaults.

After a build, if include paths look stale:

1. Run **ESP-IDF: Update IntelliSense compile commands** from **Terminal → Run Task**.
2. Run **Developer: Reload Window**.
3. If errors persist, run **C/C++: Reset IntelliSense Database**.

A fresh clone without `.vscode/` can still be used from the command line; build both nodes first so the `compile_commands.json` files exist, then recreate the editor configuration if you want IntelliSense.

---

## 7. Contributing workflow

1. **Branch from `main`:**

   ```bash
   git checkout -b feature/short-description
   ```
2. **Keep commits focused.** One logical change per commit, with imperative messages such as `Add debounce to button task`.
3. **Build before committing.** The affected node or room must compile. Use `idf.py build` for the entrance node and the relevant `-B build/LLT_1A` or `-B build/LLT_1B` command for a room. If you changed the circuit, also run the Wokwi simulation.
4. **Do not commit generated or local files.**

   - `build/` is gitignored; never force-add it.
   - `.vscode/` is gitignored; keep machine-specific paths out of commits.
   - `sdkconfig.old` is a backup and should not be committed.
5. **Commit `sdkconfig` when configuration changes.** It pins the target and Kconfig options. Avoid unrelated reformatting because the file is generated and large.
6. **Match the existing style.** Four-space indentation in C, `snake_case` for functions and variables, include guards in headers, one responsibility per file.
7. **Keep coupled files in sync:**

   - pin `#define`s ↔ that room's `diagram.json` connections,
   - `project()` name ↔ `wokwi.toml` firmware/ELF paths,
   - room ID/build directory ↔ the selected room's `wokwi.toml`,
   - `SRCS`/`INCLUDE_DIRS` ↔ the files that actually exist,
   - `room_config.h` ↔ the room slug, MQTT client ID and topics,
   - target behaviour ↔ `Readme.md` and this guide.
8. **Open a pull request.** Describe what changed, how it was tested (build, hardware or Wokwi), and include a serial log or screenshot for behaviour changes.

### Checklist before opening a PR

- [ ] The entrance build, or the affected `LLT_1A`/`LLT_1B` room build, succeeds.
- [ ] No `build/`, `.vscode/` or `sdkconfig.old` files are staged.
- [ ] New `.c` files are listed in the correct `main/CMakeLists.txt`.
- [ ] New headers are in an exported `include/` directory.
- [ ] Pin changes are mirrored in the affected room's `diagram.json`.
- [ ] Output-name changes are mirrored in the matching `wokwi.toml`.
- [ ] `Readme.md` and this guide are updated when behaviour or setup changes.

---

## 8. Troubleshooting

| Symptom                                                   | Likely cause / fix                                                                                                             |
| --------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------ |
| `IDF_PATH` is empty or `project.cmake` not found      | Run `. "$HOME/.espressif/v6.1/esp-idf/export.sh"` in the shell that invokes `idf.py`.                                      |
| `CMake Error ... CMAKE_C_COMPILER not found`            | The ESP-IDF environment was not exported, or `idf.py` was run from the wrong environment.                                    |
| `fatal error: sample_led.h: No such file or directory`  | `../include` is missing from `INCLUDE_DIRS`, or the header is outside `include/`.                                        |
| `undefined reference to exported_run_led_task`          | `sample_led.c` is missing from `SRCS`, or the definition does not match the header.                                        |
| `fatal error: driver/gpio.h: No such file or directory` | The file was moved into a component without adding `driver` to `REQUIRES`.                                                 |
| A new module is never compiled                            | Its `.c` file is not listed in `main/CMakeLists.txt`. ESP-IDF does not glob source files.                                  |
| Wokwi says the firmware was not found                     | Build the correct room, confirm the selected `wokwi.toml`, and check that its `../build/LLT_1A/...` or `../build/LLT_1B/...` paths match the build you just ran. |
| Wokwi loads the wrong room's firmware                      | Use **Wokwi: Select Config File** and choose the matching room's `wokwi.toml` before starting the simulator.              |
| Wokwi circuit does nothing                                | Check that the pin `#define`s match the selected room's `diagram.json`, and that the button pulls the pin to GND.          |
| `Permission denied` on `/dev/ttyUSB0`                 | Add your user to the `dialout` group (then log out and back in), or select the correct serial port.                          |
| Stale build after CMake edits                             | Run `idf.py reconfigure`, or `idf.py fullclean && idf.py build`.                                                           |
| IntelliSense cannot find ESP-IDF headers                  | Build the node/room, run the merge task, reload the window, and reset the IntelliSense database if needed.                     |
| IntelliSense shows errors only at the repository root     | Run the **ESP-IDF: Update IntelliSense compile commands** task so `.vscode/compile_commands.json` includes the entrance and room builds. |
