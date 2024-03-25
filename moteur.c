// moteur.c
#include "moteur.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void ajouter_joueur(struct Joueur **premier_joueur, int socket_id, int *nombre_joueurs) {
    struct Joueur *nouveau_joueur = (struct Joueur *)malloc(sizeof(struct Joueur));
    if (nouveau_joueur == NULL) {
        error("Erreur lors de l'allocation de mémoire pour le joueur");
    }
    nouveau_joueur->socket_id = socket_id;
    sprintf(nouveau_joueur->nom_utilisateur, "Joueur %d", (*nombre_joueurs) + 1);
    nouveau_joueur->suivant = *premier_joueur;
    *premier_joueur = nouveau_joueur;
    (*nombre_joueurs)++;
    printf("Joueur ajouté : ID socket %d, Nom : %s\n", socket_id, nouveau_joueur->nom_utilisateur);
}

void error(const char *msg) {
    perror(msg);
    exit(1);
}
