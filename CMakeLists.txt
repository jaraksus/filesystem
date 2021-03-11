cmake_minimum_required(VERSION 3.0)

project(mini_fs)

set(SOURCE_EXE main.c)

set(LIB_API src/api.c)
set(LIB_FS src/fs.c)
set(LIB_UTIL src/util.c)

include_directories(headers)

add_library(util STATIC ${LIB_UTIL})
add_library(fs STATIC ${LIB_FS})
add_library(api STATIC ${LIB_API})

add_executable(main ${SOURCE_EXE})

target_link_libraries(fs util)

target_link_libraries(api util)
target_link_libraries(api fs)

target_link_libraries(main api)
target_link_libraries(main fs)
