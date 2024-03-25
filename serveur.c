// serveur.c
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

#define PORT 5000
#define MAX_CLIENTS 30
#define BUFFER_SIZE 1024

struct Joueur *premier_joueur = NULL;
int nombre_joueurs = 0;

int main() {
    int master_socket, addrlen, new_socket, activity, valread;
    int max_sd;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE];

    fd_set readfds;

    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        error("socket a échoué");
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        error("liaison a échoué");
    }
    printf("Écouteur sur le port %d \n", PORT);

    if (listen(master_socket, 3) < 0) {
        error("écoute");
    }

    addrlen = sizeof(address);
    puts("En attente de connexions ...");

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
            printf("erreur de sélection");
        }

        if (FD_ISSET(master_socket, &readfds)) {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
                error("accepter");
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
                    // Supprimer le joueur de la liste des joueurs
                    // (Non implémenté dans cet exemple)
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
