#include "administrator_commands.h"
#include "esp_console.h"
#include "esp_err.h"
#include <stdio.h>
#include <string.h>

static int administrator_command(int argc, char **argv)
{
    (void)argc;
    printf("%s selected. Authentication and administrator checks are planned.\n", argv[0]);
    return 0;
}

static int user_command(int argc, char **argv)
{
    if (argc < 2) {
        puts("Usage: user <add|remove|disable|enable|list> ...");
        return 0;
    }

    if (strcmp(argv[1], "list") == 0) {
        puts("user list selected. Listing registered users is planned.");
    } else {
        printf("user %s selected. User registry management is planned.\n", argv[1]);
    }
    return 0;
}

static int holdoff_command(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    puts("holdoff selected. Setting a room light hold-off time is planned.");
    return 0;
}

void register_administrator_commands(void)
{
    static const esp_console_cmd_t commands[] = {
        {.command = "user", .help = "Add, remove, enable, disable or list registered users.", .hint = "<operation> ...", .func = user_command},
        {.command = "holdoff", .help = "Configure automatic light hold-off duration.", .hint = "<room> <seconds>", .func = holdoff_command},
        {.command = "passwd", .help = "Change the administrator password.", .hint = "", .func = administrator_command},
        {.command = "export", .help = "Export stored access records over the console.", .hint = "", .func = administrator_command},
        {.command = "reboot", .help = "Restart the entrance controller.", .hint = "", .func = administrator_command},
    };

    for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); ++i) {
        ESP_ERROR_CHECK(esp_console_cmd_register(&commands[i]));
    }
}