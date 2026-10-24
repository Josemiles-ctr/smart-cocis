#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "driver/gpio.h"

/* LCD messages. */
#define LCD_MESSAGE_LIGHTS_ON_HEX {0x4, 0x49, 0x47, 0x48, 0x54, 0x53, 0x20, 0x4F, 0x4E}
#define LCD_MESSAGE_LIGHTS_OFF_HEX {0x4, 0x49, 0x47, 0x48, 0x54, 0x53, 0x20, 0x4F, 0x46, 0x46}

/* Entrance door: top relay and status indicators. */
#define ENTRANCE_DOOR_RELAY_PIN GPIO_NUM_23
#define ENTRANCE_GRANTED_LED_PIN GPIO_NUM_22
#define ENTRANCE_DENIED_LED_PIN GPIO_NUM_21

/* Exit door: bottom relay and status indicators. */
#define EXIT_DOOR_RELAY_PIN GPIO_NUM_25
#define EXIT_GRANTED_LED_PIN GPIO_NUM_14
#define EXIT_DENIED_LED_PIN GPIO_NUM_27

/* Alarm controls. */
#define ALARM_PUSH_BUTTON_PIN GPIO_NUM_13
#define ALARM_BUZZER_PIN GPIO_NUM_26

/* LCD1602 control pins. */
#define LCD_RS_PIN GPIO_NUM_19
#define LCD_RW_PIN GPIO_NUM_18
#define LCD_ENABLE_PIN GPIO_NUM_5

/* 74HC595 shift register used for the LCD data bus. */
#define LCD_SHIFT_REGISTER_DATA_PIN GPIO_NUM_15
#define LCD_SHIFT_REGISTER_LATCH_PIN GPIO_NUM_2
#define LCD_SHIFT_REGISTER_CLOCK_PIN GPIO_NUM_4

#endif