# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.22)

if(DEFINED COCIS_ROOM_ID AND NOT COCIS_ROOM_ID STREQUAL "" AND
   NOT COCIS_ROOM_ID MATCHES "^[12]$")
    message(FATAL_ERROR "COCIS_ROOM_ID must be 1 (LLT_1A) or 2 (LLT_1B), got '${COCIS_ROOM_ID}'")
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(smart-cocis-rooms)
