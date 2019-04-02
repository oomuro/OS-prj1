cmake_minimum_required(VERSION 3.14)
project(ku_cfs C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR})

add_executable(ku_app ku_app.c)
add_executable(ku_cfs ku_cfs.c ku_cfs.h)

install(TARGETS ku_app ku_cfs
        CONFIGURATIONS Debug
        RUNTIME DESTINATION bin/Debug)
install(TARGETS ku_app ku_cfs
        CONFIGURATIONS Release
        RUNTIME DESTINATION bin/Release)
