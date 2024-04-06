Kory Mayberry
Ashley Judson
Nathan Peckam


This document provides an overview and instructions for the three key files in our networking project. 
The files are designed to demonstrate a simple client-server communication model using custom packet structures.
Files

    packetStruct.c: This file defines the structure of the packets used for communication between the client and server. 
    It includes definitions for different types of packets that could be sent or received, including but not limited to, identification, 
    data, and control packets.

    updated_client_with_packets.c: Implements the client side of the communication.
     It utilizes the packet structures defined in packetStruct.c to send and receive data to and from the server.
      The client demonstrates how to construct various types of packets, send them to the server, and process incoming packets from the server.

    updated_server_with_packets.c: Complements the client file by implementing the server side of the communication.
     It listens for incoming connections, deciphers the types of packets received, and responds accordingly.
      The server is capable of handling multiple packet types and performs appropriate actions based on the packet contents.

Usage Instructions

    Compile each C file using your preferred C compiler. For example, using gcc, you can compile the files as follows:

gcc -o client updated_client_with_packets.c packetStruct.c
gcc -o server updated_server_with_packets.c packetStruct.c

Start the server program before running the client to ensure the client can connect to it. 
The server will request your desired loss rate with a float from 0-0.99

Start the server with ./server
Run the client with ./client

The sliding window is 1 to ensure full packet retransmission.