#ifndef TIMERPROXY_H
#define TIMERPROXY_H

#include <QTimer>

class TimerProxy : public QTimer
{
    Q_OBJECT
public:
    explicit TimerProxy(int interval):mInterval(interval){}
public slots:
     void restart(){start(mInterval);}
     void start(int ms){QTimer::start(ms);}
     void start(){QTimer::start();}
private:
     int mInterval;
};
#endif // TIMERPROXY_H
