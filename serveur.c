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
    bool play_attempted = false;          // Variable pour suivre si la commande /play a été tentée
    bool players_minimum_reached = false; // Variable pour suivre si le nombre minimum de joueurs a été atteint

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

            // Distribuer les cartes au nouveau joueur
            distribuer_cartes(premier_joueur, Paquet, true); // Distribue uniquement au nouveau joueur
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

                    // Vérifier si le joueur déconnecté était celui autorisé à jouer
                    if (joueur_actuel == joueur_autorise)
                    {
                        // Passer au joueur suivant comme joueur autorisé
                        joueur_autorise = joueur_autorise->suivant;
                        if (joueur_autorise == NULL)
                        {
                            joueur_autorise = premier_joueur; // Revenir au premier joueur si le dernier joueur a joué
                        }
                    }
                }

                else
                {

                    if (strncmp(buffer, "/", 1) != 0)
                    {
                        // Si le message ne commence pas par "/", envoyer "10 Bad Command" au joueur et passer au suivant
                        char error_message[] = "10 Bad Command\n";
                        write(joueur_actuel->socket_id, error_message, strlen(error_message));
                    }
                    // Vérifier si la commande est une commande client valide
                    else if (!is_valid_command(buffer))
                    {
                        char error_message[] = "99 Unknown Command\n";
                        write(joueur_actuel->socket_id, error_message, strlen(error_message));
                    }
                    // Comparer avec "/login"
                    else if (strncmp(buffer, "/login", 6) == 0)
                    {
                        // Extraire le nom d'utilisateur de la commande
                        char *username = buffer + 7; // Ignorer "/login " dans le buffer
                        // Traiter la commande /login pour le joueur actuel
                        process_login_command(joueur_actuel, username);
                    }
                    else if (strncmp(buffer, "/players", 8) == 0)
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

                        // Envoyer un message de confirmation au joueur
                        char success_message[] = "00 OK\n";
                        write(joueur->socket_id, success_message, strlen(success_message));
                    }

                    else if (strncmp(buffer, "/hand", 5) == 0)
                    {
                        // Appeler la fonction envoyer_main_joueur avec les informations appropriées
                        envoyer_main_joueur(joueur_actuel->socket_id, joueur_actuel->cartes, TAILLE_MAIN);
                    }

                    // Vérifier si le nombre minimum de joueurs est atteint
                    if (nombre_joueurs >= players_mini)
                    {
                        players_minimum_reached = true;
                    }
                    if (joueur_actuel == joueur_autorise)
                    { // Vérifier si c'est le tour du joueur autorisé
                        // Traitement du message reçu
                        buffer[valread] = '\0';
                        printf("Message du joueur %s: %s\n", joueur_actuel->nom_utilisateur, buffer);

                        // Supprimer les caractères de nouvelle ligne de la fin de la chaîne
                        size_t length = strcspn(buffer, "\n");
                        buffer[length] = '\0';

                        // Comparer avec "/play"
                        if (strncmp(buffer, "/play", 5) == 0)
                        {
                            // Si la commande /play est tentée et le nombre minimum de joueurs n'a pas été atteint
                            if (!players_minimum_reached && !play_attempted && nombre_joueurs < players_mini)
                            {
                                char error_message[50];
                                sprintf(error_message, "20 Waiting for at least %d more user(s)\n", players_mini - nombre_joueurs);
                                write(joueur_actuel->socket_id, error_message, strlen(error_message));
                            }

                            // Vérifier si le nombre de caractères est correct
                            else if (strlen(buffer) != 8)
                            {
                                char error_message[] = "10 Bad Command\n";
                                write(joueur_actuel->socket_id, error_message, strlen(error_message));
                            }

                            else // Si la condition n'est pas remplie ou ce n'est pas la première tentative, traiter normalement
                            {
                                // Extraire la carte du jeu spécifiée dans la commande
                                const char *carte_commande = buffer + 6; // Ignorer "/play " dans le buffer

                                // Trouver l'indice de la carte dans la main du joueur
                                int indice_carte = -1;
                                for (int i = 0; i < TAILLE_MAIN; i++)
                                {
                                    if (strcmp(joueur_actuel->cartes[i], carte_commande) == 0)
                                    {
                                        indice_carte = i;
                                        break;
                                    }
                                }

                                // Si la carte spécifiée est trouvée dans la main du joueur
                                if (indice_carte != -1)
                                {
                                    // Retirer la carte de la main du joueur
                                    for (int i = indice_carte; i < TAILLE_MAIN - 1; i++)
                                    {
                                        strcpy(joueur_actuel->cartes[i], joueur_actuel->cartes[i + 1]);
                                    }
                                    strcpy(joueur_actuel->cartes[TAILLE_MAIN - 1], "");
                                    // Marquer que la commande /play a été tentée
                                    play_attempted = true;

                                    // Envoyer un message de confirmation au joueur
                                    char success_message[] = "00 OK\n";
                                    write(joueur_actuel->socket_id, success_message, strlen(success_message));
                                }

                                /*else if (strcmp(buffer, "00 OK\n") == 0)
                                {
                                    // Passer au joueur suivant comme joueur autorisé
                                    joueur_autorise = joueur_autorise->suivant;
                                    if (joueur_autorise == NULL)
                                    {
                                        joueur_autorise = premier_joueur; // Revenir au premier joueur si le dernier joueur a joué
                                    }
                                }*/
                                else
                                {
                                    // Envoyer un message d'erreur si la carte spécifiée n'est pas dans la main du joueur
                                    char error_message[] = "13 Not your card\n";
                                    write(joueur_actuel->socket_id, error_message, strlen(error_message));
                                }
                            }

                            // Passer au joueur suivant comme joueur autorisé
                            joueur_autorise = joueur_autorise->suivant;
                            if (joueur_autorise == NULL)
                            {
                                joueur_autorise = premier_joueur; // Revenir au premier joueur si le dernier joueur a joué
                            }
                        }

                        // Vérifier si le nombre minimum de joueurs est atteint
                        if (nombre_joueurs >= players_mini)
                        {
                            players_minimum_reached = true;
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
