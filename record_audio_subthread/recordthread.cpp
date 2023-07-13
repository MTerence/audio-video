#include "recordthread.h"

#include <QDebug>
#include <QFile>
#include <QDateTime>

extern "C" {
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
}

#ifdef Q_OS_WIN
    #define FMT_NAME 'dshow
    #define DEVICE_NAME "audio=线路输入 (3- 魅声T800)"
    #define FILEPATH "F:/"
#else
    #define FMT_NAME "avfoundation"
    #define DEVICE_NAME ":1"
    #define FILEPATH "/Users/kwai1/Desktop"
#endif

RecordThread::RecordThread(QObject *parent) : QThread(parent) {
    connect(this, &RecordThread::finished,
            this, &RecordThread::deleteLater);
}

RecordThread::~RecordThread() {
    //断开所有链接
    disconnect();
    //内存回收之前， 正常结束线程
    requestInterruption();
    quit();
    wait();
    qDebug() << this << "析构(内存被回收)";
}

void showSpec(AVFormatContext *ctx) {
    //获取输入流
    AVStream *stream = ctx->streams[0];
    //获取音频参数
    AVCodecParameters *params = stream->codecpar;
    //声道数
    qDebug() << params->ch_layout.nb_channels;
    //采样率
    qDebug() << params->sample_rate;
    //采样格式
    qDebug() << params->format;
    //每一个样本的一个声道占用多少个字节
    qDebug() << av_get_bytes_per_sample((AVSampleFormat) params->format);

}

void RecordThread::run() {
    qDebug() << this << "开始执行---";

    //获取输入格式对象
    const AVInputFormat *fmt = av_find_input_format(FMT_NAME);
    if (!fmt) {
        qDebug() << "获取输入格式对象失败" <<FMT_NAME;
        return;
    }

    //格式上下文
    AVFormatContext *ctx = nullptr;
    int ret = avformat_open_input(&ctx, DEVICE_NAME, fmt, nullptr);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof(errbuf));
        qDebug() << "打开设备失败" <<errbuf;
        return;
    }

    //打印录音设备的信息
    showSpec(ctx);

    //文件名
    QString filename = FILEPATH;

    filename += QDateTime::currentDateTime().toString("MM_dd_HH_mm_ss");
    filename += ".pcm";
    QFile file(filename);

    //打开文件
    //WriteOnly：只写模式，如果文件不存在，就创建文件，如果文件存在，就会清空文件内容
    if (!file.open(QFile::WriteOnly)) {
        qDebug() << "文件打开失败" << filename;

        //关闭设备
        avformat_close_input(&ctx);
        return;
    }

    //数据包
    AVPacket *pkt = av_packet_alloc();
    while (!isInterruptionRequested()) {
        //不断采集
        ret = av_read_frame(ctx, pkt);
        if (ret == 0) { //读取成功
            file.write((const char *) pkt->data, pkt->size);
        } else if (ret == AVERROR(EAGAIN)) { //资源临时不可用
            continue;
        } else { //其他错误
            char errbuf[1024];
            av_strerror(ret, errbuf, sizeof(errbuf));
            qDebug() << "av_read_frame_error" << errbuf << ret;
            break;
        }

        //释放pkt内的资源，必须要加
        av_packet_unref(pkt);
    }

    //关闭文件
    file.close();
    //释放资源
    av_packet_free(&pkt);
    //关闭设备
    avformat_close_input(&ctx);

    qDebug() << this << "正常结束-------";
}

void RecordThread::setStop(bool stop) {
    _stop = stop;
}
