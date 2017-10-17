#!/bin/bash
gcc server.c segment.c util.c -o recvfile -w
gcc client.c segment.c util.c -o sendfile -w