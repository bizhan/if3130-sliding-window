#!/bin/bash

all: server client

server:
	gcc src/server.c src/segment.c src/util.c -o recvfile -w

client:
	gcc src/client.c src/segment.c src/util.c -o sendfile -w

testsend:
	./sendfile msg.txt 5 256 127.0.0.1 8888

testrecv:
	./recvfile out.txt 5 256 8888