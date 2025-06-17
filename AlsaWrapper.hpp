#ifndef ALSATEST__ALSAWRAPPER_HPP_
#define ALSATEST__ALSAWRAPPER_HPP_

#include <alsa/asoundlib.h>

#include <memory>

#include "KRingBuff.hpp"

enum AlsaError {
  NOT_INIT = -2,
  NOT_START = -1,
  SUCCESS = 0,
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
  START_ERROR,
  FRAME_ERROR,
  PLAY_ERROR,
};

class AlsaRecorder {
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
  int StartSoundcard();
};

class AlsaPlayer {
 public:
  AlsaPlayer() = default;
  AlsaPlayer(uint sample_rate, uint channel);
  ~AlsaPlayer();
  int Init(const char* device_name);
  int DoPlay(const short* buff, int buff_size);
  void StopPlay(bool immediate = false);
  int DeInit();

  uint GetCurrentSamplerate() const { return sample_rate_; };
  uint GetCurrentChannels() const { return channels_; };
  uint GetCurrentFrameSize() const { return frames_size_; };

 private:
  bool inited_{false};
  bool started_{false};
  const char* snd_device_ = "default";
  unsigned int sample_rate_{16000};
  unsigned int channels_{1};
  std::unique_ptr<KRingBuff<short>> inner_buff_ptr_{nullptr};
  std::unique_ptr<short[]> frame_buff_ptr_{nullptr};

  snd_pcm_t* playback_handle_{nullptr};
  snd_pcm_hw_params_t* hw_params_{nullptr};
  snd_pcm_uframes_t frames_size_ = 1024;                           // 一个周期多少帧
  snd_pcm_access_t access_mode_ = SND_PCM_ACCESS_RW_INTERLEAVED;  // 访问模式:交错访问
  snd_pcm_format_t format_ = SND_PCM_FORMAT_S16_LE;               // 采样位数:16位,小端存储

  int InitSoundcard();
  int StartSoundcard();
};

#endif  // ALSATEST__ALSAWRAPPER_HPP_
