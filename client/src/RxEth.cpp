#include "TxRxEth.h"

#define PORT 5678  //ноутбук и малинка

std::string receive_eth() {
    try {
    int       sockfd, newsockfd;
    socklen_t clilen;

    char               buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int                n;

    // Создаем сокет
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        std::cerr << "Error opening socket" << std::endl;
        std::exit(1);
    }

    // Устанавливаем опцию SO_REUSEADDR
    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    // Инициализируем структуру serv_addr
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port        = htons(PORT);

    // Привязываем сокет к адресу и порту
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Error on binding" << std::endl;
        std::exit(1);
    }

    // Слушаем входящие соединения
    listen(sockfd, 5);      
    clilen = sizeof(cli_addr);

    // Принимаем входящее соединение
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0) {
        std::cerr << "Error on accept" << std::endl;
        std::exit(1);
    }

    // Читаем данные из сокета
    memset(buffer, 0, sizeof(buffer));
    time_t start_time = time(NULL);
    while ((time(NULL) - start_time) < 0.5) {
        n = read(newsockfd, buffer, sizeof(buffer) - 1);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                continue;  // Если нет данных для чтения, продолжаем цикл
            else {
                std::cerr << "Error reading from socket" << std::endl;
                exit(1);
            }
        }
        if (n > 0)
            break;
    }

    std::string str(buffer, n);  // Создание строки с явным указанием длины

    // Закрываем сокеты
    close(newsockfd);
    close(sockfd);

    return str;
     } catch (const std::exception& e) {
        std::cerr << "receive_eth exception: " << e.what() << std::endl;
        throw;
    }
}
