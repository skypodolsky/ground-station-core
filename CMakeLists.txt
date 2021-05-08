cmake_minimum_required(VERSION 3.5.1)
set(TARGET_UTIL gsc)

project(${TARGET_UTIL})

file(GLOB SOURCES "src/*.c")

find_library(LIBM m)
find_library(LIBEV ev)
find_library(LIBJSON json-c)
find_library(LIBPREDICT predict)
find_library(LIBPTHREAD pthread)

add_compile_options(-pedantic -Wall -Werror)

add_executable(${TARGET_UTIL} ${SOURCES})
target_link_libraries(${TARGET_UTIL} ${LIBM} ${LIBEV} ${LIBJSON} ${LIBPREDICT} ${LIBPTHREAD})
install(TARGETS ${TARGET_UTIL} DESTINATION sbin)
install(CODE "execute_process(COMMAND ${CMAKE_SOURCE_DIR}/post_install.sh)")
