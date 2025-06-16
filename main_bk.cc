#include <iostream>
#include <memory>
#include <fstream>
#include "AlsaWrapper.hpp"
#include "SoxResample.hpp"
#include "Recorder.hpp"

constexpr const ulong MAX_RECORD_TIME = 5;
constexpr const ulong SAMPLE_RATE = 44100;
constexpr const ulong CHANNELS = 2;
constexpr const ulong MAX_RECORD_SAMPLES = SAMPLE_RATE * MAX_RECORD_TIME;
constexpr const ulong BUFFER_SIZE = MAX_RECORD_SAMPLES * sizeof(short) * CHANNELS;

using namespace std;

static int MergeChannels(short* buff, int buff_size) {
  if (buff == nullptr) {
    return 0;
  }
  short* p_buff = buff;
  for (int i = 0; i < buff_size; i += 2) {
    p_buff[i >> 1] = p_buff[i] + p_buff[i + 1] >> 1;
  }
  return buff_size >> 1;
}

int record(char** buff, int buff_size) {
  AlsaRecorder recoder(16000, 1);
  std::unique_ptr<SoxResample> sox_resample;

  //如果是双通道的话
  bool need_merge = recoder.GetCurrentChannels() == 2;
  bool need_resample = recoder.GetCurrentSamplerate() == 16000;
  if (need_resample) {
    sox_resample = std::make_unique<SoxResample>(recoder.GetCurrentSamplerate(), 16000, 1);
  }
  do {
    if (recoder.Init("hw:1,0") != 0)
      break;
    recoder.DoRecord(*buff, buff_size);
  } while (false);
  return recoder.DeInit();
}

bool InitRecorder(int samplerate = 16000, int channels = 1) {
}

void play(const char* buff, int buff_size) {
};

int main() {
  int ret = 0;
  auto buff = new char[BUFFER_SIZE]{0};
  std::cout << "start record" << std::endl;
  std::cout << "recording..." << std::endl;
  ret = record(&buff, BUFFER_SIZE);
  if (ret != 0) {
    std::cerr << "error happened in record" << endl;
    return -1;
  }
  std::cout << "record done" << std::endl;

  ofstream out_file("1.pcm", ios::binary);
  if (!out_file.is_open()) {
    std::cerr << "file dump failed" << std::endl;
    return -1;
  }

  out_file.write(buff, BUFFER_SIZE);
  out_file.close();

  std::cout << "start play" << std::endl;
  std::cout << "playing..." << std::endl;
  // play(buff, BUFFER_SIZE);
  std::cout << "play done" << std::endl;
  return 0;
}