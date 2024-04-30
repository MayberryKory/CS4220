/***********************************************************************
 * http_server.c
 * 
 * A simple HTTPS server implementation using OpenSSL to demonstrate secure
 * communication over a network. This server listens on port 4433, accepts
 * client connections, performs an SSL handshake, and responds to HTTP GET
 * requests with a simple text message.
 *
 * Author(s): Kory Mayberry, Ashley Judson, Nathan Peckham
 * 
 * University of Colorado, Colorado Springs
 * Course: CS 4220 Networks
 * Instructor: Dr. Serena Sulllivan
 * 
 *
 * Notes:
 * - This program is part of an educational project to understand SSL/TLS operations.
 * - It is designed to handle simple HTTP GET requests and respond with a fixed message.
 * - This server is configured to listen on localhost on port 4433.
 ***********************************************************************/


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define PORT 4433  // Define the port number on which the server will listen

// Initialize OpenSSL libraries and load error strings
void init_openssl() {
    SSL_load_error_strings();   // Load human-readable error messages for SSL
    OpenSSL_add_ssl_algorithms();  // Register the available SSL algorithms
    printf("OpenSSL initialization complete.\n");
}

// Clean up and free OpenSSL resources
void cleanup_openssl() {
    EVP_cleanup();  // Remove all digests and ciphers
    printf("OpenSSL cleanup complete.\n");
}

// Create and set up an SSL context
SSL_CTX *create_context() {
    const SSL_METHOD *method;  // Pointer to a method structure for version-specific SSL methods
    SSL_CTX *ctx;  // Pointer to an SSL context structure

    method = SSLv23_server_method();  // Use the SSL/TLS server method for SSLv2, SSLv3, and TLSv1

    ctx = SSL_CTX_new(method);  // Create a new SSL context with the specified method
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);  // Print the error stack
        exit(EXIT_FAILURE);
    }

    printf("SSL context created successfully.\n");
    return ctx;
}

// Configure the SSL context with the server's certificate, private key, and AES 256-bit ciphers
void configure_context(SSL_CTX *ctx) {
    SSL_CTX_set_ecdh_auto(ctx, 1);  // Use ECDH automatically for key exchange

    // Set the certificate file for the SSL context
    if (SSL_CTX_use_certificate_file(ctx, "certs/server.crt", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Set the private key file for the SSL context
    if (SSL_CTX_use_PrivateKey_file(ctx, "certs/server.key", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Specify AES 256-bit cipher suites
    const char *cipher_list = "ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384";
    if (!SSL_CTX_set_cipher_list(ctx, cipher_list)) {
        fprintf(stderr, "Failed to set cipher list. Ensure the cipher suite is available in OpenSSL.\n");
        exit(EXIT_FAILURE);
    }

    printf("SSL context configured with certificate, private key, and AES 256-bit encryption.\n");
}

// Main function to set up the server socket and handle incoming connections
int main(int argc, char **argv) {
    int sockfd, new_sockfd;  // Socket file descriptors
    SSL_CTX *ctx;
    struct sockaddr_in addr;  // Socket address structure for IPv4
    socklen_t len = sizeof(addr);  // Length of the address
    SSL *ssl;

    // Initialize and configure OpenSSL
    init_openssl();
    ctx = create_context();
    configure_context(ctx);

    // Create a new socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    // Set up the socket address structure
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind the socket to the address and port number
    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Unable to bind");
        exit(EXIT_FAILURE);
    }

    // Listen on the socket for incoming connections
    if (listen(sockfd, 1) < 0) {
        perror("Unable to listen");
        exit(EXIT_FAILURE);
    }

    printf("Server is up and listening on port %d.\n", PORT);

    // Accept incoming connections in a loop
    while (1) {
        new_sockfd = accept(sockfd, (struct sockaddr*)&addr, &len);
        if (new_sockfd < 0) {
            perror("Unable to accept");
            continue;
        }

        printf("Connection accepted from %s:%d.\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

        // Create a new SSL structure for the connection
        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, new_sockfd);

        // Perform the SSL handshake
        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
            printf("SSL handshake failed.\n");
        } else {
            printf("SSL handshake succeeded.\n");
            SSL_write(ssl, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nOpenSSL is fun!", strlen("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nOpenSSL is fun!"));
            printf("Response sent to client.\n");
        }

        // Shutdown the SSL connection, free the SSL structure, and close the socket
        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(new_sockfd);
        printf("Connection closed.\n");
    }

    // Cleanup operations
    close(sockfd);
    SSL_CTX_free(ctx);
    cleanup_openssl();

    return 0;
}
