cmake_minimum_required(VERSION 3.0)

project(mini_fs)

if (DEBUG)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
    set (CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
endif()

set(SOURCE_EXE main.c)
set(SERVER_SOURCE src/server.c)
set(CLIENT_SOURCE src/client.c)

set(LIB_API src/api.c)
set(LIB_FS src/fs.c)
set(LIB_UTIL src/util.c)
set(LIB_NETWORK_UTIL src/network_util.c)

include_directories(headers)

add_library(util STATIC ${LIB_UTIL})
add_library(fs STATIC ${LIB_FS})
add_library(api STATIC ${LIB_API})
add_library(network_util STATIC ${LIB_NETWORK_UTIL})

add_executable(main ${SOURCE_EXE})
add_executable(server ${SERVER_SOURCE})
add_executable(client ${CLIENT_SOURCE})

target_link_libraries(fs util)

target_link_libraries(api util)
target_link_libraries(api fs)

target_link_libraries(main api)
target_link_libraries(main fs)

target_link_libraries(server network_util)
target_link_libraries(server api)
target_link_libraries(server fs)

target_link_libraries(client network_util)
