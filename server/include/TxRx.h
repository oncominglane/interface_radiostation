#pragma once

#include <cmath>
#include <csignal>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <iostream>
#include <alsa/asoundlib.h>
#include <alsa/pcm.h>

#define DEV_DIR "/dev"
#define BUFFER_SIZE 1024
#define PERIODS 512;
#define TTY "ttyAMA"
#define PORT 5678

void Rx(unsigned char * buffer);
void Tx(unsigned char * buffer);
void RxEth(unsigned char * buffer);
void TxEth(unsigned char * buffer);
void audioRxEth(unsigned char * buffer);
void audioTxEth(unsigned char * buffer);
char * find_ttyUSB_port();
void MyCallback(snd_async_handler_t *pcm_callback);

