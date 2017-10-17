#include "util.h"

void die(char *s){
    perror(s);
    exit(1);
}

int char_to_int(char* s) {
    int n = 0;
    while (*s != 0)
        n = n * 10 + ((int) (*s++) - '0');
    return n;
}