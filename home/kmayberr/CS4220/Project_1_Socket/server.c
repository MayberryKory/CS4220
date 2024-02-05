#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT_ID 14250
#define BUF_SIZE 4096 // Adjusted buffer size for file reading

int main(int argc, char *argv[]) {
    char *ip = "127.0.0.1";
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    char buffer[BUF_SIZE];
    FILE *file;
    int read_size;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("[-] Socket error");
        exit(1);
    }
    printf("[+] TCP server socket created.\n");

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_ID);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("[-] Bind error");
        exit(1);
    }
    printf("[+] Bind to the port number: %d\n", PORT_ID);

    listen(server_socket, 5);
    printf("Listening...\n");

    while (1) {
        addr_size = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_size);
        printf("[+] Client Connected\n");

        // Open the file you wish to send
        file = fopen("example.txt", "rb");
        if (file == NULL) {
            perror("[-] Error in reading file.");
            exit(1);
        }

        // Read file contents and send
        while ((read_size = fread(buffer, 1, BUF_SIZE, file)) > 0) {
            send(client_socket, buffer, read_size, 0);
        }

        // Close the file
        fclose(file);

        // Optionally, notify the client that the file transfer is complete
        strcpy(buffer, "EOF");
        send(client_socket, buffer, strlen(buffer), 0);

        close(client_socket);
        printf("[+] Client disconnected.\n\n");
    }

    return 0;
}
