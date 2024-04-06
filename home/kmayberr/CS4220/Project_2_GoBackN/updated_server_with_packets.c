#include <stdio.h>      // Standard input/output definitions
#include <sys/socket.h> // Socket programming definitions
#include <arpa/inet.h>  // Definitions for internet operations
#include <stdlib.h>     // Standard library definitions
#include <string.h>     // String manipulation definitions
#include <unistd.h>     // POSIX operating system API
#include <errno.h>      // Defines macros for reporting and retrieving error conditions
#include <signal.h>     // Signal handling definitions
#include "packetStruct.c"  // Include the Go-Back-N packet structure definitions

// Define constants for the server port and chunk size
#define SERVER_PORT 12345       
#define CHUNK_SIZE  256         
#define MAX_PACKET_SIZE 1024    // Define a max packet size for buffer allocation



// Prototype for the alarm signal handler
void CatchAlarm(int ignored);


int main(int argc, char **argv) {
    printf("Server: Starting...\n");
    char buffer[MAX_PACKET_SIZE]; // Buffer for storing received data
    int sock;                      // Socket descriptor for the server
    struct sockaddr_in gbnServAddr, gbnClntAddr; // Structs for server and client address
    unsigned int cliAddrLen;      // Variable for storing the length of the client address
    double lossRate;              // Variable for user-specified packet loss rate

    // Prompt the user for the desired packet loss rate and read the input
    printf("Enter your desired packet loss rate (e.g., 0.5 for 50%%): ");
    scanf("%lf", &lossRate);

    // Create a UDP socket
printf("Server: Creating socket...\n");
    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
printf("Server: Creating socket...\n");
        perror("socket() failed");
        exit(EXIT_FAILURE); // Exit if the socket cannot be created
    }

    memset(&gbnServAddr, 0, sizeof(gbnServAddr)); // Initialize the server address structure
    gbnServAddr.sin_family = AF_INET;             // Internet address family
    gbnServAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Listen on all interfaces
    gbnServAddr.sin_port = htons(SERVER_PORT);    // Server port

    // Bind the socket to the local address and port
printf("Server: Binding to port...\n");
    if (bind(sock, (struct sockaddr *)&gbnServAddr, sizeof(gbnServAddr)) < 0) {
printf("Server: Binding to port...\n");
        perror("bind() failed");
printf("Server: Closing the connection...\n");
        close(sock); // Cleanup
        exit(EXIT_FAILURE);
    }

    // Main loop to handle incoming packets indefinitely
   while (1) {
    printf("Listening.....\n");
    struct packetStruct currPacket; // Packet struct to store incoming data
    cliAddrLen = sizeof(gbnClntAddr);

    // Block and wait for incoming data, directly into currPacket
    if (recvfrom(sock, &currPacket, sizeof(currPacket), 0,
                 (struct sockaddr *)&gbnClntAddr, &cliAddrLen) < 0) {
        perror("recvfrom() failed");
        continue; // In case of error, log and try to receive again
    }

    // Simulate packet loss based on the specified rate
    if (lossRate > ((double)rand() / RAND_MAX)) {
        printf("Packet with sequence number %d lost\n", currPacket.seq_no);
        continue; // Skip further processing for this packet
    }

    // Assuming type 1 is data and type 2 is an ack for simplification
    printf("Received packet: Seq No %d, Length %d, Data: %s\n", 
           currPacket.seq_no, currPacket.length, currPacket.data);

    if (currPacket.type == 1) { // Check if the packet is a data packet
        struct packetStruct ackPacket;
        memset(&ackPacket, 0, sizeof(ackPacket)); // Initialize the ackPacket to zero
        
        ackPacket.type = 2; // Set packet type to ACK
        ackPacket.seq_no = currPacket.seq_no; // Set the ACK packet's sequence number to match the received packet
        
        // Send the ACK packet back to the client
        if (sendto(sock, &ackPacket, sizeof(ackPacket), 0,
                   (struct sockaddr *)&gbnClntAddr, sizeof(gbnClntAddr)) < 0) {
            perror("sendto() failed while sending ACK");
        } else {
            printf("ACK sent for packet with sequence number %d\n", ackPacket.seq_no);
        }
    }
    // Other logic for different packet types would go here
}

printf("Server: Closing the connection...\n");
    close(sock); // Cleanup
    return 0;
}


// Dummy signal handler for alarms, which could be used for implementing timeouts
void CatchAlarm(int ignored) {
    // This function is a placeholder, potentially useful for handling timeouts
    // Currently, it does nothing but could be expanded based on requirements
    exit(0);
}