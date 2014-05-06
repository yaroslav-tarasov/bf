#include "qwaitfordone.h"

QWaitForDone::QWaitForDone(QObject *obj)// :
    //QObject(obj)
{
    timer.setSingleShot(true);

    QObject::connect(&timer, SIGNAL(timeout()), &wait_for_done, SLOT(quit()));
    QObject::connect(obj, SIGNAL(done()), &wait_for_done, SLOT(quit()));
}


void QWaitForDone::start(int timeout)
{

    timer.start(timeout);
    wait_for_done.exec();

    if (timer.isActive()) {
          timer.stop();
    }
}

