#include "emergency_commands.h"
#include "esp_console.h"
#include "esp_err.h"
#include <stdio.h>

static int emergency_command(int argc, char **argv)
{
    (void)argc;
    printf("%s selected. Emergency command behavior is planned.\n", argv[0]);
    return 0;
}

void register_emergency_commands(void)
{
    static const esp_console_cmd_t commands[] = {
        {.command = "light", .help = "Override the operating mode of a room light.", .hint = "<room> auto|on|off", .func = emergency_command},
        {.command = "override", .help = "Enable or disable emergency exit override.", .hint = "exit <on|off>", .func = emergency_command},
        {.command = "evacuate", .help = "Display the occupancy list for an evacuation headcount.", .hint = "", .func = emergency_command},
        {.command = "alerts", .help = "Display active controller and room-node alerts.", .hint = "", .func = emergency_command},
    };

    for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); ++i) {
        ESP_ERROR_CHECK(esp_console_cmd_register(&commands[i]));
    }
}