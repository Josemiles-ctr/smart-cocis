#ifndef ROOM_CONFIG_H
#define ROOM_CONFIG_H

#ifndef COCIS_ROOM_ID
#error "COCIS_ROOM_ID must be defined as 1 (LLT_1A) or 2 (LLT_1B)"
#endif

#if COCIS_ROOM_ID == 1
#define COCIS_ROOM_NAME "LLT 1A"
#define COCIS_MQTT_CLIENT_ID "cocis-room-LLT_1A"
#define COCIS_MQTT_STATUS_TOPIC "cocis/LLT_1A/status"
#elif COCIS_ROOM_ID == 2
#define COCIS_ROOM_NAME "LLT 1B"
#define COCIS_MQTT_CLIENT_ID "cocis-room-LLT_1B"
#define COCIS_MQTT_STATUS_TOPIC "cocis/LLT_1B/status"
#else
#error "COCIS_ROOM_ID must be 1 (LLT 1A) or 2 (LLT 1B)"
#endif

#define COCIS_ROOM_PIR_GPIO 27
#define COCIS_ROOM_LIGHT_GPIO 26

#endif
