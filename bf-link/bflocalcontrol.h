#ifndef BFLOCALCONTROL_H
#define BFLOCALCONTROL_H

#include <QObject>
#include <QSharedPointer>
#include <QList>
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
    explicit BFLocalControl(QObject *parent = 0);
    typedef QSharedPointer <filter_rule_t > filter_rule_ptr;

signals:
    
public slots:
    
};

#endif // BFLOCALCONTROL_H
