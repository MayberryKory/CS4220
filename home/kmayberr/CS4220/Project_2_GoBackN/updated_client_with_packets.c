#include <stdio.h>      // Standard input/output definitions
#include <sys/socket.h> // Socket programming definitions
#include <arpa/inet.h>  // Definitions for internet operations
#include <stdlib.h>     // Standard library definitions
#include <string.h>     // String manipulation definitions
#include <unistd.h>     // POSIX operating system API
#include <errno.h>      // Defines macros for reporting and retrieving error conditions
#include <signal.h>     // Signal handling definitions
#include "packetStruct.c"  // Include the Go-Back-N packet structure definitions

// Defines for timeout, maximum tries, and hardcoded user inputs
#define TIMEOUT_SECS 3
#define MAXTRIES     10
#define SERVER_IP "127.0.0.1" // Example hardcoded server IP
#define SERVER_PORT 12345       // Example hardcoded server port
#define CHUNK_SIZE  256         // Example hardcoded chunk size, must be less than 512
#define WINDOW_SIZE 4           // Example hardcoded window size

// Global variables for handling state across functions
int tries = 0;           // Counter for the number of tries
int base = 0;            // Base index for the window
int windowSize = WINDOW_SIZE; // Size of the sliding window, now a constant
int sendflag = 1;        // Flag to control sending of packets

// Function prototypes
void CatchAlarm(int ignored); // Handler for the alarm signal
int max(int a, int b);        // Returns the maximum of two integers
int min(int a, int b);        // Returns the minimum of two integers

// Function to create a packet from a segment of text
struct packetStruct createPacket(const char* segment, int length, int seq_no) {
    struct packetStruct packet;
    packet.type = 1; 
    packet.seq_no = seq_no;
    packet.length = length;
    strncpy(packet.data, segment, length);
    packet.data[length] = '\0'; // Ensure null-termination
    return packet;
}

int main(int argc, char *arg[]) {
    printf("Client: Starting...\n");
    // Local variables for socket communication, now using hardcoded values
    int sock;                       // Socket descriptor
    struct sockaddr_in gbnServAddr; // Go-Back-N server address structure
    struct sockaddr_in fromAddr;    // Structure for storing the source address
    unsigned int fromSize;          // Size of the from address structure
    struct sigaction myAction;      // Structure to define signal action
    int respLen;                    // Length of the received datagram
    int packet_received = -1;       // Index of the highest acknowledgment received
    int packet_sent = -1;           // Index of the highest packet sent
    char buffer[8192];              // Buffer to store the data to be sent
    const int chunkSize = CHUNK_SIZE; // Now using a constant value
    int nPackets = 0;               // Number of packets to send, calculated based on chunkSize
    const int datasize = sizeof(buffer); // Size of the data buffer

    // Calculate the number of packets based on the hardcoded chunk size
    nPackets = datasize / chunkSize;
    if (datasize % chunkSize) nPackets++;
    printf("The number of packets is %d\n", nPackets); // Adjust for the last partial chunk

    FILE *file;

    // Open the file
    file = fopen("test.txt", "r");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    // Read file contents into buffer
    size_t bytesRead = fread(buffer, 1, sizeof(buffer) - 1, file);
    buffer[bytesRead] = '\0';  // Ensure null-termination
    fclose(file);  // Close the file as we've finished reading

     // Allocate memory for the packets array
    struct packetStruct* packets = (struct packetStruct*)malloc(nPackets * sizeof(struct packetStruct));
    if (packets == NULL) {
        perror("Failed to allocate memory for packets");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < nPackets; i++) {
        // Calculate the start index for the current chunk
        int startIndex = i * chunkSize;
        // Calculate the length of the current chunk (may be less than chunkSize for the last chunk)
        int currentChunkLength = (bytesRead - startIndex) < chunkSize ? (bytesRead - startIndex) : chunkSize;
        
        // Create a new packet with the current segment
        packets[i] = createPacket(&buffer[startIndex], currentChunkLength, i);

        // Example: Printing packet data to verify
        printf("Packet created %d: %s\n", packets[i].seq_no, packets[i].data);
    }
    
    // Create a UDP socket
    printf("Client: Creating socket...\n");
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }
    

    // Setup the signal handler for the alarm
    myAction.sa_handler = CatchAlarm;
    if (sigfillset(&myAction.sa_mask) < 0) // Block all other signals in handler
        perror("Failed to initialize the signal mask");
    myAction.sa_flags = 0;
    if (sigaction(SIGALRM, &myAction, 0) < 0)
        perror("Failed to set signal handler for SIGALRM");

    // Initialize the server address structure with hardcoded values
    memset(&gbnServAddr, 0, sizeof(gbnServAddr)); // Clear structure
    gbnServAddr.sin_family = AF_INET; // Internet address family
    gbnServAddr.sin_addr.s_addr = inet_addr(SERVER_IP); // Server IP address
    gbnServAddr.sin_port = htons(SERVER_PORT); // Server port

   for(int i = 0; i < nPackets; i++) {
    // Sending each packet in the array to the server
    printf("sending packet %d\n", i);
    if (sendto(sock, &packets[i], sizeof(struct packetStruct), 0, 
               (struct sockaddr *)&gbnServAddr, sizeof(gbnServAddr)) < 0) {
        perror("sendto() failed");
        printf("Client: Error sending packet %d. Closing the socket...\n", i);
        close(sock);
        exit(EXIT_FAILURE);
        }
    printf("Packet %d sent\n", i);
   }
    //printf("File content sent to the server successfully.\n");

    printf("Client: Closing the socket...\n");
    close(sock); // Close the socket when done
    return 0;
}

// Handler for the SIGALRM signal
void CatchAlarm(int ignored) {
    tries += 1; // Increment the try counter
    sendflag = 1; // Set flag to trigger packet sending
}

// Utility functions to find the maximum and minimum of two integers
int max(int a, int b) {
    return b > a ? b : a;
}
int min(int a, int b) {
    return a < b ? a : b;
}
