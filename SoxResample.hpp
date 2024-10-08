#ifndef SOXRESAMPLE_HPP
#define SOXRESAMPLE_HPP
#include <string>
#include <soxr.h>

#include <vector>

class SoxResample {
  private:
    soxr_t soxr_{nullptr};
    std::vector<short> inner_buff_v_;

  public:
    SoxResample() = delete;
    SoxResample(int in_sample, int out_sample, int channels);
    short* Process(const short* in_audio, int in_sample, int& out_sample);
    ~SoxResample();
};

#endif //SOXRESAMPLE_HPP
