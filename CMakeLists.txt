cmake_minimum_required(VERSION 3.16)
project(data_structures_training)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra -Wzero-as-null-pointer-constant -Winit-self -Wfloat-equal -Wno-unused-private-field -Wunused-parameter -Wsign-conversion -Wold-style-cast")

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMake")

add_subdirectory(src/array-list)

FILE(GLOB_RECURSE SOURCES_LIST ${PROJECT_SOURCE_DIR}/src/*.h ${PROJECT_SOURCE_DIR}/src/*.cpp)

include(CppLint)
ADD_STYLE_CHECK_TARGET(lint "${SOURCES_LIST}")