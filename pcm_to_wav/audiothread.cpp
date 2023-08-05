#include "audiothread.h"

#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <ffmpegs.h>

extern "C" {
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

#ifdef Q_OS_WIN
    // 格式名称
    #define FMT_NAME "show"
    // 设备名称
    #define DEVICE_NAME "audio=线路输入 (3- 魅声T800)"
    // PCM文件名
    #define FILEPATH "F:/"
#else
    #define FMT_NAME "avfoundation"
    #define DEVICE_NAME ":1"
    #define FILEPATH "/Users/kwai1/Desktop/"
#endif

AudioThread::AudioThread(QObject *parent)
    : QThread{parent} {
    connect(this, &AudioThread::finished,
            this, &AudioThread::deleteLater);
}

AudioThread::~AudioThread() {
    // 断开所有连接
    disconnect();
    // 内存回收之前，正常结束线程
    requestInterruption();
    // 安全退出
    quit();
    wait();
    qDebug() << this << "析构（内存被回收）";
}

void showSpec(AVFormatContext *ctx) {
    // 获取输入流
    AVStream *stream = ctx->streams[0];
    // 获取音频参数
    AVCodecParameters *params = stream->codecpar;

    // 声道数
    qDebug() << params->ch_layout.nb_channels;
    // 采样率
    qDebug() << params->sample_rate;
    // 采样格式
    qDebug() << params->format;
    // 每一个样本的每一个声道占用多少个字节
    qDebug() << av_get_bytes_per_sample((AVSampleFormat)params->format);
    // 编码ID（可以看出采样格式）
    qDebug() << params->codec_id;
    // 每一个样本的一个声道占用多少位
    qDebug() << av_get_bits_per_sample(params->codec_id);
}

// 当线程启动的时候，就自动调用run函数
// run函数中的代码是在子线程中执行的
// 耗时操作应该放在run函数中
void AudioThread:: run() {
    qDebug() << this << "开始执行---------";

    //获取输入格式对象
    const AVInputFormat *fmt = av_find_input_format(FMT_NAME);
    if (!fmt) {
        qDebug() << "获取输入格式对象失败" <<FMT_NAME;
        return;
    }

    // 格式上下文(将来可以利用上下文操作设备)
    AVFormatContext *ctx = nullptr;
    //打开设备
    int ret = avformat_open_input(&ctx, DEVICE_NAME, fmt, nullptr);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof(errbuf));
        qDebug() << "打开设备失败" << errbuf;
         return;
    }
    // 打印录音设备的信息参数
    showSpec(ctx);

    // 文件名
    QString filename = FILEPATH;
    filename += QDateTime::currentDateTime().toString("MM_dd_HH_mm_ss");
    QString pcmFilename = filename + ".pcm";
    QString wavFilename = filename + ".wav";
    QFile file(pcmFilename);


    // 打开文件
    // WriteOnly 只写模式，如果文件不存在，就创建文件，如果文件存在，就清除文件内容
    if(!file.open(QFile::WriteOnly)) {
         qDebug() << "文件打开失败" << pcmFilename;
         //关闭设备
         avformat_close_input(&ctx);
         return;
    }

    // 数据包
    AVPacket pkt;
    while (!isInterruptionRequested()) {
         qDebug() << "!isInterruptionRequested";
         //不断采集数据,存入pkt中
         ret = av_read_frame(ctx, &pkt);

         if (ret == 0) { // 读取成功, 写入到文件中
             file.write((const char *)pkt.data, pkt.size);
         } else if (ret == AVERROR(EAGAIN)) { // error = -35, 资源不可用,跳过当前资源
             //qDebug() << "资源不可用";
             continue;
         } else {
             char errbuf[1024];
             av_strerror(ret, errbuf, sizeof(errbuf));
             qDebug() << "av_read_frame error" << errbuf << ret;
             break;
         }
    }

    // 释放资源
    // 关闭文件
    file.close();

    // 获取输入流
    AVStream *stream = ctx->streams[0];
    // 获取音频参数
    AVCodecParameters *params = stream->codecpar;

    // pcm转wav文件
    WAVHeader header;
    header.sampleRate = params->sample_rate;
    header.bitsPerSample = av_get_bits_per_sample(params->codec_id);
    header.numChannels = params->channels;
    if (params->codec_id >= AV_CODEC_ID_PCM_F32BE) {
         header.audioFormat = AUDIO_FORMAT_FLOAT;
    }
    FFmpegs::pcm2wav(header,
                     pcmFilename.toUtf8().data(),
                     wavFilename.toUtf8().data());
    // 关闭设备
    avformat_close_input(&ctx);
    qDebug() << this << "正常结束---------";
}

void AudioThread:: setStop(bool stop) {
    _stop = stop;
}
