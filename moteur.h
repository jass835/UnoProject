#ifndef MOTEUR_H
#define MOTEUR_H

#include <sys/socket.h>
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

#define TAILLE_PAQUET 72  // Nombre total d'éléments dans le paquet
#define TAILLE_MAIN 7
#define MAX_USERNAME_LENGTH 16
#define MIN_USERNAME_LENGTH 3

extern const char *Paquet[TAILLE_PAQUET];
extern struct Joueur *joueur_autorise;


struct Joueur {
    int socket_id;
    char nom_utilisateur[20];
    char cartes[TAILLE_MAIN][3];
    struct Joueur *suivant;
};

void ajouter_joueur(struct Joueur **premier_joueur, int socket_id, int *nombre_joueurs);
void supprimer_joueur(struct Joueur **premier_joueur, int socket_id, int *nombre_joueurs);
void error(const char *msg);
void affichePaquet(const char *paquet[], int taille);
void melanger_paquet(const char *paquet[], int taille);
void distribuer_cartes(struct Joueur *premier_joueur, const char *paquet[], int nombre_joueurs, bool melanger);
void envoyer_main_joueur(int socket_id, const char cartes[][3], int taille_main);
void demarrer_partie(struct Joueur *premier_joueur);
void process_login_command(struct Joueur *joueur, const char *username);

extern bool partie_en_cours;
extern struct Joueur *premier_joueur;
extern int nombre_joueurs;




#endif
