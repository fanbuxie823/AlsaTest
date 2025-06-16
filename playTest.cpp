#include <alsa/asoundlib.h>
#include <bits/this_thread_sleep.h>

#include <fstream>
#include <iosfwd>
#include <iostream>

snd_pcm_t *handle;
snd_pcm_hw_params_t *params;
snd_pcm_uframes_t frames = 1024;                               // 一个周期多少帧
snd_pcm_access_t access_mode = SND_PCM_ACCESS_RW_INTERLEAVED;  // 访问模式:交错访问
snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;               // 采样位数:16位,小端存储
int channel = 1;
unsigned int simple_rate = 16000;  // 采样率
int dir;                           // 设备采样率与输入采样的偏差

constexpr int size = 1024 * 2;  // 缓冲区大小
static char buffer[size]{0};    // 缓冲区数据
static char buffer_1[size]{0};  // 缓冲区数据

int main() {
  /* 1. 打开pcm设备 */
  int rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
  if (rc < 0) {
    printf("open device failed\n");
    return 0;
  }

  /* 分配一个硬件参数对象 */
  snd_pcm_hw_params_alloca(&params);
  snd_pcm_hw_params_any(handle, params); /* 使用默认值填充参数对象. */
  /* 设置硬件参数 */
  snd_pcm_hw_params_set_access(handle, params, access_mode);      /* 交错模式 Interleaved mode */
  int err = snd_pcm_hw_params_set_format(handle, params, format); /* 采样位数 Signed 16-bit little-endian format */
  if (err < 0) {
    std::cerr << "[ALSA] cannot set format " << "(" << snd_strerror(err) << ", " << err << ")"
              << "\n";
  }
  err = snd_pcm_hw_params_set_channels(handle, params, channel); /* 通道数 Two channels (stereo) */
  if (err < 0) {
    std::cerr << "[ALSA] cannot set channel " << "(" << snd_strerror(err) << ", " << err << ")"
              << "\n";
  }
  err = snd_pcm_hw_params_set_rate_near(handle, params, &simple_rate,
                                        &dir); /* 采样率 44100 bits/second sampling rate (CDsss quality) */
  if (err < 0) {
    std::cerr << "[ALSA] cannot set rate " << "(" << snd_strerror(err) << ", " << err << ")"
              << "\n";
  }
  err = snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);  // 设置一个周期的多少帧
  if (err < 0) {
    std::cerr << "[ALSA] cannot set period " << "(" << snd_strerror(err) << ", " << err << ")"
              << "\n";
  }
  rc = snd_pcm_hw_params(handle, params); /* 将设置好的参数写入驱动 */
  if (rc < 0) {
    printf("unable to set hw parameters: %s\n", snd_strerror(rc));
    return 0;
  }

  std::ifstream in_file("out_audio.pcm", std::ios::binary);

  std::ofstream out_file("out_audio_test.pcm", std::ios::binary);

  do {
    in_file.read(buffer, size);
    if (in_file.eof()) {
      printf("end of file\n");
      break;
    }
    rc = snd_pcm_writei(handle, buffer, frames); // 阻塞的接口
    if (rc > 0 && rc < frames) {
      printf("short write\n");
    } else if (rc == -EPIPE) {
      printf("underrun occurred\n");
      snd_pcm_prepare(handle);
    } else if (rc < 0) {
      printf("error from writei: %s\n", snd_strerror(rc));
      break;
    }
  } while (1);

  snd_pcm_drain(handle); // 清空缓冲区
  // std::this_thread::sleep_for(std::chrono::seconds(2));
  // snd_pcm_drop(handle); // 丢弃缓冲区
  // std::cout << "droped" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(2));
  snd_pcm_close(handle);
}