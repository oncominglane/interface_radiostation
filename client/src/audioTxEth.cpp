#include "TxRxEth.h"

void audioTxEth(unsigned char *buffer, std::atomic<bool> &audio_transmit) {
    // Параметры для захвата звука
    snd_pcm_t           *capture_handle;
    snd_pcm_hw_params_t *hw_params;

    // Создание сокета для передачи данных
    int                sockfd;
    struct sockaddr_in serv_addr;

    unsigned int      resample      = 1;
    unsigned int      sampleRate    = 44100;
    long int          dataCapacity  = 0;
    int               channels      = 2;
    snd_pcm_uframes_t local_buffer  = BUFFER_SIZE;
    snd_pcm_uframes_t local_periods = PERIODS;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
    }

    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_port        = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);  // IP

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
    }

    // Открываем PCM устройство
    if (snd_pcm_open(&capture_handle, "hw:0,0", SND_PCM_STREAM_CAPTURE, 0) < 0) {
        perror("Cannot open audio device");
        close(sockfd);
        return;
    }

    // Выделение памяти для hw_params с использованием malloc
    if (snd_pcm_hw_params_malloc(&hw_params) < 0) {
        perror("Cannot allocate hardware parameters");
        snd_pcm_close(capture_handle);
        close(sockfd);
        return;
    }

    if (snd_pcm_hw_params_any(capture_handle, hw_params) < 0) {
        perror("Cannot configure this PCM device");
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(capture_handle);
        close(sockfd);
        return;
    }

    snd_pcm_hw_params_get_buffer_size(hw_params, &local_buffer);
    snd_pcm_hw_params_get_period_size(hw_params, &local_periods, 0);

    printf("Buffer size: %lu, Period size: %lu\n", local_buffer, local_periods);

    if (snd_pcm_hw_params_set_format(capture_handle, hw_params, SND_PCM_FORMAT_S16_LE) < 0) {
        perror("Cannot set sample format");
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(capture_handle);
        close(sockfd);
        return;
    }

    if (snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
        perror("Cannot set access rate");
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(capture_handle);
        close(sockfd);
        return;
    }

    if (snd_pcm_hw_params_set_channels(capture_handle, hw_params, channels) < 0) {
        perror("Cannot set channel count");
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(capture_handle);
        close(sockfd);
        return;
    }

    if (snd_pcm_hw_params_set_rate_near(capture_handle, hw_params, &sampleRate, 0) < 0) {
        perror("Cannot set rate near");
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(capture_handle);
        close(sockfd);
        return;
    }

    if (snd_pcm_hw_params_set_rate_resample(capture_handle, hw_params, resample) < 0) {
        perror("Cannot set sample rate");
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(capture_handle);
        close(sockfd);
        return;
    }

    if (snd_pcm_hw_params_set_buffer_size_near(capture_handle, hw_params, &local_buffer) < 0) {
        perror("Cannot set buffer size near");
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(capture_handle);
        close(sockfd);
        return;
    }

    if (snd_pcm_hw_params_set_period_size_near(capture_handle, hw_params, &local_periods, 0) < 0) {
        perror("Cannot set period size near");
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(capture_handle);
        close(sockfd);
        return;
    }

    if (snd_pcm_hw_params(capture_handle, hw_params) < 0) {
        perror("Cannot set hardware parameters");
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(capture_handle);
        close(sockfd);
        return;
    }

    // Освобождение выделенной памяти
    snd_pcm_hw_params_free(hw_params);

    printf("Buffer size: %lu, Period size: %lu\n", local_buffer, local_periods);

    // Подготовка устройства к воспроизведению
    if (snd_pcm_prepare(capture_handle) < 0) {
        perror("Cannot prepare audio interface for use");
        snd_pcm_close(capture_handle);
        close(sockfd);
        return;
    }

    // Основной цикл для захвата и передачи данных
    while (audio_transmit) {
        // Захватываем аудиоданные

        int frames = snd_pcm_readi(capture_handle, buffer, BUFFER_SIZE / (channels * 2));

        if (frames < 0) {
            fprintf(stderr, "Read error: %s\n", snd_strerror(frames));  // Выводим точную ошибку ALSA
            snd_pcm_prepare(capture_handle);  // Попробуем восстановить поток
            continue;
        }

        // Передаем данные по сети
        // ssize_t bytes_sent = send(sockfd, buffer, frames * channels * 2, 0);
        ssize_t bytes_sent = send(sockfd, buffer, BUFFER_SIZE, 0);
        if (bytes_sent < 0) {
            perror("Send error");
            close(sockfd);
            snd_pcm_drop(capture_handle);
            snd_pcm_close(capture_handle);
            fprintf(stderr, "Connection lost. Reconnecting...\n");
            break;
        }

        dataCapacity += bytes_sent;
        // printf("\ndataCapacity: %ld\n\n", dataCapacity);
    }
    memset(buffer, 0, BUFFER_SIZE);
    snd_pcm_drop(capture_handle);
    snd_pcm_close(capture_handle);
    close(sockfd);
}