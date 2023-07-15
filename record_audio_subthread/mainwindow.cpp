#include "mainwindow.h"
#include "ui_mainwindow.h"

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


void MainWindow::on_audioButton_clicked()
{
    if (!_recordThread) { //点击了开始录制
        _recordThread = new RecordThread(this);
        _recordThread->start();

        connect(_recordThread, &RecordThread::finished, [this]() {
            _recordThread = nullptr;
            ui->audioButton->setText("开始录音");
        });

        ui->audioButton->setText("结束录音");
    } else { //点击了结束录制
        _recordThread->requestInterruption();
        _recordThread = nullptr;
        ui->audioButton->setText("开始录音");
    }
}

