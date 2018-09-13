#!/bin/bash
rm config.o
rm db.o
rm rest_server.o
rm json.o
rm task.o
rm dictionary.o
rm parce.o
rm cloud.o

gcc -c config.c
gcc -c db.c
gcc -c rest_server.c
gcc -c json.c
gcc -c task.c
gcc -c dictionary.c
gcc -c parce.c
gcc -c cloud.c

gcc core.c -lpthread -lmicrohttpd dictionary.o cloud.o parce.o config.o task.o json.o rest_server.o db.o -L/usr/lib64/mysql/ -lmysqlclient -o core
