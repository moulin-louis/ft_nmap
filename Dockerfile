FROM debian:latest

RUN apt update && apt install -y nmap fish valgrind cmake build-essential clang curl git make libpcap-dev
RUN mkdir -p /ft_nmap/src && mkdir -p /ft_nmap/inc

WORKDIR /ft_nmap

RUN cmake -S . -B build && cmake --build build
ENTRYPOINT ["tail", "-f"]