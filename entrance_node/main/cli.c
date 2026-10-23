#include "cli.h"
#include "administrator_commands.h"
#include "emergency_commands.h"
#include "esp_console.h"
#include "esp_err.h"
#include "esp_stdio.h"
#include "monitoring_commands.h"
#include "open_commands.h"
#include <stdio.h>
#include <string.h>

#define CLI_CYAN "\033[36m"
#define CLI_GREEN "\033[32m"
#define CLI_YELLOW "\033[33m"
#define CLI_RESET "\033[0m"

static void print_help_entry(const char *command, const char *hint, const char *description)
{
    printf("  " CLI_GREEN "%-10s" CLI_RESET " %-30s %s\r\n",
           command, hint, description);
}

static int help_handler(int argc, char **argv);
static int commands_handler(int argc, char **argv);

static const esp_console_cmd_t help_command = {
    .command = "help",
    .help = "Display the CoCIS command reference or help for one command",
    .hint = "[command]",
    .func = help_handler,
};

static const esp_console_cmd_t commands_command = {
    .command = "commands",
    .help = "Display the command catalogue grouped by access level",
    .hint = "",
    .func = commands_handler,
};

static int help_handler(int argc, char **argv)
{
    if (argc > 2 || (argc == 2 && strcmp(argv[1], "--help") != 0 &&
                     strcmp(argv[1], "-h") != 0)) {
        printf("Usage: help [command]\r\n");
        printf("Run 'help <command>' to view detailed usage.\r\n");
        return 1;
    }

    if (argc == 2) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            printf("\r\n" CLI_CYAN "HELP(1)" CLI_RESET "                 CoCIS Entrance CLI\r\n\r\n");
            printf("Usage: help [command]\r\n");
            printf("Display the command catalogue or detailed information for one command.\r\n");
            return 0;
        }

        if (strcmp(argv[1], "schedule") == 0) {
            printf("\r\n" CLI_CYAN "schedule" CLI_RESET " - View or change the access schedule\r\n");
            printf("Usage: schedule\r\n");
            printf("       schedule set <HH:MM> <HH:MM>\r\n");
            printf("The first form displays the active access period. The 'set' form\r\n");
            printf("changes it and requires an authenticated administrator session.\r\n");
            return 0;
        }
        if (strcmp(argv[1], "user") == 0) {
            printf("\r\n" CLI_CYAN "user" CLI_RESET " - Manage registered users\r\n");
            printf("Usage: user add <UID> <name>\r\n");
            printf("       user remove <UID>\r\n");
            printf("       user disable <UID>\r\n");
            printf("       user enable <UID>\r\n");
            printf("       user list\r\n");
            printf("All user-management operations require administrator authentication.\r\n");
            return 0;
        }
        if (strcmp(argv[1], "light") == 0) {
            printf("\r\n" CLI_CYAN "light" CLI_RESET " - Set a room light operating mode\r\n");
            printf("Usage: light <room> auto|on|off\r\n");
            printf("Select automatic control, force the light on, or force it off.\r\n");
            return 0;
        }
        if (strcmp(argv[1], "override") == 0) {
            printf("\r\n" CLI_CYAN "override" CLI_RESET " - Control the emergency exit override\r\n");
            printf("Usage: override exit <on|off>\r\n");
            printf("Enable or disable emergency exit override mode.\r\n");
            return 0;
        }
        printf("No detailed help is available for '%s'. Run 'help' to list commands.\r\n",
               argv[1]);
        return 1;
    }

    printf("\r\n" CLI_CYAN "CoCIS Entrance Control CLI" CLI_RESET "\r\n");
    printf("Command reference for access control, monitoring, administration and safety.\r\n\r\n");
    printf(CLI_GREEN "OPEN ACCESS" CLI_RESET "\r\n");
    print_help_entry("status", "", "Display controller health and current operating state.");
    print_help_entry("count", "", "Display the current number of people inside.");
    print_help_entry("rooms", "", "Display occupancy and light state for all rooms.");
    print_help_entry("nodes", "", "Display connectivity and health for room nodes.");
    print_help_entry("schedule", "[set <start> <end>]", "View the access period; administrators can change it.");
    print_help_entry("time", "", "Display the system clock and its synchronization source.");
    print_help_entry("login", "<user> <password>", "Authenticate an administrator session.");
    print_help_entry("logout", "", "End the current administrator session.");
    printf("\r\n" CLI_GREEN "MONITORING" CLI_RESET "\r\n");
    print_help_entry("inside", "", "List people currently inside, including entry times.");
    print_help_entry("log", "[n]", "Display the most recent access records.");
    print_help_entry("whois", "<UID>", "Display registry information for a user tag.");
    print_help_entry("denied", "[n]", "Display the most recent denied access attempts.");
    print_help_entry("stats", "", "Display entry, exit and denial statistics.");
    print_help_entry("storage", "", "Display log storage capacity and health.");
    printf("\r\n" CLI_YELLOW "ADMINISTRATION" CLI_RESET " (authenticated)\r\n");
    print_help_entry("user", "<operation> ...", "Add, remove, enable, disable or list users.");
    print_help_entry("holdoff", "<room> <seconds>", "Configure automatic light hold-off duration.");
    print_help_entry("passwd", "", "Change the administrator password.");
    print_help_entry("export", "", "Export stored access records over the console.");
    print_help_entry("reboot", "", "Restart the entrance controller.");
    printf("\r\n" CLI_YELLOW "EMERGENCY" CLI_RESET "\r\n");
    print_help_entry("light", "<room> auto|on|off", "Override the operating mode of a room light.");
    print_help_entry("override", "exit <on|off>", "Enable or disable emergency exit override.");
    print_help_entry("evacuate", "", "Display the occupancy list for an evacuation headcount.");
    print_help_entry("alerts", "", "Display active controller and room-node alerts.");
    printf("\r\nUse 'help <command>' for detailed usage. Commands are placeholders until implemented.\r\n");
    return 0;
}

static int commands_handler(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    return help_handler(1, argv);
}

void cli_start(void)
{
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    esp_console_dev_uart_config_t uart_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();

    repl_config.prompt = CLI_CYAN "cocis" CLI_RESET "> ";
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&uart_config, &repl_config, &repl));
    ESP_ERROR_CHECK(esp_console_cmd_deregister("help"));
    ESP_ERROR_CHECK(esp_console_cmd_register(&help_command));
    ESP_ERROR_CHECK(esp_console_cmd_register(&commands_command));
    register_open_commands();
    register_monitoring_commands();
    register_administrator_commands();
    register_emergency_commands();

    printf("\r\n" CLI_CYAN "========================================" CLI_RESET "\r\n");
    printf(CLI_CYAN "        CoCIS Entrance Control           " CLI_RESET "\r\n");
    printf(CLI_CYAN "========================================" CLI_RESET "\r\n");
    printf("Type " CLI_GREEN "commands" CLI_RESET " for grouped commands or "
           CLI_GREEN "help" CLI_RESET " for detailed help.\r\n");
    ESP_ERROR_CHECK(esp_console_start_repl(repl));
}
