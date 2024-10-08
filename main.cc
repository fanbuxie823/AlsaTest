#include <fstream>
#include <iostream>
#include "Recorder.hpp"

int main(int argc, char* argv[]) {
  Recorder recorder;
  //初始化
  recorder.InitRecorder();

  //设置回调函数
  std::ofstream out_audio("out_audio.pcm", std::ios::binary);
  recorder.SetCallBackFunc([&out_audio](short* data, int data_size) {
    // std::cout << "[MAIN] callback for write:" << data_size << std::endl;
    out_audio.write(reinterpret_cast<char*>(data), data_size * sizeof(short));
  });

  //开始
  recorder.StartRecord();

  //等待结束
  std::cout << "press q to exit" << std::endl;
  while (std::cin.get() != 'q'){
    std::cout << "press q to exit" << std::endl;
  }
  recorder.StopRecord();
  recorder.ReleaseRecorder();
}