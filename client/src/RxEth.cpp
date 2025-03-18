#include "TxRxEth.h"

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
            throw std::runtime_error("Error opening socket: " + std::string(strerror(errno)));
        }

        // Устанавливаем опцию SO_REUSEADDR
        int optval = 1;
        // setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

        // Инициализируем структуру serv_addr
        memset((char *)&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family      = AF_INET;
        // serv_addr.sin_addr.s_addr = inet_addr("192.168.0.119");

        /*
        if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
            serv_addr.sin_addr.s_addr = INADDR_ANY; // Если не сработало, слушаем на всех интерфейсах
        }
        */


        serv_addr.sin_addr.s_addr = INADDR_ANY; 

        serv_addr.sin_port        = htons(PORT_ETH);




        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &serv_addr.sin_addr, ip_str, INET_ADDRSTRLEN);
        std::cout << "Binding to " << ip_str << ":" << ntohs(serv_addr.sin_port) << std::endl;



        
        // Привязываем сокет к адресу и порту
        if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            close(sockfd);
            throw std::runtime_error("Error on binding: " + std::string(strerror(errno)));
        }

        // Слушаем входящие соединения
        listen(sockfd, 5);
        clilen = sizeof(cli_addr);

        // Принимаем входящее соединение
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0) {
            close(sockfd);
            throw std::runtime_error("Error on accept: " + std::string(strerror(errno)));
        }

        // Читаем данные из сокета
        memset(buffer, 0, sizeof(buffer));
        time_t start_time = time(NULL);
        while ((time(NULL) - start_time) < 1) {
            n = read(newsockfd, buffer, sizeof(buffer) - 1);
            if (n < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    continue;  // Если нет данных для чтения, продолжаем цикл
                else {
                    close(newsockfd);
                    close(sockfd);
                    throw std::runtime_error("Error reading from socket: " + std::string(strerror(errno)));
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
    }
    catch (const std::exception &e) {
        std::cerr << "receive_eth exception: " << e.what() << std::endl;
        return "";
    }
}