#include "TxRx.h"
#include <sys/ioctl.h>
#define DEV_DIR "/dev"

void Tx(unsigned char *buffer) {
    int            fd;
    struct termios options;

    printf("TX EXECS NOW\n");

    char *port = find_ttyUSB_port();

    printf("Found ttyAMA port: %s\n", port);

    // Открываем COM порт для передачи
    fd = open(port, O_RDWR | O_NOCTTY);
    if (fd == -1) {
        perror("open_port: Unable to open /dev/ttyAMA0 - ");
    }

    // Получаем текущие параметры порта
    if (tcgetattr(fd, &options) < 0)
        printf("Error getting options");

    // Устанавливаем стандартные параметры порта для передачи
    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);

    // Устанавливаем параметры порта
    options.c_cflag &= ~(CSIZE | CSTOPB | PARENB);  // Очищаем биты размера байта, стоп-бита и биты контроля четности
    options.c_cflag |= CS8;  // Устанавливаем 8 битов данных
    options.c_cflag |=
        CLOCAL | CREAD |
        HUPCL;  // Устанавливаем флаги для локального режима, разрешения приема данных и сброса линии при закрытии
    options.c_iflag |= IGNBRK | IGNPAR;  // Игнорируем BREAK-сигналы и ошибки битов четности
    options.c_oflag = 0;                 // Отключаем обработку выходного потока
    options.c_lflag = 0;                 // Отключаем обработку локального потока

    // Устанавливаем флаги управления потоком (например, RTS/CTS)
    options.c_cflag |= CRTSCTS;

    // Включаем сигнал RTS
    int status;
    ioctl(fd, TIOCMGET, &status);  // Получаем текущее состояние сигналов
    status |= TIOCM_RTS;           // Включаем RTS
    ioctl(fd, TIOCMSET, &status);  // Устанавливаем новое состояние сигналов

    // Применяем новые параметры порта
    tcsetattr(fd, TCSANOW, &options);

    // Отправляем данные через COM порт
    int bytes_written = write(fd, buffer, BUFFER_SIZE);

    if (bytes_written < 0) {
        perror("Error writing to port - ");
        close(fd);
    }

    /*ioctl(fd, TIOCMGET, &status); // Получаем текущее состояние сигналов
    status &= ~TIOCM_DTR; // Отключаем RTS
    ioctl(fd, TIOCMSET, &status);*/

    // Закрываем файл и COM порт для передачи
    close(fd);
}