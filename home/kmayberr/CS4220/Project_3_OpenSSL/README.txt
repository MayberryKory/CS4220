Kory Mayberry
Ashley Judson
Nathan Peckham

We have neither given nor received unauthorized assistance on this work.

Project Title: Secure HTTP Server and Client with OpenSSL

Description:

This project involves creating a simple HTTPS server and client using C and OpenSSL. The server listens on a specified port and manages SSL connections,
responding to HTTP GET requests with a simple text message. The client establishes a connection to the server, performs an SSL handshake, sends an HTTP GET request,
and processes the server’s response.

Files:

1. http_server.c - Implements the HTTPS server.
2. http_client.c - Implements the HTTPS client.
3. certs - Directory containing the generated certificates for SSL operation.

Building the Program:

Ensure that GCC and OpenSSL are installed on your machine:

1. Install OpenSSL:
   - On Ubuntu: sudo apt-get install libssl-dev
   - On macOS: brew install openssl

2. Compilation Instructions:
   - Compile the server: gcc -o http_server http_server.c -lssl -lcrypto
   - Compile the client: gcc -o http_client http_client.c -lssl -lcrypto
   - Note: If there are issues finding OpenSSL, specify the include and lib paths:
     gcc -o http_client http_client.c -I/opt/homebrew/opt/openssl@3/include -L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto
     gcc -o http_server http_server.c -I/opt/homebrew/opt/openssl@3/include -L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto

Running the Program:

1. Start the server with: ./http_server
2. In a new terminal, run the client with: ./http_client

Both the server and client will log connection details, SSL handshakes, and data exchanges to the console.

Challenges Overcome:

1. Integrating OpenSSL: Addressed issues related to linking and initializing OpenSSL within the C environment.
2. Error Handling: Enhanced error management to ensure robust handling of SSL handshake failures and network communication errors.
3. Platform Compatibility: Modified the code to ensure functionality across Linux and macOS, accommodating different 
    OpenSSL paths and system configurations.
4. Forcing OpenSSL to use AES was really confusing.

Resources Used:

1. OpenSSL Documentation: https://www.openssl.org/docs/
2. Stack Overflow: Consulted various threads for troubleshooting OpenSSL integration and network programming.
3. Network Programming lectures and personal notes from UCCS, provided by Dr. Serena Sullivan.

Notes:

- The server and client are set to operate on localhost (127.0.0.1) and port 4433. These settings can be modified in the source code.
- Check firewall settings to ensure the port is open for traffic to facilitate proper communication between the client and server.
