#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "freertos/task.h"

#define LED GPIO_NUM_18
#define BUTTON GPIO_NUM_14

void initialize_pins()
{
    gpio_reset_pin(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);

    gpio_reset_pin(BUTTON);
    gpio_set_direction(BUTTON, GPIO_MODE_INPUT);
    gpio_pullup_en(BUTTON);
}

void run_led_task(void *pvParameters)
{

    while (1)
    {
        int button_state = gpio_get_level(BUTTON);
        if (button_state == 0)
        {
            gpio_set_level(LED, 1);
        }
        else
        {
            gpio_set_level(LED, 0);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void exported_run_led_task()
{
    initialize_pins();
    xTaskCreate(run_led_task, "LED Task", 2048, NULL, 5, NULL);
}