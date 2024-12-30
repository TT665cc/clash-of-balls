#ifndef SERVER_H

#define SERVER_H

#include "../include/utils.h"
#include "../include/physics.h"

#define BUFFER_SIZE 128

typedef union Data {
    char buffer[BUFFER_SIZE];
    struct {
        char type;
        union {
            Object object;
        };
    } data;
} Data;


int createServer(int port);

int acceptClient(int server);

int closeServer(int server);


#endif

