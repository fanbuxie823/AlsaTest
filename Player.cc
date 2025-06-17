#include "Player.hpp"

#include <iostream>

constexpr int kInnerBufferSize = 1024 * 8;

// 持续播放，分为两种状态
// 如果当前缓冲区的空闲不足一帧，则本次轮空
// 如果当前缓冲区的空闲大于等于一帧，则掉回调函数取一帧音频

Player::Player() {
  // inner_buff_ptr_ = std::make_unique<KRingBuff<short>>(frame_size_ * channels_ * 4);
}

Player::~Player() { alsa_palyer_ = nullptr; }

bool Player::InitPlayer(int samplerate, int channels) {
  alsa_palyer_ = std::make_unique<AlsaPlayer>(samplerate, channels);
  frame_size_ = alsa_palyer_->GetCurrentFrameSize();
  return alsa_palyer_->Init(nullptr) == 0;
}
int Player::DoPlay(const short* buffer, int buffer_size) {
  return alsa_palyer_->DoPlay(buffer, buffer_size);
}

void Player::StopPlayer(bool immediate) {
  // is_playing_ = false;
  // if (play_thread_.joinable()) {
  //   play_thread_.join();
  // }
  alsa_palyer_->StopPlay(immediate);
}

void Player::ReleasePlayer() {
  StopPlayer(true);
  alsa_palyer_->DeInit();
}

#ifdef PLAYER_TEST
#include <fstream>
constexpr int size = 1024 * 2;  // 缓冲区大小
static char buffer[size]{0};    // 缓冲区数据

int main(int argc, char* argv[]) {
  std::ifstream in_file("out_audio.pcm", std::ios::binary);
  Player player;
  if (!player.InitPlayer(22050, 1)) {
    std::cerr << "init player failed\n";
    return -1;
  }

  int read_len = 0, write_len = 0;
  do {
    in_file.read(buffer, size);
    read_len = in_file.gcount();  // 本次读取的数据
    if (in_file.eof()) {
      printf("end of file\n");
      break;
    }
    write_len = player.DoPlay(reinterpret_cast<short*>(buffer), read_len / 2) * 2;
    while (write_len > 0 && write_len != read_len) {
      std::cout << "buffer is full,will try write again" << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      read_len = read_len - write_len;
      write_len = player.DoPlay(reinterpret_cast<short*>(buffer + write_len), read_len / 2) * 2;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  } while (true);
  player.StopPlayer(false);
  player.ReleasePlayer();
  return 0;
}

#endif