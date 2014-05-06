#ifndef QWAITFORDONE_H
#define QWAITFORDONE_H

#include <QObject>
#include <QTimer>
#include <QEventLoop>

class QWaitForDone : public QObject
{
    Q_OBJECT
public:
    explicit QWaitForDone(QObject *obj = 0);

signals:
    void done();
public slots:
    void start(int timeout);
private:
    QEventLoop wait_for_done;
    QTimer timer;

};

#endif // QWAITFORDONE_H
