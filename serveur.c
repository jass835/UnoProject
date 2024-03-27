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
#include <stdbool.h>

#define DEFAULT_PORT 5000
#define DEFAULT_PLAYERS_MINI 3
#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    int opt;
    int port = DEFAULT_PORT;
    int players_mini = DEFAULT_PLAYERS_MINI;

    while ((opt = getopt(argc, argv, "p:m:")) != -1)
    {
        switch (opt)
        {
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

    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket a échoué");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("liaison a échoué");
        exit(EXIT_FAILURE);
    }
    printf("Écouteur sur le port %d \n", port);

    if (listen(master_socket, players_mini) < 0)
    {
        perror("écoute");
        exit(EXIT_FAILURE);
    }

    addrlen = sizeof(address);
    puts("En attente de connexions ...");

    struct Joueur *joueur_autorise = NULL; // Déclaration du joueur autorisé

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        struct Joueur *joueur_actuel = premier_joueur;
        while (joueur_actuel != NULL)
        {
            FD_SET(joueur_actuel->socket_id, &readfds);
            if (joueur_actuel->socket_id > max_sd)
            {
                max_sd = joueur_actuel->socket_id;
            }
            joueur_actuel = joueur_actuel->suivant;
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR))
        {
            perror("erreur de sélection");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
            {
                perror("accepter");
                exit(EXIT_FAILURE);
            }

            printf("Nouvelle connexion, le socket fd est %d, l'ip est: %s, le port: %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            ajouter_joueur(&premier_joueur, new_socket, &nombre_joueurs);

            if (joueur_autorise == NULL)
            {
                joueur_autorise = premier_joueur; // Définir le premier joueur comme joueur autorisé
            }
        }

        joueur_actuel = premier_joueur;
        while (joueur_actuel != NULL)
        {
            if (FD_ISSET(joueur_actuel->socket_id, &readfds))
            {
                if ((valread = read(joueur_actuel->socket_id, buffer, BUFFER_SIZE)) == 0)
                {
                    printf("Déconnexion de Joueur : %s\n", joueur_actuel->nom_utilisateur);
                    close(joueur_actuel->socket_id);
                    supprimer_joueur(&premier_joueur, joueur_actuel->socket_id, &nombre_joueurs);
                }
                else
                {
                    if (joueur_actuel == joueur_autorise)
                    { // Vérifier si c'est le tour du joueur autorisé
                        // Traitement du message reçu
                        buffer[valread] = '\0';
                        printf("Message du joueur %s: %s\n", joueur_actuel->nom_utilisateur, buffer);

                        // Supprimer les caractères de nouvelle ligne de la fin de la chaîne
                        size_t length = strcspn(buffer, "\n");
                        buffer[length] = '\0';

                        // Comparer avec "/login"
                        if (strncmp(buffer, "/login", 6) == 0)
                        {
                            // Extraire le nom d'utilisateur de la commande
                            char *username = buffer + 7; // Ignorer "/login " dans le buffer
                            // Traiter la commande /login pour le joueur actuel
                            process_login_command(joueur_actuel, username);
                        }

                        // Comparer avec "/play"
                        if (strcmp(buffer, "/play") == 0 && nombre_joueurs >= players_mini && !partie_en_cours)
                        {
                            demarrer_partie(premier_joueur);
                        }

                        // Passer au joueur suivant comme joueur autorisé
                        joueur_autorise = joueur_autorise->suivant;
                        if (joueur_autorise == NULL)
                        {
                            joueur_autorise = premier_joueur; // Revenir au premier joueur si le dernier joueur a joué
                        }

                        if (strncmp(buffer, "/players", 8) == 0)
                        {
                            // Construire la liste des joueurs connectés
                            char player_list[BUFFER_SIZE];
                            memset(player_list, 0, BUFFER_SIZE);
                            strcat(player_list, "Liste des joueurs connectés :\n");
                            struct Joueur *joueur = premier_joueur;
                            while (joueur != NULL)
                            {
                                strcat(player_list, joueur->nom_utilisateur);
                                strcat(player_list, "\n");
                                joueur = joueur->suivant;
                            }
                            // Envoyer la liste des joueurs au joueur qui a demandé
                            write(joueur_actuel->socket_id, player_list, strlen(player_list));
                        }
                        if (strncmp(buffer, "/hand", 5) == 0)
                        {
                            // Appeler la fonction envoyer_main_joueur avec les informations appropriées
                            envoyer_main_joueur(joueur_actuel->socket_id, joueur_actuel->cartes, TAILLE_MAIN);
                        }
                    }
                    else
                    {
                        // Envoyer un message d'erreur au joueur
                        char error_message[] = "11 Not your turn\n";
                        write(joueur_actuel->socket_id, error_message, strlen(error_message));
                    }
                }
            }
            joueur_actuel = joueur_actuel->suivant;
        }
    }

    return 0;
}
