#include <stdio.h>      // Standard input/output definitions
#include <sys/socket.h> // Socket programming definitions
#include <arpa/inet.h>  // Definitions for internet operations
#include <stdlib.h>     // Standard library definitions
#include <string.h>     // String manipulation definitions
#include <unistd.h>     // POSIX operating system API
#include <errno.h>      // Defines macros for reporting and retrieving error conditions
#include <signal.h>     // Signal handling definitions
#include "packetStruct.c"  // Include the Go-Back-N packet structure definitions
#include <sys/select.h>  // For select()

// Defines for timeout, maximum tries, and hardcoded user inputs
#define TIMEOUT_SECS 3
#define MAXTRIES     10
#define SERVER_IP "127.0.0.1" // Example hardcoded server IP
#define SERVER_PORT 12345       // Example hardcoded server port
#define CHUNK_SIZE  256         // Example hardcoded chunk size, must be less than 512
#define WINDOW_SIZE 4           // Example hardcoded window size

// Global variables for handling state across functions
int sendBase = 0;
int nextSeqNum = 0;
int ackReceived = 0;           // Base index for the window
int tries = 0;
int windowSize = WINDOW_SIZE; // Size of the sliding window, now a constant
int sendflag = 1;        // Flag to control sending of packets


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

// Handler for the SIGALRM signal
void CatchAlarm(int ignored) {
    tries += 1; // Increment the try counter
    sendflag = 1; // Set flag to trigger packet sending
}

void sendPackets(int sock, struct sockaddr_in gbnServAddr, struct packetStruct packets[], int nPackets) {
    struct timeval tv;
    fd_set readfds;
    int maxfd = sock + 1;
    int waitForAckFor = 0; // This represents the lowest packet in the window for which ACK is awaited.

    while (sendBase < nPackets) {
        // Send packets within the window
        while (sendBase + waitForAckFor < sendBase + WINDOW_SIZE && sendBase + waitForAckFor < nPackets) {
            int seqNum = sendBase + waitForAckFor;
            printf("Sending packet %d\n", seqNum);
            packets[seqNum].seq_no = seqNum;
            
            if (sendto(sock, &packets[seqNum], sizeof(struct packetStruct), 0, 
                       (struct sockaddr *)&gbnServAddr, sizeof(gbnServAddr)) < 0) {
                perror("sendto() failed");
                printf("Error sending packet %d. Closing socket...\n", seqNum);
                close(sock);
                exit(EXIT_FAILURE);
            }
            printf("Packet %d sent\n", seqNum);
            waitForAckFor++;
        }

        // Initialize or reset the timer for waiting ACKs
        tv.tv_sec = 2; // 2 seconds for example
        tv.tv_usec = 0;

        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);

        int retval = select(maxfd, &readfds, NULL, NULL, &tv);
        if (retval == -1) {
            perror("select() failed");
            exit(EXIT_FAILURE);
        } else if (retval) {
            // ACK is available to be read
            struct packetStruct ackPacket;
            if (recvfrom(sock, &ackPacket, sizeof(ackPacket), 0, NULL, NULL) > 0) {
                if (ackPacket.type == 2 && ackPacket.seq_no == sendBase) { // Ensure ACK is for the lowest packet
                    printf("ACK received for packet %d\n", ackPacket.seq_no);
                    sendBase++; // Move window forward only for the lowest acknowledged packet
                    waitForAckFor--; // Adjust waitForAckFor since the window has slid forward
                }
            }
        } else {
            // Timeout occurred
            printf("Timeout, resending packets starting from %d\n", sendBase);
            // Reset waitForAckFor to start resending packets from the lowest unacknowledged one
            waitForAckFor = 0;
        }
    }
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

    sendPackets(sock, gbnServAddr, packets, nPackets);
    //printf("File content sent to the server successfully.\n");

    printf("Client: Closing the socket...\n");
    close(sock); // Close the socket when done
    return 0;
}


