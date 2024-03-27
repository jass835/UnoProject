#include "moteur.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "moteur.h"
#include <stdio.h>
#include <time.h>

const char *Paquet[] = {
    "J0", "J1", "J2", "J3", "J4", "J5", "J6", "J7", "J8", "J9", "J+", "J+", "J%", "J%", "J~","J~",
    "B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8", "B9", "B+", "B+", "B%", "B%", "B~","B~",
    "V0", "V1", "V2", "V3", "V4", "V5", "V6", "V7", "V8", "V9", "V+", "V+", "V%", "V%", "V~","V~",
    "R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8", "R9", "R+", "R+", "R%", "R%", "R~","R~",
      "K+", "K+", "KJ", "KJ",  "K+", "K+", "KJ", "KJ"
};


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

void supprimer_joueur(struct Joueur **premier_joueur, int socket_id, int *nombre_joueurs) {
    struct Joueur *temp = *premier_joueur;
    struct Joueur *prev = NULL;

    // Si le joueur à supprimer est en tête de liste
    if (temp != NULL && temp->socket_id == socket_id) {
        *premier_joueur = temp->suivant;
        free(temp);
        (*nombre_joueurs)--;
        return;
    }

    // Chercher le joueur à supprimer dans le reste de la liste
    while (temp != NULL && temp->socket_id != socket_id) {
        prev = temp;
        temp = temp->suivant;
    }

    // Si le joueur n'a pas été trouvé
    if (temp == NULL) {
        printf("Le joueur avec l'ID socket %d n'existe pas dans la liste.\n", socket_id);
        return;
    }

    // Lien du joueur précédent avec le suivant du joueur à supprimer
    prev->suivant = temp->suivant;

    // Libération de la mémoire du joueur à supprimer
    free(temp);

    (*nombre_joueurs)--;
    printf("Joueur avec l'ID socket %d supprimé de la liste.\n", socket_id);
}

void affichePaquet(const char *paquet[], int taille) {
    printf("Paquet de cartes :\n");
    for (int i = 0; i < taille; ++i) {
        printf("%s ", paquet[i]);
        if ((i + 1) % 15 == 0) // Nouvelle ligne tous les 15 éléments
            printf("\n");
    }
    printf("\n");
}

void melanger_paquet(const char *paquet[], int taille) {
    srand(time(NULL)); // Initialisation du générateur de nombres aléatoires
    for (int i = 0; i < taille - 1; ++i) {
        int j = i + rand() % (taille - i); // Génère un indice aléatoire
        // Échange de cartes entre l'élément courant et l'élément d'indice aléatoire
        const char *temp = paquet[i];
        paquet[i] = paquet[j];
        paquet[j] = temp;
    }
}

void distribuer_cartes(struct Joueur *premier_joueur, const char *paquet[], int nombre_joueurs) {
    struct Joueur *joueur_actuel = premier_joueur;
    int cartes_distribuees = 0;
    int index_paquet = 0;

    while (joueur_actuel != NULL && cartes_distribuees < nombre_joueurs * 4) {
        for (int i = 0; i < 4; ++i) {
            if (paquet[index_paquet] != NULL) {
                strcpy(joueur_actuel->cartes[i], paquet[index_paquet]);
                index_paquet++;
                cartes_distribuees++;
            }
        }
        joueur_actuel = joueur_actuel->suivant;
    }
}

void error(const char *msg) {
    perror(msg);
    exit(1);
}
