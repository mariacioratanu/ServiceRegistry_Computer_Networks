#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <string>
#include <iostream>
#include <sstream>

// Wrapper class for client-side communication
class LinuxAPIClientCommunication {
public:
    // Wrapper for sending data to the server
    static bool safe_write(int socket, const std::string &message) {
        ssize_t bytesWritten = write(socket, message.c_str(), message.size());
        if (bytesWritten < 0) {
            perror("[client] Error at write() to server.\n");
            return false;
        }
        return true;
    }

    // Wrapper for reading data from the server
    static ssize_t safe_read(int socket, char *buffer, size_t size) {
        ssize_t bytesRead = read(socket, buffer, size);
        if (bytesRead < 0) {
            perror("[client] Error at read() from server.\n");
        }
        return bytesRead;
    }

    // Function to handle receiving JSON responses
    static void receive_json_response(int socket) {
        char buffer[1024];
        ssize_t bytes_read = safe_read(socket, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            std::cout << "[client] Received JSON response: " << buffer << "\n";
        } else {
            std::cerr << "[client] Failed to read server response.\n";
        }
    }
};

class Client {
private:
    int socket_fd;
    struct sockaddr_in server;
    int port;

    // Connects to the server
    bool connect_to_server(const std::string &server_address) {
        // Create the socket
        if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("Error at socket().\n");
            return false;
        }

        // Fill in the server structure
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = inet_addr(server_address.c_str());
        server.sin_port = htons(port);

        // Connect to the server
        if (connect(socket_fd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {
            perror("[client] Error at connect().\n");
            return false;
        }

        return true;
    }

    // Handles the communication
    void communication() {
        std::string username;
        std::string password;

        // Prompt the user for username and password
        std::cout << "Enter username for new account: ";
        std::getline(std::cin, username);

        std::cout << "Enter password for new account: ";
        std::getline(std::cin, password);

        // Build the JSON request
        std::ostringstream jsonRequest;
        jsonRequest << "{ \"object\": \"user\", \"command\": \"register\", \"username\": \""
                    << username << "\", \"password\": \"" << password << "\", \"role\": \"user\" }";
        std::string json = jsonRequest.str();

        // Send the JSON request
        if (!LinuxAPIClientCommunication::safe_write(socket_fd, json)) {
            std::cerr << "[client] Failed to send JSON request.\n";
            return;
        } else {
            std::cout << "[client] Sent JSON request: " << json << "\n";
        }

        // Wait for the server response
        LinuxAPIClientCommunication::receive_json_response(socket_fd);
    }

public:
    Client(int port) : port(port) {}

    int run(const std::string &server_address) {
        if (!connect_to_server(server_address)) {
            return errno;
        }

        std::cout << "[client] Connected to the server at " << server_address << ":" << port << "\n";
        communication();

        // Close the socket after the communication
        close(socket_fd);
        return 0;
    }
};

int main(int argc, char *argv[]) {
    // Validate command-line arguments
    if (argc != 3) {
        std::cerr << "Syntax: " << argv[0] << " <server_address> <port>\n";
        return -1;
    }

    // Parse arguments
    std::string server_address = argv[1];
    int port = atoi(argv[2]);

    // Run the client
    Client client(port);
    return client.run(server_address);
}
