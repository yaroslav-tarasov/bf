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
#include "bf-link_global.h"

struct BFControlPrivate;

class BFLINKSHARED_EXPORT BFControl : public QObject
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
    int sendRulesSync(QList<filter_rule_ptr >& ruleslst );
    int deleteRule(filter_rule_t &pattern);
    int deleteRules(filter_rule_t &pattern);
    int addRule(filter_rule_t &pattern);
    int updateRule(filter_rule_t &pattern);
    int setChainPolicy(filter_rule_t &pattern);
    int subscribeLog(pid_t pid);

signals:
    void data(QByteArray ba);
    void data(QList <filter_rule_ptr > );
    void log (filter_rule_t);
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
