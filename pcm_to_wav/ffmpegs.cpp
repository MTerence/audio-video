#include "ffmpegs.h"
#include <QFile>
#include <QDebug>

FFmpegs::FFmpegs() {

}

void FFmpegs::pcm2wav(WAVHeader &header,
                      const char *pcmFilename,
                      const char *wavFilename) {
    header.blockAlign = header.bitsPerSample * header.numChannels >> 3;
    header.byteRate = header.sampleRate * header.blockAlign;

    // 打开PCM文件
    QFile pcmFile(pcmFilename);
    if (!pcmFile.open(QFile::ReadOnly)) {
        qDebug() << "文件打开失败" << pcmFilename;
        return;
    }
    header.dataChunkDataSize = pcmFile.size();
    header.riffChunkDataSize = header.dataChunkDataSize
                               + sizeof(WAVHeader)
                               - sizeof(header.riffChunkId)
                               - sizeof(header.riffChunkDataSize);
    // 打开WAV文件
    QFile wavFile(wavFilename);
    if (!wavFile.open(QFile::WriteOnly)) {
        qDebug() << "打开文件失败" << wavFilename;
        pcmFile.close();
        return;
    }

    //写入文件头部
    wavFile.write((const char*)&header, sizeof(header));

    // 写入PCM数据
    char buf[1024];
    int size;
    while ((size = pcmFile.read(buf, sizeof(buf))) > 0) {
        wavFile.write(buf, size);
    }
    //关闭文件
    pcmFile.close();
    wavFile.close();
}

