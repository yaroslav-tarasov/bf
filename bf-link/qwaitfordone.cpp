#include "qwaitfordone.h"

QWaitForDone::QWaitForDone(QObject *obj,con_t con)// :
    //QObject(obj)
{
    timer.setSingleShot(true);

    QObject::connect(&timer, SIGNAL(timeout()), &wait_for_done, SLOT(quit()));

    if(con == CONNECT_DONE)
         QObject::connect(obj, SIGNAL(done()), &wait_for_done, SLOT(quit()));
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

void QWaitForDone::quit()
{
    wait_for_done.quit();
}

