cmake_minimum_required(VERSION 3.22)
project(ft_nmap C)

set(CMAKE_C_STANDARD 23)

add_compile_options(-Wall -Werror -Wextra -gdwarf-3)
include_directories(./inc)


add_executable(ft_nmap
        src/ft_nmap.c
        src/syn_scan.c
        src/parser.c
        src/worker.c
        src/scan_types.c
        inc/ft_nmap.h
        src/tcp_scan.c
        src/utils.c
        src/packet.c
)

target_link_libraries(ft_nmap pcap pthread)