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

    explicit BFControl(QObject *parent = 0);
    int  create();
    void close();
    int getRulesSync   (const filter_rule_t& pattern, QList<filter_rule_ptr >& ruleslst,int timeout_ms=3000);
    int getRulesAsync  (const filter_rule_t& pattern);
    int sendRulesSync  (const QList<filter_rule_ptr >& ruleslst );
    int deleteRule     (const filter_rule_t &pattern);
    int deleteRules    (const filter_rule_t &pattern);
    int addRule        (const filter_rule_t &pattern);
    int updateRule     (const filter_rule_t &pattern);
    int setChainPolicy (const filter_rule_t &pattern);
    int subscribeLog   (pid_t pid);

private:
    inline int  sendMsg(int type, const void *msg, size_t size);


signals:
    void data(filter_rule_t);
    void data(QByteArray);
    void data(QList <filter_rule_ptr > );
    void log (filter_rule_t);
    void done();
    void error (quint16);
    void error (msg_err_t);
public slots:
private slots:
    void process(QByteArray ba);
private:
    std::shared_ptr<struct BFControlPrivate> d;
};

#endif // BFCONTROL_H
