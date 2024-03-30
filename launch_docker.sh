#!/bin/zsh

docker compose up --build -d
docker exec -it ft_nmap bash
