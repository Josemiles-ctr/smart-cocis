idf_component_register(SRCS "sample_led.c" "main.c"
                    INCLUDE_DIRS "../include")

if(DEFINED COCIS_ROOM_ID AND NOT COCIS_ROOM_ID STREQUAL "")
    target_compile_definitions(${COMPONENT_LIB} PRIVATE
        COCIS_ROOM_ID=${COCIS_ROOM_ID})
endif()
