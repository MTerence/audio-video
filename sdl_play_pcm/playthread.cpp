#include "playthread.h"

#include <SDL2/SDL.h>
#include <QDebug>
#include <QFile>

#define FILENAME "/Users/kwai1/Documents/github/audio-video/sdl_play_pcm/in.pcm"
#define SAMPLE_RATE 44100
#define SAMPLE_FORMAT AUDIO_S16LSB
#define SAMPLE_SIZE SDL_AUDIO_BITSIZE(SAMPLE_FORMAT)
#define CHANLES 2
//音频缓冲区的样本数量
#define SAMPLES 1024
//每个样本占用多少字节
#define BYTES_PER_SAMPLE ((SAMPLE_SIZE * CHANLES) >> 3)
//文件缓冲区的大小
#define BUFFER_SIZE (SAMPLES * BYTES_PER_SAMPLE)

typedef struct {
    int len = 0; //从文件中每次读取的大小
    int pullLen = 0;
    Uint8 *data = nullptr;
} AudioBuffer;

PlayThread::PlayThread(QObject *parent)
    : QThread{parent}
{
    connect(this, &PlayThread::finished,
            this, &PlayThread::deleteLater);
}

PlayThread::~PlayThread() {
    disconnect();
    requestInterruption();
    quit();
    wait();

    qDebug() << this << "析构了";
}

//等待音频回调，会回调多次
void pull_audio_data(void *userdata,
                     //需要往stream中填充的PCM数据
                     Uint8 *stream,
                     //希望填充的大小(samples * format * channles / 8)
                     int len) {
    qDebug() << "-------00";
    //清空stream（静音处理）
    SDL_memset(stream, 0, len);

    qDebug() << "-------0";
    AudioBuffer *buffer = (AudioBuffer *) userdata;

    //文件数据还没准备好
    if (buffer->len <= 0) return;

    //取len， bufferLen的最小值（为了保证数据安全，防止指针越界）
    buffer->pullLen = (len > buffer->pullLen) ? buffer->len : len;

    SDL_MixAudio(stream,
                 buffer->data,
                 len,
                 SDL_MIX_MAXVOLUME);
    buffer->data += buffer->pullLen;
    buffer->len -= buffer->pullLen;

}

void PlayThread:: run() {
    //初始化SDL子系统
    if (SDL_Init(SDL_INIT_AUDIO)) {
        qDebug() << "SDL_Init error" << SDL_GetError();
        return;
    }

    SDL_AudioSpec spec;
    //采样率
    spec.freq = SAMPLE_RATE;
    //采样格式(s16le)
    spec.format = AUDIO_S16LSB;
    //声道数
    spec.channels = CHANLES;
    //音频缓冲区的样本数量(必须是2的幂)
    spec.samples = 1024;
    //回调
    spec.callback = pull_audio_data;
    AudioBuffer buffer;
    spec.userdata = &buffer;

    //打开设备
    if (SDL_OpenAudio(&spec, nullptr)) {
        qDebug() << "SDL_OpenAudio error" << SDL_GetError();
        //清除所有子系统
        SDL_Quit();
        return;
    }

    //打开文件
    QFile file(FILENAME);
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "file open error" << FILENAME;
        //关闭设备
        SDL_CloseAudio();
        //清除子系统
        SDL_Quit();
        return;
    }

    //开始播放
    SDL_PauseAudio(0);

    //存放从文件中读取的数据
    Uint8 data[BUFFER_SIZE];
    qDebug() << "-------1";
    while (!isInterruptionRequested()) {
        //只要从文件中读取的音频数据，还没有填充完毕，就跳过
        if (buffer.len > 0) continue;

        buffer.len = file.read((char *)data, BUFFER_SIZE);
        //文件已经读取完毕
        if (buffer.len <= 0) {
            int samples = buffer.pullLen / BYTES_PER_SAMPLE;
            int ms = samples * 1000 / SAMPLE_RATE;
            SDL_Delay(ms);
            break;
        };

        //读取到了文件数据
        buffer.data = data;
    }

    //关闭文件
    file.close();

    //关闭设备
    SDL_CloseAudio();

    //关闭子系统
    SDL_Quit();
}
