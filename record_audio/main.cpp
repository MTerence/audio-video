#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <SDL2/SDL.h>
#include <QThread>
#include <iostream>

SDL_version v;
//SDL_VERSION(&x);

extern "C" {

#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>

}

void log() {
    qDebug() << "log begin ---";
    printf("log--------");
    printf("printf----");

    std::cout << "std::count------";

    av_log_set_level(AV_LOG_ERROR);

    av_log(nullptr, AV_LOG_ERROR, "AV_LOG_ERROR---");
    av_log(nullptr, AV_LOG_WARNING, "AV_LOG_WARNING----");
    av_log(nullptr, AV_LOG_INFO, "AV_LOG_INFO----");

    //刷新标准输出流
    fflush(stdout);
    fflush(stderr);

    qDebug() << "log end ---";
}

int main(int argc, char *argv[])
{

    qDebug() << "main" << QThread::currentThread();
    qDebug() << "FFMpeg version = " << av_version_info();
    qDebug() << "SDL version = " << v.major << v.minor << v.patch;

    //注册设备
    avdevice_register_all();
    log();
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
