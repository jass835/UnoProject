#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];

    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);

    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(1);
    }

    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(1);
    }

    while (1) {
        printf("Menu:\n");
        printf("1. entrer votre login\n");
        printf("2. voir joueurs connecter\n");
        printf("3. voir votre main\n");
        printf("4. jouer une carte \n");
        printf("Entrer votre choix : ");

        int choice;
        scanf("%d", &choice);
        getchar(); // Consume newline character

        switch (choice) {
            case 1:
                printf("Enter your username: ");
                fgets(buffer, BUFFER_SIZE, stdin);
                break;
            case 2:
                strcpy(buffer, "/players");
                break;
            case 3:
                strcpy(buffer, "/hand");
                break;
            case 4:
                printf("Enter the card you want to play: ");
                fgets(buffer, BUFFER_SIZE, stdin);
                break;
            default:
                printf("Invalid choice.\n");
                continue;
        }

        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0) {
            perror("ERROR writing to socket");
            exit(1);
        }

        memset(buffer, 0, BUFFER_SIZE);
        n = read(sockfd, buffer, BUFFER_SIZE-1);
        if (n < 0) {
             perror("ERROR reading from socket");
             exit(1);
        }

        printf("Server response: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}
