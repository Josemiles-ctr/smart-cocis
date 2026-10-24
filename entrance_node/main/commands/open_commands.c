#include "commands/open_commands.h"
#include "esp_console.h"
#include "esp_err.h"
#include <stdio.h>
#include <string.h>

static int open_command(int argc, char **argv)
{
    (void)argc;
    printf("%s selected. This command is available without login; implementation is planned.\n",
           argv[0]);
    return 0;
}

static int schedule_command(int argc, char **argv)
{
    if (argc == 1)
    {
        puts("schedule selected. Showing the current access period is planned.");
    }
    else if (argc == 4 && strcmp(argv[1], "set") == 0)
    {
        puts("schedule set selected. Changing the access period will require administrator authentication.");
    }
    else
    {
        puts("Usage: schedule [set <HH:MM> <HH:MM>]");
    }
    return 0;
}

void register_open_commands(void)
{
    static const esp_console_cmd_t commands[] = {
        {.command = "status",
         .help = "Display controller health and operating state.",
         .hint = "",
         .func = open_command},
        {.command = "count",
         .help = "Display the current number of people inside.",
         .hint = "",
         .func = open_command},
        {.command = "rooms",
         .help = "Display occupancy and light state for all rooms.",
         .hint = "",
         .func = open_command},
        {.command = "nodes",
         .help = "Display connectivity and health for room nodes.",
         .hint = "",
         .func = open_command},
        {.command = "schedule",
         .help = "View the access period or change it as an administrator.",
         .hint = "[set <start> <end>]",
         .func = schedule_command},
        {.command = "time",
         .help = "Display the system clock and synchronization source.",
         .hint = "",
         .func = open_command},
        {.command = "login",
         .help = "Authenticate an administrator session.",
         .hint = "<user> <password>",
         .func = open_command},
        {.command = "logout",
         .help = "End the current administrator session.",
         .hint = "",
         .func = open_command},
    };

    for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); ++i)
    {
        ESP_ERROR_CHECK(esp_console_cmd_register(&commands[i]));
    }
}