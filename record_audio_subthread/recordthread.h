#ifndef RECORDTHREAD_H
#define RECORDTHREAD_H

#include <QThread>

class RecordThread : public QThread {
    Q_OBJECT
private:
    void run();
    bool _stop = false;
public:
    explicit RecordThread(QObject *parent = nullptr);
    ~RecordThread();
    void setStop(bool stop);
};

#endif // RECORDTHREAD_H
