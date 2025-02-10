#include "TxRx.h"

char *find_ttyUSB_port() {
    DIR           *dir;
    struct dirent *entry;
    char          *port = NULL;

    dir = opendir(DEV_DIR);
    if (!dir) {
        perror("Failed to open /dev directory");
        return NULL;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, TTY, 6) == 0) {
            port = (char *)malloc(strlen(DEV_DIR) + strlen(entry->d_name) + 2);
            if (!port) {
                perror("Memory allocation error");
                closedir(dir);
                return NULL;
            }
            sprintf(port, "%s/%s", DEV_DIR, entry->d_name);
            break;
        }
    }

    closedir(dir);
    return port;
}

void Rx(unsigned char *buffer) {
    int            fd;
    struct termios options;

    printf("RX EXECS NOW\n");

    char *port = find_ttyUSB_port();
    printf("Found ttyAMA port: %s\n", port);

    // Открываем COM порт для передачи
    fd = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd == -1) {
        perror("open_port: Unable to open /dev/ttyAMA0 - ");
    }

    // Получаем текущие параметры порта
    tcgetattr(fd, &options);

    // Устанавливаем стандартные параметры порта для приёма
    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);

    // Устанавливаем параметры порта
    options.c_cflag &= ~PARENB;  // Без контроля четности
    options.c_cflag &= ~CSTOPB;  // Один стоп-бит
    options.c_cflag &= ~CSIZE;   // Сбрасываем биты размера байта
    options.c_cflag |= CS8;      // Устанавливаем 8 битов данных

    // Устанавливаем флаг RTS
    options.c_cflag |= CRTSCTS;

    // Отключаем сигнал RTS
    int status;
    ioctl(fd, TIOCMGET, &status);  // Получаем текущее состояние сигналов
    status &= ~TIOCM_RTS;          // Отключаем RTS
    ioctl(fd, TIOCMSET, &status);  // Устанавливаем новое состояние сигналов
    // usleep(100000);

    // Применяем новые параметры порта
    tcsetattr(fd, TCSANOW, &options);

    ssize_t bytes_read = 0;

    // Ожидаем первой порции данных
    while (bytes_read <= 0) {
        bytes_read = read(fd, buffer, BUFFER_SIZE);
        if (bytes_read == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Если нет данных для чтения, продолжаем ожидать
                continue;
            }
            else {
                perror("read error");
                close(fd);
            }
        }
    }

    memset(buffer, 0, BUFFER_SIZE);

    time_t start_time = time(NULL);  // Засекаем начальное время чтения данных

    while ((time(NULL) - start_time) < 0.05) {
        bytes_read = read(fd, buffer, BUFFER_SIZE);  // Читаем данные из порта
        if (bytes_read == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Если нет данных для чтения, продолжаем цикл
                continue;
            }
            else {
                perror("read error");
                close(fd);
            }
        }
        buffer += bytes_read;

        /*for (int i = 0; i < bytes_read; ++i)
            printf("Received data: %02X (Char: `%c`, Int: `%d`)\n", buffer[i], buffer[i], buffer[i]);*/
    }
    // Закрываем COM порт для приёма
    close(fd);
}