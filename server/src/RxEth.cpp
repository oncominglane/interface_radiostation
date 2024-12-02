#include "TxRx.h"

void RxEth(unsigned char * buffer) { 
    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    // Создаем сокет
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error opening socket");
        exit(1);
    }

    // Устанавливаем опцию SO_REUSEADDR
    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    // Инициализируем структуру serv_addr
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    // Привязываем сокет к адресу и порту
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error on binding");
        exit(1);
    }

    // Слушаем входящие соединения
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    // Принимаем входящее соединение
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

    if (newsockfd < 0) {
        perror("Error on accept");
        exit(1);
    }

    // Читаем данные из сокета
    int n = read(newsockfd, buffer, BUFFER_SIZE);
    if (n < 0) {
        perror("Error reading from socket");
        exit(1);
    }
    
    // Закрываем сокеты
    close(newsockfd);
    close(sockfd);
}   