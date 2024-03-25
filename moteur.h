#ifndef MOTEUR_H
#define MOTEUR_H

#include <sys/socket.h>

#define TAILLE_PAQUET 108

typedef struct Carte {
    char valeur[3]; // Modifier pour stocker directement "B1", "R2", etc.
} Carte;

struct Joueur {
    int socket_id;
    char nom_utilisateur[20];
    struct Joueur *suivant;
};

void ajouter_joueur(struct Joueur **premier_joueur, int socket_id, int *nombre_joueurs);
void initialiserPaquet(Carte paquet[], int taille);
void afficherPaquet(const Carte paquet[], int taille);
void error(const char *msg);

#endif
