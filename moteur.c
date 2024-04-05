#include "moteur.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "moteur.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#define BUFFER_SIZE 1024

bool partie_en_cours = false;
struct Joueur *premier_joueur = NULL;
int nombre_joueurs = 0;
struct Joueur *joueur_autorise = NULL;

const char *Paquet[] = {
    "Y0", "Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7", "Y8", "Y9", "Y+", "Y+", "Y%", "Y%", "Y~", "Y~",
    "B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8", "B9", "B+", "B+", "B%", "B%", "B~", "B~",
    "G0", "G1", "G2", "G3", "G4", "G5", "G6", "G7", "G8", "G9", "G+", "G+", "G%", "G%", "G~", "G~",
    "R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8", "R9", "R+", "R+", "R%", "R%", "R~", "R~",
    "K+", "K+", "KJ", "KJ", "K+", "K+", "KJ", "KJ"};

void ajouter_joueur(struct Joueur **premier_joueur, int socket_id, int *nombre_joueurs)
{
    struct Joueur *nouveau_joueur = (struct Joueur *)malloc(sizeof(struct Joueur));
    if (nouveau_joueur == NULL)
    {
        error("Erreur lors de l'allocation de mémoire pour le joueur");
    }
    nouveau_joueur->socket_id = socket_id;
    sprintf(nouveau_joueur->nom_utilisateur, "Joueur %d", (*nombre_joueurs) + 1);
    nouveau_joueur->suivant = *premier_joueur;
    *premier_joueur = nouveau_joueur;
    (*nombre_joueurs)++;
    printf("Joueur ajouté : ID socket %d, Nom : %s\n", socket_id, nouveau_joueur->nom_utilisateur);

    // Si c'est le premier joueur connecté, définissez-le comme joueur autorisé à lancer la partie
    if (*nombre_joueurs == 1)
    {
        joueur_autorise = nouveau_joueur;
    }
}

