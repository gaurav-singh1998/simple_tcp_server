#include<iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h> // Add this header for inet_ntop()

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        throw std::runtime_error("Couldn't create a socket\n");
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr*)(&address), sizeof(address)) == -1) {
        close(server_fd);
        throw std::runtime_error("Error binding to port\n");
    }

    if (listen(server_fd, 5) == -1) {
        close(server_fd);
        throw std::runtime_error("Error listening to connections\n");
    }

    int con_clients = 0;

    while (true) {
        sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket == -1) {
            close(server_fd);
            throw std::runtime_error("Error in accepting connections\n");
        }

        con_clients += 1;

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        std::cout << "Client connected from IP address: " << client_ip << " " << "with concurrent clients " << con_clients << std::endl;

        while (true) {
            char buffer[1024];
            int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
            if (bytes_received <= 0) {
                break; // Exit the loop if there's an error or connection closed
            }
            buffer[bytes_received] = '\0'; // Null-terminate the received data
            std::cout << "Received from client " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << ": " << buffer << std::endl;
        }

        close(client_socket);

        con_clients -= 1;

    }

    return 0;
}
