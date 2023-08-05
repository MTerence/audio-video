#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    if (!_audioThread) { //点击了开始录制
        _audioThread = new AudioThread(this);
        _audioThread->start();

        connect(_audioThread, &AudioThread::finished, [this]() {
            _audioThread = nullptr;
            ui->pushButton->setText("开始录音");
        });

        ui->pushButton->setText("结束录音");
    } else { //点击了结束录制
        qDebug() << "点击了结束录制";
        _audioThread->setStop(true);
        _audioThread->requestInterruption();
        _audioThread = nullptr;
        ui->pushButton->setText("开始录音");
    }
}

