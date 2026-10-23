# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.22)

if(NOT DEFINED COCIS_ROOM_ID OR COCIS_ROOM_ID STREQUAL "")
    message(FATAL_ERROR "COCIS_ROOM_ID must be provided as 1 or 2.")
endif()

if(NOT COCIS_ROOM_ID MATCHES "^[12]$")
    message(FATAL_ERROR "COCIS_ROOM_ID must be 1 (LLT_1A) or 2 (LLT_1B), got '${COCIS_ROOM_ID}'")
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(embedded-course-work)
