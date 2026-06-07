cmake_minimum_required(VERSION 3.5)

project(esp32_bluetooth_speaker C)

set(EXTRA_COMPONENT_DIRS main)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project_info(DESCRIPTION "ESP32 Bluetooth Speaker with PAM8403 Amplifier and 16x2 LCD")
