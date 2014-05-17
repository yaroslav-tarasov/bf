#ifndef QWAITFORDONE_H
#define QWAITFORDONE_H

#include <QObject>
#include <QTimer>
#include <QEventLoop>

class QWaitForDone : public QObject
{
    Q_OBJECT
public:
    enum con_t {DISCONNECT_DONE,CONNECT_DONE};
    explicit QWaitForDone(QObject *obj = 0,con_t con = CONNECT_DONE);

signals:
    void done();
public slots:
    void start(int timeout);
    void restart(){timer.start(mInterval);}
    void quit();
private:
    QEventLoop wait_for_done;
    QTimer timer;
    int mInterval;
};

#endif // QWAITFORDONE_H
