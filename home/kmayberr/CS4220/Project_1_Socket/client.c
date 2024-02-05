#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT_ID 14250
#define BUF_SIZE 4096 // Use a buffer size that matches your needs

int main(int argc, char *argv[]) {
    char *ip = "127.0.0.1";
    int sock;
    struct sockaddr_in addr;
    char buffer[BUF_SIZE];
    int bytes_received;

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("[-]Socket error");
        exit(1);
    }
    printf("[+]TCP client socket created.\n");

    // Configure server address
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_ID);
    addr.sin_addr.s_addr = inet_addr(ip);

    // Connect to server
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("[-]Connect error");
        exit(1);
    }
    printf("Connected to the server\n");

    // Initially clear the buffer
    memset(buffer, 0, BUF_SIZE);

    // Loop to receive file contents
    printf("Receiving file...\n");
    while ((bytes_received = recv(sock, buffer, BUF_SIZE, 0)) > 0) {
        // Print the received buffer
        printf("%.*s", bytes_received, buffer);
        memset(buffer, 0, BUF_SIZE); // Clear buffer after printing

        // Check for a special end-of-file marker if applicable
        if (strcmp(buffer, "EOF") == 0) {
            break;
        }
    }
    printf("\nFile received successfully.\n");

    // Close the socket
    close(sock);
    printf("Disconnected from the server\n");

    return 0;
}
