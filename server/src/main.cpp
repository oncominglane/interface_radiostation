#include "TxRx.h"

int main() {   
    
    unsigned char *buffer = (unsigned char *)malloc(BUFFER_SIZE * sizeof(*buffer));// Выделение памяти для буфера

    //while (1) {
        //RxEth(buffer);
        //Tx(buffer);
        audioRxEth(buffer);
        //audioTxEth(buffer);
        //Rx(buffer);
        //TxEth(buffer);
    //}

    free(buffer);

    return 0;
}