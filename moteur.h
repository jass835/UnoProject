#ifndef MOTEUR_H
#define MOTEUR_H

#include <sys/socket.h>

#define TAILLE_PAQUET 72  // Nombre total d'éléments dans le paquet

extern const char *Paquet[TAILLE_PAQUET];

struct Joueur {
    int socket_id;
    char nom_utilisateur[20];
    struct Joueur *suivant;
};

void ajouter_joueur(struct Joueur **premier_joueur, int socket_id, int *nombre_joueurs);
void supprimer_joueur(struct Joueur **premier_joueur, int socket_id, int *nombre_joueurs);
void error(const char *msg);
void affichePaquet(const char *paquet[], int taille);

#endif
