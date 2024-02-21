#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>

#define PORT 5000
#define MAX_CLIENTS 30
#define BUFFER_SIZE 1024

int client_socket[MAX_CLIENTS];

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void init_client_socket_array() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_socket[i] = 0;
    }
}

int main(int argc, char *argv[]) {
    int master_socket, addrlen, new_socket, activity, i, valread, sd;
    int max_sd;
    struct sockaddr_in address;
    
    char buffer[BUFFER_SIZE];

    fd_set readfds;

    init_client_socket_array();

    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        error("socket a échoué");
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) {
        error("liaison a échoué");
    }
    printf("Écouteur sur le port %d \n", PORT);

    if (listen(master_socket, 3) < 0) {
        error("écoute");
    }

    addrlen = sizeof(address);
    puts("En attente de connexions ...");

    while(1) {
        FD_ZERO(&readfds);
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;
        
        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = client_socket[i];
            if(sd > 0) FD_SET(sd, &readfds);
            if(sd > max_sd) max_sd = sd;
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            printf("erreur de sélection");
        }

        if (FD_ISSET(master_socket, &readfds)) {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
                error("accepter");
            }

            printf("Nouvelle connexion, le socket fd est %d, l'ip est: %s, le port: %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            for (i = 0; i < MAX_CLIENTS; i++) {
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    printf("Ajout à la liste des sockets en tant que %d\n", i);
                    break;
                }
            }
        }

        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = client_socket[i];

            if (FD_ISSET(sd, &readfds)) {
                if ((valread = read(sd, buffer, BUFFER_SIZE)) == 0) {
                    getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
                    printf("Hôte déconnecté, ip %s, port %d \n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                    close(sd);
                    client_socket[i] = 0;
                } else {
                    // Traitement du message reçu
                    buffer[valread] = '\0';
                    printf("Message du client %d: %s\n", i, buffer);
                    // Ici, vous pouvez traiter le message comme vous le souhaitez, sans renvoyer de réponse
                }
            }
        }
    }

    return 0;
}
