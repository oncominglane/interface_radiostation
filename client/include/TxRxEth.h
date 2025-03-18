#pragma once

#include <cmath>
#include <csignal>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <thread>
#include <functional>
#include <atomic>
#include <cerrno>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <dirent.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <alsa/asoundlib.h>
#include <alsa/asoundlib.h>
#include <alsa/pcm.h>

#define DEV_DIR "/dev"
#define BUFFER_SIZE 8192
#define PERIODS 1024
#define TTY "ttyAMA"
#define PORT 5678
#define PORT_ETH 7777
#define SAMPLERATE 44100

#define SERVER_IP "192.168.1.2" // подключение по проводу
//#define SERVER_IP "10.10.1.62"  // подключение по WiFi офис
//#define SERVER_IP "192.168.0.119" // подключение по WiFi общага

int         transmit_eth(std::string message1);
void        audioTxEth(unsigned char *buffer, std::atomic<bool> &audio_transmit);
void        audioRxEth(unsigned char *buffer, std::atomic<bool> &audio_receive, std::atomic<bool> &signal_received);
std::string receive_eth();

// 403.050 МГц
