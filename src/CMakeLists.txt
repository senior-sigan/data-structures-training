project(data_structures CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra -Wzero-as-null-pointer-constant -Winit-self -Wfloat-equal -Wno-unused-private-field -Wunused-parameter -Wsign-conversion -Wold-style-cast")

FILE(GLOB_RECURSE SOURCES_LIST ${PROJECT_SOURCE_DIR}/*.h ${PROJECT_SOURCE_DIR}/*.cpp)

add_library(${PROJECT_NAME} ${SOURCES_LIST})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

include(CppLint)
ADD_STYLE_CHECK_TARGET(lint "${SOURCES_LIST}")