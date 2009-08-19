

cmake_minimum_required (VERSION 2.6)
project (ppi-server-project)

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_MODULE_PATH cmake)

add_subdirectory (src)
#add_subdirectory (src/bin)

