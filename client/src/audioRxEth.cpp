#include "TxRxEth.h"
#include <alsa/pcm.h>
#include <cerrno>
#include <cstdlib>

void audioRxEth(unsigned char *buffer, std::atomic<bool> &audio_receive, std::atomic<bool> &signal_received) {
    //Параметры для захвата звука
    snd_pcm_t           *playback_handle;
    snd_pcm_hw_params_t *hw_params;
    // snd_pcm_sw_params_t *sw_params;
    // snd_async_handler_t *pcm_callback;

    // Создание сокета для передачи данных
    int                sockfd, newsockfd;
    struct sockaddr_in serv_addr, cli_addr;

    unsigned int      resample      = 1;
    unsigned int      sampleRate    = 44100;
    long int          dataCapacity  = 0;
    int               channels      = 1;
    snd_pcm_uframes_t local_buffer  = BUFFER_SIZE;
    snd_pcm_uframes_t local_periods = PERIODS;
    socklen_t         clilen;
    snd_pcm_state_t   state;

    // Настройка сокета
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return;
    }

    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port        = htons(PORT);

    int bindResult = bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (bindResult < 0) {
        std::cerr << "Error on binding: " << strerror(errno) << " (errno: " << errno << ")" << std::endl;
        throw std::runtime_error("Binding failed.");
    }

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    // Открываем PCM устройство
    if (snd_pcm_open(&playback_handle, "plughw:0,0", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK) < 0) {
        perror("Cannot open audio device");
        close(sockfd);
        return;
    }

    // Выделение памяти для hw_params с использованием malloc
    if (snd_pcm_hw_params_malloc(&hw_params) < 0) {
        perror("Cannot allocate hardware parameters");
        snd_pcm_close(playback_handle);
        close(sockfd);
        return;
    }

    if (snd_pcm_hw_params_any(playback_handle, hw_params) < 0) {
        perror("Cannot configure hardware parameters on this PCM device");
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(playback_handle);
        close(sockfd);
        return;
    }

    snd_pcm_hw_params_get_buffer_size(hw_params, &local_buffer);
    snd_pcm_hw_params_get_period_size(hw_params, &local_periods, 0);

    printf("Buffer size: %lu, Period size: %lu\n", local_buffer, local_periods);

    if (snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_S16_LE) < 0) {
        perror("Cannot set sample format");
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(playback_handle);
        close(sockfd);
        return;
    }

    if (snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
        perror("Cannot set access rate");
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(playback_handle);
        close(sockfd);
        return;
    }

    if (snd_pcm_hw_params_set_channels(playback_handle, hw_params, channels) < 0) {
        perror("Cannot set channel count");
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(playback_handle);
        close(sockfd);
        return;
    }

    if (snd_pcm_hw_params_set_rate_near(playback_handle, hw_params, &sampleRate, 0) < 0) {
        perror("Cannot set rate near");
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(playback_handle);
        close(sockfd);
        return;
    }

    if (snd_pcm_hw_params_set_rate_resample(playback_handle, hw_params, resample) < 0) {
        perror("Cannot set sample rate");
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(playback_handle);
        close(sockfd);
        return;
    }

    if (snd_pcm_hw_params_set_buffer_size_near(playback_handle, hw_params, &local_buffer) < 0) {
        perror("Cannot set buffer size near");
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(playback_handle);
        close(sockfd);
        return;
    }

    if (snd_pcm_hw_params_set_period_size_near(playback_handle, hw_params, &local_periods, 0) < 0) {
        perror("Cannot set period size near");
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(playback_handle);
        close(sockfd);
        return;
    }

    if (snd_pcm_hw_params(playback_handle, hw_params) < 0) {
        perror("Cannot set hardware parameters");
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(playback_handle);
        close(sockfd);
        return;
    }

    // Освобождение выделенной памяти
    snd_pcm_hw_params_free(hw_params);

    printf("Buffer size: %lu, Period size: %lu\n", local_buffer, local_periods);

    while (1) {
        if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) < 0) {
            perror("Accept error");
            continue;
        }

        printf("Client connected\n");

        if (snd_pcm_prepare(playback_handle) < 0) {
            printf("Error preparing\n");
        }

        // Основной цикл для приёма и воспроизведения звуковых данных
        while (audio_receive) {
            int n = recv(newsockfd, buffer, BUFFER_SIZE, 0);

            // включаем флаг приема для индикатора
            signal_received = true;

            if (n <= 0) {
                if (n == 0) {
                    printf("Connection closed by client\n");
                    signal_received = false;  // Сбрасываем флаг приема сигнала
                    snd_pcm_drop(playback_handle);
                    close(newsockfd);
                    memset(buffer, 0, BUFFER_SIZE);
                    break;
                }
                else {
                    perror("Receive error");
                    signal_received = false;  // Сбрасываем флаг приема сигнала
                    snd_pcm_drop(playback_handle);
                    close(newsockfd);
                    memset(buffer, 0, BUFFER_SIZE);
                }
                break;
            }

            int err    = 0;
            int frames = n / (channels * 2);
            state      = snd_pcm_state(playback_handle);
            if (state == SND_PCM_STATE_XRUN) {
                snd_pcm_prepare(playback_handle);
            }

            err = snd_pcm_writei(playback_handle, buffer, frames);
            // Воспроизводим данные с помощью ALSA
            if (err < 0) {
                if (err == -EPIPE) {
                    fprintf(stderr, "Temporary underrun, retrying...\n");  //Обработка, если установлен флаг SND_PCM_NONBLOCK
                    snd_pcm_recover(playback_handle, err, 0);
                    continue;
                }
                if (err == EAGAIN) {
                    fprintf(stderr, "Temporary unavailable, retrying...\n");
                    continue;
                }
            }
            dataCapacity += n;

            // printf("\ndataCapacity: %ld\n\n", dataCapacity);
        }
    }

    // Освобождаем ресурсы
    snd_pcm_drop(playback_handle);
    snd_pcm_close(playback_handle);
    close(newsockfd);
    close(sockfd);
    memset(buffer, 0, BUFFER_SIZE);
}