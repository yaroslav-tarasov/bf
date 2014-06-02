#include "qwaitfordone.h"

QWaitForDone::QWaitForDone(QObject *obj,con_t con) : reason (RES_NONE)
    //QObject(obj)
{
    timer.setSingleShot(true);

    QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(timeout()));

    if(con == CONNECT_DONE)
         QObject::connect(obj, SIGNAL(done()), this, SLOT(quit()));
}


void QWaitForDone::start(int timeout)
{
    mInterval = timeout;
    timer.start(timeout);
    wait_for_done.exec();

    if (timer.isActive()) {
          timer.stop();
    }
}

void QWaitForDone::timeout()
{
    wait_for_done.quit();
    reason = RES_TIMEOUT;
}

void QWaitForDone::quit()
{
    wait_for_done.quit();
    reason = RES_EVENTDONE;
}

