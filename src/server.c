#include "../include/server.h"

#include <netinet/in.h>
#include <sys/socket.h>



int createServer(int port)
{
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0)
    {
        perror("Erreur lors de la création du serveur");
        return -1;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Erreur lors de la liaison du serveur");
        return -1;
    }

    if (listen(server, 3) < 0)
    {
        perror("Erreur lors de l'écoute du serveur");
        return -1;
    }

    return server;
}

int acceptClient(int server)
{
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int client = accept(server, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    if (client < 0)
    {
        perror("Erreur lors de l'acceptation du client");
        return -1;
    }

    return client;
}

int closeServer(int server)
{
    return shutdown(server, SHUT_RDWR);
}

int recieveData(int client, Data data, int size)
{
    return recv(client, data.buffer, size, 0);
}

int sendData(int client, Data data, int size)
{
    return send(client, data.buffer, size, 0);
}

int interpretData(Data buffer, int size)
{
    switch (buffer.data.type)
    {
        case 'c': // Create ball
            printf("Création d'une balle\n");
            break;
    }
}

