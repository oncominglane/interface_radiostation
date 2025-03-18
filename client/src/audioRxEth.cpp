#include "TxRxEth.h"

void audioRxEth(unsigned char *buffer, std::atomic<bool> &audio_receive, std::atomic<bool> &signal_received) {
    //Параметры для захвата звука
    snd_pcm_t           *playback_handle;
    snd_pcm_hw_params_t *hw_params;

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

    // Делаем сокет **неблокирующим**
    fcntl(sockfd, F_SETFL, O_NONBLOCK);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port        = htons(PORT);

    std::cout << "Binding to port: " << PORT << std::endl;
    /*
    int bindResult = bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (bindResult < 0) {
        std::cerr << "Error on binding: " << strerror(errno) << " (errno: " << errno << ")" << std::endl;
        close(sockfd);
        throw std::runtime_error("Binding failed.");
    }
    */


    try {
        bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    } catch (const std::exception &e) {
        std::cerr << "Binding failed: " << e.what() << std::endl;
    }

    struct linger so_linger;
    so_linger.l_onoff = 1;
    so_linger.l_linger = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));
        



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

    while (audio_receive) {
        // Проверяем, не пришел ли сигнал завершения перед `accept()`
        if (!audio_receive) break;



        if (fcntl(sockfd, F_GETFD) == -1) {
            std::cerr << "Socket closed unexpectedly!" << std::endl;
        }

        


        // Принимаем подключение (неблокирующий режим)
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;  // Повторяем цикл
            } else {
                perror("Accept error");
                break;
            }
        }

        printf("receiving: Client connected\n");

        if (snd_pcm_prepare(playback_handle) < 0) {
            printf("Error preparing\n");
        }

        // Делаем клиентский сокет неблокирующим
        fcntl(newsockfd, F_SETFL, O_NONBLOCK);

        // Основной цикл для приёма и воспроизведения звуковых данных
        while (audio_receive) {
            int n = recv(newsockfd, buffer, BUFFER_SIZE, 0);

            // включаем флаг приема для индикатора
            signal_received = true;
            
            if (n < 0) {
                signal_received = false;
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                } else {
                    perror("Receive error");

                    
                    break;
                }
            } else if (n == 0) {
                std::cout << "Client disconnected" << std::endl;
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
                snd_pcm_recover(playback_handle, err, 0);
            }
            dataCapacity += n;

            // printf("\ndataCapacity: %ld\n\n", dataCapacity);
        }
        close(newsockfd);
        std::cout << "Connection closed" << std::endl;
    }

    // Освобождаем ресурсы
    std::cout << "Stopping audio receiver..." << std::endl;
    snd_pcm_drop(playback_handle);
    snd_pcm_close(playback_handle);
    close(sockfd);
    memset(buffer, 0, BUFFER_SIZE);
    std::cout << "Audio receiver stopped." << std::endl;
}