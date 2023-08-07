#include "audiothread.h"
#include <QDebug>
#include <ffmpegs.h>

AudioThread::AudioThread(QObject *parent)
    : QThread{parent} {
    //监听当线程结束时，就调用deleteLater回收
    connect(this, &AudioThread::finished,
            this, &AudioThread::deleteLater);
}

AudioThread::~AudioThread() {
    // 断开所有连接
    disconnect();
    requestInterruption();
    // 安全退出
    quit();
    wait();
    qDebug() << this << "析构（内存被回收）";
}

void AudioThread:: run() {
    ResampleAudioSpec ras1;
    ras1.filename = "/Users/kwai1/Documents/github/audio-video/audio_resample/in.pcm";
    ras1.sampleFmt = AV_SAMPLE_FMT_S16;
    ras1.sampleRate = 44100;
    ras1.chLayout = AV_CH_LAYOUT_STEREO;

    ResampleAudioSpec ras2;
    ras2.filename = "/Users/kwai1/Documents/github/audio-video/audio_resample/out.pcm";
    ras2.sampleFmt = AV_SAMPLE_FMT_S32;
    ras2.sampleRate = 44800;
    ras2.chLayout = AV_CH_LAYOUT_MONO;

    FFmpegs::resampleAudio(ras1, ras2);
}