void supprimer_joueur(struct Joueur **premier_joueur, int socket_id, int *nombre_joueurs)
{
    struct Joueur *temp = *premier_joueur;
    struct Joueur *prev = NULL;

    // Si le joueur à supprimer est en tête de liste
    if (temp != NULL && temp->socket_id == socket_id)
    {
        *premier_joueur = temp->suivant;
        free(temp);
        (*nombre_joueurs)--;
        return;
    }

    // Chercher le joueur à supprimer dans le reste de la liste
    while (temp != NULL && temp->socket_id != socket_id)
    {
        prev = temp;
        temp = temp->suivant;
    }

    // Si le joueur n'a pas été trouvé
    if (temp == NULL)
    {
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

void affichePaquet(const char *paquet[], int taille)
{
    printf("Paquet de cartes :\n");
    for (int i = 0; i < taille; ++i)
    {
        printf("%s ", paquet[i]);
        if ((i + 1) % 15 == 0) // Nouvelle ligne tous les 15 éléments
            printf("\n");
    }
    printf("\n");
}

void melanger_paquet(const char *paquet[], int taille)
{
    srand(time(NULL)); // Initialisation du générateur de nombres aléatoires
    for (int i = 0; i < taille - 1; ++i)
    {
        int j = i + rand() % (taille - i); // Génère un indice aléatoire
        // Échange de cartes entre l'élément courant et l'élément d'indice aléatoire
        const char *temp = paquet[i];
        paquet[i] = paquet[j];
        paquet[j] = temp;
    }
}

void distribuer_cartes_et_mettre_a_jour_paquet(struct Joueur *joueur, const char *paquet[], bool melanger)
{
    if (melanger)
    {
        melanger_paquet(paquet, TAILLE_PAQUET);
    }

    int index_paquet = 0;
    int cartes_distribuees = 0;

    // Distribuer les 7 premières cartes au joueur spécifié
    while (cartes_distribuees < 7 && index_paquet < TAILLE_PAQUET)
    {
        // Trouver la première carte disponible dans le paquet
        while (index_paquet < TAILLE_PAQUET && paquet[index_paquet] == NULL)
        {
            index_paquet++;
        }

        // S'assurer que le paquet n'est pas épuisé
        if (index_paquet < TAILLE_PAQUET)
        {
            // Copier la carte dans la main du joueur
            strcpy(joueur->cartes[cartes_distribuees], paquet[index_paquet]);
            // Passer à la carte suivante
            cartes_distribuees++;
            // Marquer la carte comme distribuée en supprimant la référence du paquet
            paquet[index_paquet] = NULL;
        }
        // Passer à la carte suivante dans le paquet
        index_paquet++;
    }

    // Mettre à jour le paquet en supprimant les cartes distribuées
    int index_destination = 0;
    for (int index_source = 0; index_source < TAILLE_PAQUET; index_source++)
    {
        if (paquet[index_source] != NULL)
        {
            // Déplacer la carte non distribuée à sa nouvelle position dans le paquet
            paquet[index_destination++] = paquet[index_source];
        }
    }

    // Marquer la fin du paquet avec NULL pour indiquer les cartes non utilisées
    for (int i = index_destination; i < TAILLE_PAQUET; i++)
    {
        paquet[i] = NULL;
    }
}

void demarrer_partie(struct Joueur *premier_joueur)
{
    printf("Démarrage de la partie.\n");
    // Distribuer les cartes aux joueurs en mélangeant le paquet

    // Envoyer les cartes à chaque joueur
    struct Joueur *joueur_actuel = premier_joueur;
    while (joueur_actuel != NULL)
    {
        envoyer_main_joueur(joueur_actuel->socket_id, joueur_actuel->cartes, TAILLE_MAIN);
        joueur_actuel = joueur_actuel->suivant;
    }
    // Marquer la partie comme en cours
    partie_en_cours = true;
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}
void envoyer_main_joueur(int socket_id, const char cartes[][3], int taille_main)
{
    char message[BUFFER_SIZE] = "";
    for (int i = 0; i < taille_main; i++)
    {
        strcat(message, cartes[i]);
        if (i < taille_main - 1 && cartes[i + 1][0] != '\0') // Ajouter une virgule si ce n'est pas la dernière carte et si la prochaine carte existe
        {
            strcat(message, ",");
        }
    }

    // Supprimer la virgule finale si elle existe
    if (taille_main > 0 && message[strlen(message) - 1] == ',')
    {
        message[strlen(message) - 1] = '\0';
    }

    if (write(socket_id, message, strlen(message)) < 0)
    {
        perror("Erreur lors de l'envoi des cartes du joueur");
    }

    // Ajouter un saut de ligne à la fin de l'envoi des cartes
    char saut_ligne[] = "\n";
    write(socket_id, saut_ligne, strlen(saut_ligne));
}

// Fonction pour traiter la commande /login
void process_login_command(struct Joueur *joueur, const char *username)
{
    // Vérifier la longueur du nom d'utilisateur
    size_t username_length = strcspn(username, "\r\n");
    if (username_length < MIN_USERNAME_LENGTH || username_length > MAX_USERNAME_LENGTH)
    {
        // Envoyer un message d'erreur au joueur
        char error_message[] = "15 Invalid login\n";
        write(joueur->socket_id, error_message, strlen(error_message));
        return;
    }

    // Vérifier les caractères interdits dans le nom d'utilisateur
    if (strchr(username, ',') != NULL || strchr(username, '#') != NULL)
    {
        // Envoyer un message d'erreur au joueur
        char error_message[] = "15 Invalid login\n";
        write(joueur->socket_id, error_message, strlen(error_message));
        return;
    }

    else
    {

        // Mettre à jour le nom d'utilisateur du joueur
        strncpy(joueur->nom_utilisateur, username, username_length);
        joueur->nom_utilisateur[username_length] = '\0';

        // Envoyer un message de confirmation au joueur
        char success_message[] = "00 OK\n";
        write(joueur->socket_id, success_message, strlen(success_message));
    }
}

bool is_valid_command(const char *command)
{
    if (strncmp(command, "/login", 6) == 0 ||
        strncmp(command, "/heap", 5) == 0 ||
        strncmp(command, "/players", 8) == 0 ||
        strncmp(command, "/hand", 5) == 0 ||
        strncmp(command, "/pick", 5) == 0)
    {
        return true;
    }
    // Vérifier si la commande commence par "/play " (avec un espace après "/play")
    else if (strncmp(command, "/play ", 6) == 0 && strlen(command) >= 8)
    {
        return true;
    }
    return false;
}
void ajouter_carte_a_main(struct Joueur *joueur, const char *paquet[])
{
    // Chercher la première carte disponible dans le paquet
    int index_paquet = 0;
    while (index_paquet < TAILLE_PAQUET && paquet[index_paquet] == NULL)
    {
        index_paquet++;
    }

    // Vérifier si le paquet n'est pas épuisé
    if (index_paquet < TAILLE_PAQUET)
    {
        // Chercher la première place disponible dans la main du joueur
        int index_main = 0;
        while (index_main < TAILLE_MAIN && joueur->cartes[index_main][0] != '\0')
        {
            index_main++;
        }

        // Vérifier si la main du joueur n'est pas déjà pleine
        if (index_main < TAILLE_MAIN)
        {
            // Ajouter la carte du paquet à la main du joueur
            strcpy(joueur->cartes[index_main], paquet[index_paquet]);
            // Marquer la carte comme distribuée en supprimant la référence du paquet
            paquet[index_paquet] = NULL;
        }
        else
        {
            printf("La main du joueur est pleine, impossible d'ajouter une nouvelle carte.\n");
        }
    }
    else
    {
        printf("Le paquet est épuisé, aucune carte à distribuer.\n");
    }
}

// Fonction pour remettre une carte au paquet initial
void remettre_carte_au_paquet(const char *carte, const char *paquet[], int taille_paquet)
{
    for (int i = 0; i < taille_paquet; ++i)
    {
        if (paquet[i] == NULL)
        {
            paquet[i] = carte;
            break;
        }
    }
}
