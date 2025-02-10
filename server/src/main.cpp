#include "TxRx.h"
#include <wiringPi.h>

int main() {
    unsigned char *buffer = (unsigned char *)malloc(BUFFER_SIZE * sizeof(*buffer));  // Выделение памяти для буфера

    /*if (wiringPiSetupGpio() == -1) {
        perror("GPIO setup failed");
        return 0;
    }

    int gpio_pin = 19; // GPIO номер для 37 пина на плате
    pinMode(gpio_pin, INPUT); // Настройка пина как вход
    pullUpDnControl(gpio_pin, PUD_DOWN); // Подтяжка к "земле" для стабильности
*/
    // while (1) {
    // RxEth(buffer);
    // Tx(buffer);
    /*if (kbhit()) { // Проверка нажатия клавиши
        char key = getchar();
        if (key == 'p') {
            printf("Transmitting audio...\n");
            audioTxEth_client(buffer); // Захват и передача звука
        }
    } else {
        printf("Receiving audio...\n");
        audioRxEth_client(buffer); // Воспроизведение звука
    }*/

    audioRxEth_client(buffer);
    // audioTxEth_PI(buffer);

    /*if (digitalRead(gpio_pin) == HIGH){
        audioTxEth_PI(buffer);
    }
    if (digitalRead(gpio_pin) == LOW){
        audioRxEth_PI(buffer);
    }*/
    // Rx(buffer);
    // TxEth(buffer);

    //}
    free(buffer);

    return 0;
}