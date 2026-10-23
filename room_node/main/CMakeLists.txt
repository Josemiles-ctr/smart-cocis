idf_component_register(SRCS "sample_led.c" "main.c"
                    INCLUDE_DIRS "../include")

target_compile_definitions(${COMPONENT_LIB} PRIVATE
    COCIS_ROOM_ID=${COCIS_ROOM_ID})
