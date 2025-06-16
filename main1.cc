#include <iostream>
#include <memory>
#include <fstream>
#include "AlsaWrapper.hpp"

constexpr const ulong MAX_RECORD_TIME = 5;
constexpr const ulong SAMPLE_RATE = 44100;
constexpr const ulong CHANNELS = 2;
constexpr const ulong MAX_RECORD_SAMPLES = SAMPLE_RATE * MAX_RECORD_TIME;
constexpr const ulong BUFFER_SIZE = MAX_RECORD_SAMPLES * sizeof(short) * CHANNELS;

using namespace std;

int record(char **buff, int buff_size) {
//  AlsaRecorder recoder(44100, 2);
  AlsaRecorder recoder(16000, 1);
  do {
	if (recoder.Init("hw:1,0") != 0)
	  break;
	recoder.DoRecord(*buff, buff_size);
  } while (false);
  return recoder.DeInit();
}

void play(const char *buff, int buff_size) {};

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

  out_file.write(buff,BUFFER_SIZE);
  out_file.close();

  std::cout << "start play" << std::endl;
  std::cout << "playing..." << std::endl;
//  play(std::move(buff));
  std::cout << "play done" << std::endl;
  return 0;
}
