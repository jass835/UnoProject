#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <getopt.h>

#define COMMAND_SIZE 7 // Taille fixe de la commande "/login "
#define BUFFER_SIZE 2048

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];
    char *ip_address = NULL;

    // Analyser les options de la ligne de commande
    int opt;
    while ((opt = getopt(argc, argv, "s:p:")) != -1)
    {
        switch (opt)
        {
        case 's':
            ip_address = optarg;
            break;
        case 'p':
            portno = atoi(optarg);
            break;
        default:
            fprintf(stderr, "Usage: %s -s IP [-p PORT]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    // Vérifier si l'adresse IP est fournie
    if (ip_address == NULL)
    {
        fprintf(stderr, "You must provide the server IP address using -s option.\n");
        exit(EXIT_FAILURE);
    }

    // Si le port n'est pas fourni, utiliser un port par défaut
    if (portno == 0)
    {
        portno = 5000; // Port par défaut
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("ERROR opening socket");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);

    if (inet_pton(AF_INET, ip_address, &serv_addr.sin_addr) <= 0)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR connecting");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        printf("Menu:\n");
        printf("1. enter your login\n");
        printf("2. see connected players\n");
        printf("3. see your hand\n");
        printf("4. play a card\n");
        printf("5. pick a card\n");
        printf("6. see the last card\n");
        printf("Enter your choice: ");

        int choice;
        scanf("%d", &choice);
        getchar(); // Consume newline character

        switch (choice)
        {
        case 1:
            printf("Enter your username: ");
            fgets(buffer + COMMAND_SIZE, BUFFER_SIZE - COMMAND_SIZE, stdin);
            // Supprimer le caractère de nouvelle ligne de la chaîne lue
            buffer[strcspn(buffer, "\n")] = '\0';
            // Concaténer la commande "/login " avant le nom d'utilisateur
            strncpy(buffer, "/login ", COMMAND_SIZE);
            break;

        case 2:
            strcpy(buffer, "/players");
            break;
        case 3:
            strcpy(buffer, "/hand");
            break;
        case 4:
            printf("Enter your card: ");
            fgets(buffer + COMMAND_SIZE, BUFFER_SIZE - COMMAND_SIZE, stdin);
            // Supprimer le caractère de nouvelle ligne de la chaîne lue
            buffer[strcspn(buffer, "\n")] = '\0';
            // Concaténer la commande "/play " avant la carte
            strncpy(buffer, "/play ", COMMAND_SIZE);
            strcat(buffer, buffer + COMMAND_SIZE);
            break;

        case 5:
            strcpy(buffer, "/pick");
            break;

        case 6:
            strcpy(buffer, "/heap");
            break;

        default:
            printf("Invalid choice.\n");
            continue;
        }

        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0)
        {
            perror("ERROR writing to socket");
            exit(EXIT_FAILURE);
        }

        memset(buffer, 0, BUFFER_SIZE);
        n = read(sockfd, buffer, BUFFER_SIZE - 1);
        if (n < 0)
        {
            perror("ERROR reading from socket");
            exit(EXIT_FAILURE);
        }

        printf("Server response: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}

