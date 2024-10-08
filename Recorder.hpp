#ifndef RECORDER_H
#define RECORDER_H
#include <atomic>
#include <functional>
#include <memory>
#include <thread>

#include "SoxResample.hpp"
#include "AlsaWrapper.hpp"

using RecorderCallBackFuncType = std::function<void(short* data,int data_size)>;

class Recorder
{
private:
    std::atomic<bool> is_recording_{false};
    bool need_merge_{false};
    bool need_resample_{false};
    int current_samplerate_{0};
    int current_channel_{0};
    std::unique_ptr<SoxResample> sox_resample_{nullptr};
    std::unique_ptr<AlsaRecorder> alsa_recorder_{nullptr};
    RecorderCallBackFuncType call_back_func_{nullptr};
    std::thread recorder_thread_;

    int MergeChannels(short* buff, int buff_size);
    void DoRecord();

public:
    Recorder() = default;
    ~Recorder();
    //初始化
    bool InitRecorder(int samplerate = 16000, int channels = 1);
    //设置回调函数
    bool SetCallBackFunc(const RecorderCallBackFuncType& func);

    //开始录音
    bool StartRecord();
    //停止录音
    void StopRecord();
    //释放资源
    void ReleaseRecorder();
};


#endif //RECORDER_H
