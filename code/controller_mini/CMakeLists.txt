# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS "/home/andy/esp/esp-protocols/components/esp_websocket_client")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(controller)
