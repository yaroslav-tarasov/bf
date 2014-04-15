#ifndef BFCONTROL_H
#define BFCONTROL_H

#include <QObject>
#include <QSharedPointer>
#include <QList>
#include <memory>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <linux/if_ether.h>
#include <linux/netlink.h>
#include "trx_data.h"

struct BFControlPrivate;

class BFControl : public QObject
{

    Q_OBJECT
public:
    typedef QSharedPointer <filter_rule_t > filter_rule_ptr;

    explicit BFControl(QObject *parent = 0);
    int  create();
    int  sendMsg(int type,void* msg,size_t size);
    void close();
    int getRulesSync(filter_rule_t& pattern, QList<filter_rule_ptr >& ruleslst,int timeout_ms=3000);
    int getRulesAsync(filter_rule_t& pattern);
    int deleteRule(filter_rule_t &pattern);

    signals:
    void data(QByteArray ba);
    void data(QList <filter_rule_ptr > );
    void done();
public slots:
private slots:
    void process(QByteArray ba);
#ifdef TEST_ASYNC_GET_RULES
    void process(QList <filter_rule_ptr > ruleslst);
#endif
private:
    std::shared_ptr<struct BFControlPrivate> d;
};

#endif // BFCONTROL_H
