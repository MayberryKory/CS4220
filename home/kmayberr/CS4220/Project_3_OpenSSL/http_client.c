/***********************************************************************
 * http_client.c
 * 
 * A simple HTTPS client implementation using OpenSSL to demonstrate secure
 * communication over a network. This client connects to a specified server,
 * performs an SSL handshake, sends an HTTP GET request, and prints the server's
 * response.
 *
 * Authors: Kory Mayberry, Ashley Judson, Nathan Peckham
 * 
 * University of Colorado Springs
 * Course: CS 4220 Networks Spring 2024
 * Instructor: Dr. Serena Sullivan
 * 
 *
 * Notes:
 * - This program is part of an educational project to understand SSL/TLS operations.
 * - Ensure OpenSSL is correctly installed and configured on the system where this is run.
 * - This client is configured to connect to localhost on port 4433.
 ***********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define PORT 4433 // Define the server port the client will connect to
#define SERVER "127.0.0.1"  // IP address of the server

// Initialize OpenSSL by loading error strings and algorithms
void init_openssl() {
    SSL_load_error_strings(); // Load SSL error strings
    OpenSSL_add_ssl_algorithms(); // Register available cryptographic algorithms
    printf("OpenSSL initialized.\n");
}

// Clean up and free all allocated OpenSSL resources
void cleanup_openssl() {
    EVP_cleanup(); // Clean up cryptographic functions
    printf("OpenSSL cleaned up.\n");
}

// Create a new SSL context for the client
SSL_CTX *create_context() {
    const SSL_METHOD *method; // Pointer to data structure describing the connection method
    SSL_CTX *ctx; // Pointer to the SSL context

    method = SSLv23_client_method(); // Use SSL/TLS client method that is compatible with various SSL/TLS versions
    ctx = SSL_CTX_new(method); // Create a new context using the method
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr); // Print detailed SSL error messages
        exit(EXIT_FAILURE);
    }

    // Set the cipher list to AES 256-bit cipher suites
    const char *cipher_list = "ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384";
    if (!SSL_CTX_set_cipher_list(ctx, cipher_list)) {
        fprintf(stderr, "Failed to set cipher list. Ensure the cipher suite is available in OpenSSL.\n");
        exit(EXIT_FAILURE);
    }

    printf("SSL context created and configured with AES 256-bit encryption.\n");
    return ctx;
}

// Open a TCP connection to the specified hostname and port
int open_connection(const char *hostname, int port) {
    int sockfd; // Socket file descriptor
    struct sockaddr_in addr; // Internet socket address structure

    sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create a TCP socket
    if (sockfd < 0) {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr)); // Clear structure
    addr.sin_family = AF_INET; // Set address family to Internet
    addr.sin_port = htons(port); // Set port number, using network byte order
    addr.sin_addr.s_addr = inet_addr(hostname); // Set IP address in the address structure

    printf("Connecting to %s on port %d...\n", hostname, port);
    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { // Connect to the server
        perror("Unable to connect");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server.\n");
    return sockfd;
}

// Send an HTTP GET request and print the server's response
void perform_request(SSL *ssl) {
    const char *request = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    char buffer[1024] = {0};

    printf("Sending request...\n");
    if (SSL_write(ssl, request, strlen(request)) <= 0) { // Write the request to the SSL connection
        ERR_print_errors_fp(stderr); // Print SSL errors
        exit(EXIT_FAILURE);
    }

    printf("Request sent. Waiting for response...\n");
    if (SSL_read(ssl, buffer, sizeof(buffer) - 1) <= 0) { // Read the server's response
        ERR_print_errors_fp(stderr); // Print SSL errors
        exit(EXIT_FAILURE);
    }

    printf("Response received:\n%s\n", buffer); // Print the response
}

// Main function to setup SSL and perform the request
int main(int argc, char **argv) {
    int server_fd; // Server socket file descriptor
    SSL_CTX *ctx; // SSL context
    SSL *ssl; // SSL connection

    init_openssl(); // Initialize OpenSSL
    ctx = create_context(); // Create SSL context
    server_fd = open_connection(SERVER, PORT); // Open connection to the server
    ssl = SSL_new(ctx); // Create a new SSL connection state
    SSL_set_fd(ssl, server_fd); // Associate the connection with the file descriptor

    printf("Starting SSL handshake...\n");
    if (SSL_connect(ssl) != 1) { // Perform the SSL handshake
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(server_fd);
        SSL_CTX_free(ctx);
        cleanup_openssl();
        exit(EXIT_FAILURE);
    }

    printf("SSL handshake completed.\n");
    perform_request(ssl); // Perform the request

    SSL_free(ssl); // Free the SSL structure
    close(server_fd); // Close the socket
    SSL_CTX_free(ctx); // Free the SSL context
    cleanup_openssl(); // Clean up OpenSSL

    return 0;
}
