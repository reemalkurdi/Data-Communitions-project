#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib") // Link Winsock library

#define BUFFER_SIZE 1024

void clean_newline(char *str) {
    str[strcspn(str, "\n")] = 0;
}


void communicate_with_server(SOCKET sock, char *message, char *response) {
    send(sock, message, strlen(message), 0);
    memset(response, 0, BUFFER_SIZE);
    recv(sock, response, BUFFER_SIZE, 0);
    printf("%s\n", response);
}

int main() {
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in server_addr;
    char input[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    int authenticated = 0;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Winsock initialization failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation failed. Error Code: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Connection failed. Error Code: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("Connected to the server.\n");

    while (1) {
        if (!authenticated) {
            printf("\nOptions:\n1. Login\n2. Signup\n3. Logout\nChoose an option: ");
            fgets(input, sizeof(input), stdin);
            clean_newline(input);

            if (strcmp(input, "1") == 0) {
                printf("Enter email: ");
                fgets(buffer, sizeof(buffer), stdin);
                clean_newline(buffer);
                char email[BUFFER_SIZE];
                strcpy(email, buffer);

                printf("Enter password: ");
                fgets(buffer, sizeof(buffer), stdin);
                clean_newline(buffer);
                char password[BUFFER_SIZE];
                strcpy(password, buffer);

                snprintf(buffer, sizeof(buffer), "LOGIN %s %s", email, password);
                communicate_with_server(sock, buffer, buffer);

                if (strcmp(buffer, "You logged in") == 0) {
                    authenticated = 1;
                }

            } else if (strcmp(input, "2") == 0) {
                printf("Enter email: ");
                fgets(buffer, sizeof(buffer), stdin);
                clean_newline(buffer);
                char email[BUFFER_SIZE];
                strcpy(email, buffer);

                printf("Enter password: ");
                fgets(buffer, sizeof(buffer), stdin);
                clean_newline(buffer);
                char password[BUFFER_SIZE];
                strcpy(password, buffer);

                snprintf(buffer, sizeof(buffer), "SIGNUP %s %s", email, password);
                communicate_with_server(sock, buffer, buffer);

            } else if (strcmp(input, "3") == 0) {
                communicate_with_server(sock, "LOGOUT", buffer);
                break;

            } else {
                printf("Invalid option. Please try again.\n");
            }
        } else {
            printf("Enter a mathematical expression (or type 'exit' to logout): ");
            fgets(buffer, sizeof(buffer), stdin);
            clean_newline(buffer);

            if (strcmp(buffer, "exit") == 0) {
                communicate_with_server(sock, "exit", buffer);
                break;
            }

            communicate_with_server(sock, buffer, buffer);
        }
    }

    closesocket(sock);
    WSACleanup();
    printf("Disconnected from the server.\n");

    return 0;
}
