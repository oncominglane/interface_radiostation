#include "TxRx.h"
#include <alsa/pcm.h>

snd_pcm_t *setup_hw(const char *device, unsigned int channels, unsigned int rate, snd_pcm_uframes_t buffer_size,
                    snd_pcm_uframes_t period_size, int sockfd) {
    snd_pcm_t           *handle;
    snd_pcm_hw_params_t *hw_params;

    if (snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0) < 0) {
        perror("Cannot open audio device");
        close(sockfd);
        return nullptr;
    }

    // Выделение памяти для hw_params с использованием malloc
    if (snd_pcm_hw_params_malloc(&hw_params) < 0) {
        perror("Cannot allocate hardware parameters");
        snd_pcm_close(handle);
        close(sockfd);
        return nullptr;
    }

    if (snd_pcm_hw_params_any(handle, hw_params) < 0) {
        perror("Cannot configure hardware parameters on this PCM device");
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(handle);
        close(sockfd);
        return nullptr;
    }

    snd_pcm_hw_params_get_buffer_size(hw_params, &local_buffer);
    snd_pcm_hw_params_get_period_size(hw_params, &local_periods, 0);

    printf("Buffer size: %lu, Period size: %lu\n", local_buffer, local_periods);

    if (snd_pcm_hw_params_set_format(handle, hw_params, SND_PCM_FORMAT_S16_LE) < 0) {
        perror("Cannot set sample format");
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(handle);
        close(sockfd);
        return nullptr;
    }

    if (snd_pcm_hw_params_set_access(handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
        perror("Cannot set access rate");
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(handle);
        close(sockfd);
        return nullptr;
    }

    if (snd_pcm_hw_params_set_channels(handle, hw_params, channels) < 0) {
        perror("Cannot set channel count");
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(handle);
        close(sockfd);
        return nullptr;
    }

    if (snd_pcm_hw_params_set_rate_near(handle, hw_params, &sampleRate, 0) < 0) {
        perror("Cannot set rate near");
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(handle);
        close(sockfd);
        return nullptr;
    }

    if (snd_pcm_hw_params_set_rate_resample(handle, hw_params, resample) < 0) {
        perror("Cannot set sample rate");
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(handle);
        close(sockfd);
        return nullptr;
    }

    if (snd_pcm_hw_params_set_buffer_size_near(handle, hw_params, &local_buffer) < 0) {
        perror("Cannot set buffer size near");
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(handle);
        close(sockfd);
        return nullptr;
    }

    if (snd_pcm_hw_params_set_period_size_near(handle, hw_params, &local_periods, 0) < 0) {
        perror("Cannot set period size near");
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(handle);
        close(sockfd);
        return nullptr;
    }

    if (snd_pcm_hw_params(handle, hw_params) < 0) {
        perror("Cannot set hardware parameters");
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(handle);
        close(sockfd);
        return nullptr;
    }

    // Освобождение выделенной памяти
    snd_pcm_hw_params_free(hw_params);

    // snd_pcm_hw_params_get_buffer_size(hw_params, &local_buffer);
    // snd_pcm_hw_params_get_period_size(hw_params, &local_periods, 0);

    printf("Buffer size: %lu, Period size: %lu\n", local_buffer, local_periods);
    return handle;
}