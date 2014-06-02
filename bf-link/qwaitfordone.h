#ifndef QWAITFORDONE_H
#define QWAITFORDONE_H

#include <QObject>
#include <QTimer>
#include <QEventLoop>

class QWaitForDone : public QObject
{
    Q_OBJECT
public:
    enum reason_t {RES_NONE,RES_TIMEOUT,RES_EVENTDONE};
    enum con_t    {DISCONNECT_DONE,CONNECT_DONE};
    explicit QWaitForDone(QObject *obj = 0,con_t con = CONNECT_DONE);
    reason_t getReason() {return reason;}
signals:
    void done();
public slots:
    void start(int timeout);
    void restart(){timer.start(mInterval);}
    void quit();
    void timeout();
private:
    QEventLoop wait_for_done;
    QTimer     timer;
    int        mInterval;
    reason_t   reason;
};

#endif // QWAITFORDONE_H
