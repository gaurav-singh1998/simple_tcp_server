#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <semaphore.h>

// Semaphore to limit the number of concurrent connections
sem_t connection_sem;

// Function to handle each client request
void handle_client(int client_socket) {
    // Send a simple HTTP response
    const char *response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello, World!";
    sleep(5);
    send(client_socket, response, strlen(response), 0);

    // Close the client socket
    close(client_socket);

    // Release the semaphore to allow another connection
    sem_post(&connection_sem);
}

int main() {
    // Initialize the semaphore with a maximum count of 3
    sem_init(&connection_sem, 0, 3);

    // Create a socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Error creating socket\n";
        return 1;
    }

    // Set up the address structure
    sockaddr_in address;
    address.sin_family = AF_INET;  // IPv4
    address.sin_addr.s_addr = INADDR_ANY;  // Accept connections from any address
    address.sin_port = htons(8080);  // Port to listen on (example: 8080)

    // Bind the socket to the specified port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        std::cerr << "Error binding to port\n";
        close(server_fd);
        return 1;
    }

    // Start listening for incoming connections
    if (listen(server_fd, 5) == -1) {
        std::cerr << "Error listening for connections\n";
        close(server_fd);
        return 1;
    }

    std::cout << "Server listening on port 8080...\n";

    while (true) {
        // Wait until a slot is available in the semaphore
        sem_wait(&connection_sem);

        // Accept incoming connections
        int client_socket = accept(server_fd, nullptr, nullptr);
        if (client_socket == -1) {
            std::cerr << "Error accepting connection\n";
            close(server_fd);
            return 1;
        }

        std::cout << "Connection accepted\n";

        // Spawn a new thread to handle the client request
        std::thread(handle_client, client_socket).detach();
    }

    // Close the server socket (this code is unreachable in this example)
    close(server_fd);

    // Destroy the semaphore
    sem_destroy(&connection_sem);

    return 0;
}
