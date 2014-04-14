#ifndef NETLINK_SOCKET_H
#define NETLINK_SOCKET_H

#include <QObject>
#include <memory>
#include "bf-link_global.h"


namespace test
{


class NetlinkSocketPrivate;

class /*BFLINKSHARED_EXPORT*/ NetlinkSocket : public QObject
{
    Q_OBJECT
public:
    explicit NetlinkSocket(QObject *parent = 0);
    ~NetlinkSocket();
    int sendMsg(int type, void* msg, size_t size);
    int create(int proto);
protected:

signals:
    void error(QString err);
    void readyRead();

protected:
   std::shared_ptr<NetlinkSocketPrivate> d;
   int mProto;
   int mBuffSize;
};

}
#endif // NETLINK_SOCKET_H
