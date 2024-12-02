#include "TxRx.h"

#define SERVER_IP "192.168.0.105" // IP адрес дом
//#define SERVER_IP "10.10.1.211"  // IP адрес работа

void TxEth(unsigned char * buffer) { // TODO Change int to void, or return error codes instead of exit
    int sockfd;
    struct sockaddr_in server_addr;

    printf("TX_ETH EXECS NOW\n");

    // Создание сокета
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    // Настройка структуры адреса сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Подключение к серверу
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
    
    /*for (int i = 0; i < BUFFER_SIZE; ++i) 
        printf("Transmited data: %02X (Char: `%c`, Int: `%d`)\n", buffer[i], buffer[i], buffer[i]);
*/
    // Отправка сообщения на сервер
    send(sockfd, buffer, 255, 0);
    close(sockfd);
}