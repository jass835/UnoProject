#include "moteur.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void initialiserPaquet(Carte paquet[], int taille) {
    // Liste complète des cartes en chaînes de caractères
    const char *toutesLesCartes[] = {
        "B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8", "B9", "B%", "B%", "B+", "B+", "B~", "B~",
        "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8", "R9", "R%", "R%", "R+", "R+", "R~", "R~",
        "J1", "J2", "J3", "J4", "J5", "J6", "J7", "J8", "J9", "J%", "J%", "J+", "J+", "J~", "J~",
        "V1", "V2", "V3", "V4", "V5", "V6", "V7", "V8", "V9", "V%", "V%", "V+", "V+", "V~", "V~",
        "K+", "K+", "K+", "K+", "KJ", "KJ", "KJ", "KJ"
    };

    // Assurez-vous que la taille est correcte
    if (taille != sizeof(toutesLesCartes) / sizeof(toutesLesCartes[0])) {
        printf("Erreur: la taille du paquet ne correspond pas au nombre de cartes défini.\n");
        return;
    }

    // Initialisation du paquet avec les cartes définies
    for (int i = 0; i < taille; i++) {
        strncpy(paquet[i].valeur, toutesLesCartes[i], sizeof(paquet[i].valeur));
        paquet[i].valeur[sizeof(paquet[i].valeur) - 1] = '\0'; // Assurez la terminaison par un caractère nul
    }
}

void afficherPaquet(const Carte paquet[], int taille) {
    printf("Paquet de cartes Uno :\n");
    for (int i = 0; i < taille; i++) {
        printf("%s ", paquet[i].valeur);
    }
    printf("\n");
}
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