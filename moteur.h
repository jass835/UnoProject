#ifndef MOTEUR_H
#define MOTEUR_H

#include <sys/socket.h>

#define TAILLE_PAQUET 72  // Nombre total d'éléments dans le paquet
#define TAILLE_MAIN 7

extern const char *Paquet[TAILLE_PAQUET];

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
void distribuer_cartes(struct Joueur *premier_joueur, const char *paquet[], int nombre_joueurs);
void envoyer_main_joueur(int socket_id, const char cartes[][3], int taille_main);


#endif
