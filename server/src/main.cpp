#include "TxRx.h"
#include <wiringPi.h>

int main() {
    unsigned char *buffer = (unsigned char *)malloc(BUFFER_SIZE * sizeof(*buffer));  // Выделение памяти для буфера

    

    audioTxEth_client(buffer);
    // audioTxEth_PI(buffer);

    free(buffer);

    return 0;
}