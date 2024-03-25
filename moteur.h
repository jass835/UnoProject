// moteur.h
#ifndef MOTEUR_H
#define MOTEUR_H

#include <sys/socket.h>

struct Joueur {
    int socket_id;
    char nom_utilisateur[20];
    struct Joueur *suivant;
};

void ajouter_joueur(struct Joueur **premier_joueur, int socket_id, int *nombre_joueurs);
void error(const char *msg);

#endif

