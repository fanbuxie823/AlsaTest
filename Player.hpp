#ifndef PLAYER_HPP
#define PLAYER_HPP
#include <thread>

using PlayerCallBackFuncType = std::function<void(short* data, int data_size)>;

class Player {
 private:
  PlayerCallBackFuncType call_back_func_{nullptr};
  std::thread player_thread_;

  void DoPlay();

 public:
  // Default
  Player() = default;
  ~Player();

  // 初始化
  bool InitPlayer(int samplerate = 16000, int channels = 1);
  // 设置回调函数
  bool SetCallBackFunc(const PlayerCallBackFuncType& func) { call_back_func_ = func; }
  // 开始播放
  bool StartPlayer();
  // 停止播放
  void StopPlayer(bool immediate = true);
  // 释放资源
  void ReleasePlayer();
};

#endif  // PLAYER_HPP
