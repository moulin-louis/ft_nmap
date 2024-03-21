cmake_minimum_required(VERSION 3.22)
project(ft_nmap C)

set(CMAKE_C_STANDARD 11)

add_compile_options(-Wall -Werror -Wextra -g)
include_directories(./inc)
add_link_options(-lpcap -lpthread)

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
