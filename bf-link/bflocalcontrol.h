#ifndef BFLOCALCONTROL_H
#define BFLOCALCONTROL_H

#include <QObject>
#include <QSharedPointer>
#include <QList>

//#include <arpa/inet.h>
//#include <netinet/ip.h>
//#include <linux/if_ether.h>
//#include <linux/netlink.h>

//#include "trx_data.h"
#include "bf-link_global.h"
#include "bf_defs.h"
#include "bfcontrolbase.h"

class BFLINKSHARED_EXPORT BFLocalControl : public BFControlBase
{
    Q_OBJECT
public:

    explicit    BFLocalControl (QObject *parent = 0);
    int         init           (init_params_t p = init_params_t(QString(BARRIER_BF_LOCAL_SOCK)));
    void        close();
    int         getRulesSync   (const filter_rule_t& pattern, QList<filter_rule_ptr >& ruleslst,int timeout_ms=3000);
    int         getRulesAsync  (const filter_rule_t& pattern);
    int         sendRulesSync  (const QList<filter_rule_ptr >& ruleslst );
    int         deleteRule     (const filter_rule_t &pattern);
    int         deleteRules    (const filter_rule_t &pattern);
    int         addRule        (const filter_rule_t &pattern);
    int         updateRule     (const filter_rule_t &pattern);
    int         setChainPolicy (const filter_rule_t &pattern);
    int         subscribeLog   (/*pid_t pid*/);

private:
    class BFLocalControlPrivate;
    BFLocalControlPrivate*  d;
};

#endif // BFLOCALCONTROL_H
