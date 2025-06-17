#ifndef PLAYER_HPP
#define PLAYER_HPP
#include <alsa/asoundlib.h>

#include <thread>

#include "AlsaWrapper.hpp"
#include "KRingBuff.hpp"

#endif  // PLAYER_HPP
class Player {
 private:
  int channels_{1};
  int samplerate_{22400};
  int frame_size_{1024};
  std::unique_ptr<AlsaPlayer> alsa_palyer_{nullptr};

 public:
  Player();
  ~Player();

  // 初始化
  bool InitPlayer(int samplerate = 16000, int channels = 1);
  // 设置回调函数
  int DoPlay(const short *buffer, int buffer_size);  // 返回值表示实际写入了多少
  // 停止播放
  void StopPlayer(bool immediate = true);
  // 释放资源
  void ReleasePlayer();
};
