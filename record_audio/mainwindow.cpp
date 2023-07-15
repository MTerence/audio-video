#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "QThread"
#include "QFile"
#include <QThread>

extern "C" {
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

#ifdef Q_OS_WIN
    #define FMT_NAME 'dshow
    #define DEVICE_NAME "audio=线路输入 (3- 魅声T800)"
    #define FILENAME "F:/out.pcm"
#else
    #define FMT_NAME "avfoundation"
    #define DEVICE_NAME ":2"
    #define FILENAME "/Users/kwai1/Desktop/out.pcm"
#endif

void logThread() {
    qDebug() << QThread::currentThread();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug() << "\nMainWIndow";
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_recordButton_clicked()
{
    qDebug() << "开始录音" << QThread::currentThread();

    // 获取输入格式对象
     const AVInputFormat *fmt = av_find_input_format(FMT_NAME);
    if (!fmt) {
        qDebug() << "获取输入格式对象失败" << FMT_NAME;
        return;
    }

    // 格式上下文
    AVFormatContext *ctx = nullptr;
    int ret = avformat_open_input(&ctx, DEVICE_NAME, fmt, nullptr);
    if (ret < 0) {
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof(errbuf));
        qDebug() << "打开设备失败" << errbuf;
        return;
    }
    // 文件名
    QFile file(FILENAME);

    // 打开文件
    if (!file.open(QFile::WriteOnly)) {
        qDebug() << "文件打开失败" << FILENAME;
        // 关闭设备
        avformat_close_input(&ctx);
        return;
    }
    qDebug() << "----------2";
    QThread::msleep(5000);
    // 采集次数
    int count = 100;
    AVPacket *pkt = av_packet_alloc();
    while (count > 0) {
        ret = av_read_frame(ctx, pkt);
        qDebug() << "count=" << count << "ret=" << ret;
        if ( ret == 0) {
            count --;
            qDebug() << "=========count" << count;
            // 数据写入文件
            file.write((const char *) pkt->data, pkt->size);
        } else if (ret == AVERROR(EAGAIN)) {
            qDebug() << "av_read_frame 资源不可用";
            continue;
        } else {
            char errbuf[1024];
            av_strerror(ret, errbuf, sizeof(errbuf));
            qDebug() << "av_read_frame error" << errbuf << ret;
            break;
        }
    }

    qDebug() << "----------3";
    //关闭文件
    file.close();

    // 关闭设备
    avformat_close_input(&ctx);
}

