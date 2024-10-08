#include "SoxResample.hpp"
#include <array>
#include <cassert>
#include <iostream>

SoxResample::SoxResample(int in_sample, int out_sample, int channels) {
  soxr_error_t error;
  struct soxr_io_spec spec;
  spec.itype = SOXR_INT16_I;
  spec.otype = SOXR_INT16_I;
  spec.scale = 1;
  spec.flags = 0;
  soxr_ = soxr_create(in_sample, out_sample, channels, &error, &spec, nullptr, nullptr);
  if (error)
    soxr_ = nullptr;
}

short* SoxResample::Process(const short* in_audio, const int in_sample, int& out_sample) {
  size_t out_done = 0, in_done = 0, total_in_done = 0;
  out_sample = 0;
  std::array<short, 1024> out_buf{0};
  const short* p_in_audio = in_audio;
  inner_buff_v_.clear();
  while (true) {
    out_done = 0;
    in_done = 0;
    soxr_process(soxr_, p_in_audio, in_sample, &in_done, out_buf.data(), out_buf.size(), &out_done);
    // std::cout << "[SOXR] out_done:" << out_done << std::endl;
    p_in_audio += in_done;
    total_in_done += in_done;
    out_sample += out_done;
    //保存导内部缓冲区中
    inner_buff_v_.insert(std::end(inner_buff_v_), out_buf.begin(), out_buf.begin() + out_done);
    //重置缓冲
    out_buf.fill(0);
    if (total_in_done >= in_sample) {
      break;
    }
  }
  // std::cout << "[SOXR] process done" << std::endl;
  return inner_buff_v_.data();
}

SoxResample::~SoxResample() {
  if (soxr_) {
    soxr_delete(soxr_);
    soxr_ = nullptr;
  }
}