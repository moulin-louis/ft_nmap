FROM debian:latest

RUN apt update && apt install -y nmap fish valgrind cmake build-essential clang curl git make

RUN mkdir -p /ft_nmap/src && mkdir -p /ft_nmap/inc
WORKDIR /ft_nmap
COPY ./CMakeLists.txt .
COPY ./inc/* ./inc
COPY ./src/* ./src

RUN cmake -S . -B build && cmake --build build

ENTRYPOINT ["./build/ft_nmap"]