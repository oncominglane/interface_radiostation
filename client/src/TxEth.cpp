#include "TxRxEth.h"

//#define SERVER_IP "192.168.1.1"  // IP адрес сервера       ноутбук
#define SERVER_IP "10.10.1.62"  // IP адрес сервера      малинка
//#define SERVER_IP "192.168.0.119"  // IP адрес сервера       малинка дома

#define SERVER_PORT 5678  // Порт сервера        ноутбук и малинка

int transmit_eth(std::string message1) {
    const char *message = message1.c_str();

    int                sockfd;
    struct sockaddr_in server_addr;
    // Создание сокета
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    // Настройка структуры адреса сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Подключение к серверу
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Отправка сообщения на сервер
    send(sockfd, message, strlen(message), 0);
    printf("Message sent to server: %s\n", message);

    close(sockfd);
    return 0;
}
