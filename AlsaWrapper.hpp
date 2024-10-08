#ifndef ALSATEST__ALSAWRAPPER_HPP_
#define ALSATEST__ALSAWRAPPER_HPP_

#include <alsa/asoundlib.h>

enum AlsaError
{
    OPEN_ERROR = 1,
    MALLOC_ERROR,
    ANY_ERROR,
    ACCESS_ERROR,
    FORMAT_ERROR,
    RATE_ERROR,
    CHANNELS_ERROR,
    PARAMS_ERROR,
    PREPARE_ERROR,
    FOPEN_ERROR,
    FCLOSE_ERROR,
    SNDREAD_ERROR,
    START_ERROR
};

class AlsaRecorder
{
public:
    AlsaRecorder() = default;
    AlsaRecorder(uint sample_rate, uint channel);
    ~AlsaRecorder();
    int Init(const char* device_name);
    int DoRecord(char* buff, int buff_size);
    int DeInit();
    uint GetCurrentSamplerate() const { return sample_rate_; };
    uint GetCurrentChannels() const { return channels_; };

private:
    bool inited_{false};
    bool started_{false};
    const char* snd_device_ = "hw:1,0\n";
    unsigned int sample_rate_{16000};
    unsigned int channels_{1};

    snd_pcm_t* capture_handle_{nullptr};
    snd_pcm_hw_params_t* hw_params_{nullptr};

    int InitSoundcard();
    int SetSoundcard();
    int StartSoundcard();
};

class AlsaPlayer
{
};

#endif //ALSATEST__ALSAWRAPPER_HPP_
