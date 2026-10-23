#include "monitoring_commands.h"
#include "esp_console.h"
#include "esp_err.h"
#include <stdio.h>

static int monitoring_command(int argc, char **argv)
{
    (void)argc;
    printf("%s selected. Monitoring implementation is planned.\n", argv[0]);
    return 0;
}

void register_monitoring_commands(void)
{
    static const esp_console_cmd_t commands[] = {
        {.command = "inside", .help = "List people currently inside, including entry times.", .hint = "", .func = monitoring_command},
        {.command = "log", .help = "Display the most recent access records.", .hint = "[n]", .func = monitoring_command},
        {.command = "whois", .help = "Display registry information for a user tag.", .hint = "<UID>", .func = monitoring_command},
        {.command = "denied", .help = "Display the most recent denied access attempts.", .hint = "[n]", .func = monitoring_command},
        {.command = "stats", .help = "Display entry, exit and denial statistics.", .hint = "", .func = monitoring_command},
        {.command = "storage", .help = "Display log storage capacity and health.", .hint = "", .func = monitoring_command},
    };

    for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); ++i) {
        ESP_ERROR_CHECK(esp_console_cmd_register(&commands[i]));
    }
}