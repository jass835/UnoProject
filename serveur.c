#include "moteur.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <getopt.h>

#define DEFAULT_PORT 5000
#define DEFAULT_PLAYERS_MINI 3
#define PORT 5000
#define MAX_CLIENTS 30
#define BUFFER_SIZE 1024
struct Joueur *premier_joueur = NULL;
int nombre_joueurs = 0;

int main(int argc, char *argv[]) {
    int opt;
    int port = DEFAULT_PORT;
    int players_mini = DEFAULT_PLAYERS_MINI;

    while ((opt = getopt(argc, argv, "p:m:")) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'm':
                players_mini = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-p PORT] [-m PLAYERS_MINI]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    int master_socket, addrlen, new_socket, activity, valread;
    int max_sd;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE];

    fd_set readfds;

    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket a échoué");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("liaison a échoué");
        exit(EXIT_FAILURE);
    }
    printf("Écouteur sur le port %d \n", port);

    if (listen(master_socket, players_mini) < 0) {
        perror("écoute");
        exit(EXIT_FAILURE);
    }

    addrlen = sizeof(address);
    puts("En attente de connexions ...");

    affichePaquet(Paquet, TAILLE_PAQUET);

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        struct Joueur *joueur_actuel = premier_joueur;
        while (joueur_actuel != NULL) {
            FD_SET(joueur_actuel->socket_id, &readfds);
            if (joueur_actuel->socket_id > max_sd) {
                max_sd = joueur_actuel->socket_id;
            }
            joueur_actuel = joueur_actuel->suivant;
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            perror("erreur de sélection");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(master_socket, &readfds)) {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
                perror("accepter");
                exit(EXIT_FAILURE);
            }

            printf("Nouvelle connexion, le socket fd est %d, l'ip est: %s, le port: %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            ajouter_joueur(&premier_joueur, new_socket, &nombre_joueurs);
        }

        joueur_actuel = premier_joueur;
        while (joueur_actuel != NULL) {
            if (FD_ISSET(joueur_actuel->socket_id, &readfds)) {
                if ((valread = read(joueur_actuel->socket_id, buffer, BUFFER_SIZE)) == 0) {
                    printf("Déconnexion de Joueur : %s\n", joueur_actuel->nom_utilisateur);
                    close(joueur_actuel->socket_id);
                    supprimer_joueur(&premier_joueur, joueur_actuel->socket_id, &nombre_joueurs);
                } else {
                    // Traitement du message reçu
                    buffer[valread] = '\0';
                    printf("Message du joueur %s: %s\n", joueur_actuel->nom_utilisateur, buffer);
                }
            }
            joueur_actuel = joueur_actuel->suivant;
        }
    }

    return 0;
}
