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
#include <wiringPi.h>

#define DEV_DIR "/dev"
#define BUFFER_SIZE 16384
#define PERIODS 4096
#define TTY "ttyAMA"
#define PORT 5678
#define CHANNELS 1
#define SAMPLERATE 48000
#define RESAMPLE 1
#define CAPTURE_DEV "hw:0,6"
#define PLAYBACK_DEV "hw:0,0"

void Rx(unsigned char * buffer);
void Tx(unsigned char * buffer);
void RxEth(unsigned char * buffer);
void TxEth(unsigned char * buffer);
void audioRxEth_client(unsigned char * buffer);
void audioTxEth_PI(unsigned char * buffer);
void audioTxEth_client(unsigned char * buffer);
void audioRxEth_PI(unsigned char * buffer);
char * find_ttyUSB_port();
snd_pcm_t * setup_hw(const char * device, unsigned int channels, unsigned int rate, snd_pcm_uframes_t buffer_size, snd_pcm_uframes_t period_size, int sockfd);
int kbhit(void);


