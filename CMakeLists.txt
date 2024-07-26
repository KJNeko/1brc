# /CMakeLists.txt

cmake_minimum_required(VERSION 3.25.0)
project(BillionRowsChallenge LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MESSAGE_LOG_LEVEL DEBUG CACHE STRING "CMake messaging level")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/fgl_cmake_modules)

message("-- CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
string(TOUPPER ${CMAKE_BUILD_TYPE} UPPER_BUILD_TYPE)

PreSetup()

AddFGLExecutable(Main ${CMAKE_CURRENT_SOURCE_DIR}/src)

PostSetup()