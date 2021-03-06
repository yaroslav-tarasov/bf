#ifndef NETLINKSOCKET_H
#define NETLINKSOCKET_H

#include <QObject>
#include <memory>
#include "bf-link_global.h"

class NetlinkSocketPrivate;

class /*BFLINKSHARED_EXPORT*/ NetlinkSocket : public QObject
{
    Q_OBJECT
public:
    explicit    NetlinkSocket(QObject *parent = 0);
    virtual     ~NetlinkSocket();
    int         create  (int proto);
    int         sendMsg (int type,void* msg,size_t size);
    void        close   ();
protected:
#ifdef NL_THREAD
    static void threadStart(NetlinkSocket *p);
    void        runListener();
#endif
public slots:
    void        readyRead();
signals:
    void        finished ();
    void        error    (QString err);
    void        data     (QByteArray ba);

private:
   std::shared_ptr<NetlinkSocketPrivate> d;
   Q_DISABLE_COPY(NetlinkSocket)
};

#endif // NETLINKSOCKET_H
