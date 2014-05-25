#ifndef BFLOCALCONTROL_H
#define BFLOCALCONTROL_H

#include <QObject>
#include <QSharedPointer>
#include <QList>
#include <QLocalSocket>

#include <memory>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <linux/if_ether.h>
#include <linux/netlink.h>

#include "trx_data.h"
#include "bf-link_global.h"

class BFLINKSHARED_EXPORT BFLocalControl : public QObject
{
    Q_OBJECT
public:

    typedef QSharedPointer <filter_rule_t > filter_rule_ptr;


    explicit    BFLocalControl (QObject *parent = 0);
    void        init           (const QString& serverName);
signals:
    
public slots:

private slots:
    void onConnected();
    void onReadyRead();
    void onDisconnected();
    void onSocketError(QLocalSocket::LocalSocketError err);

private:
    QLocalSocket *mLocalSocket;
    int           mSocketWantedData;
};

#endif // BFLOCALCONTROL_H
