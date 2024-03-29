cmake_minimum_required(VERSION 3.22)
project(ft_nmap C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_COMPILER gcc)

add_compile_options(-Wall -Werror -Wextra -gdwarf-3 -I${CMAKE_CURRENT_SOURCE_DIR}/lib/libdata/inc)
include_directories(./inc)

add_custom_target(
        libdata ALL
        COMMAND if [ "${CMAKE_BUILD_TYPE}" = "debug" ]\; then make -C ${CMAKE_CURRENT_SOURCE_DIR}/lib/libdata debug\; else make -C ${CMAKE_CURRENT_SOURCE_DIR}/lib/libdata\; fi
)

add_executable(ft_nmap
        src/ft_nmap.c
        src/options.c
        src/packet.c
        src/parser.c
        src/scan_types.c
        src/t_host.c
        src/tcp_scan.c
        src/timing.c
        src/ultra_scan.c
        src/utils.c
        src/worker.c
        src/analysis.c
)

target_link_libraries(ft_nmap -lpcap -lpthread -lm -L${CMAKE_CURRENT_SOURCE_DIR}/lib/libdata -ldata)
add_dependencies(ft_nmap libdata)
