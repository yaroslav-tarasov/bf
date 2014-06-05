#ifndef BFCONTROLBASE_H
#define BFCONTROLBASE_H

#include <QObject>

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <linux/if_ether.h>
#include <linux/netlink.h>


#include "trx_data.h"

//typedef struct init_params_base
//{
//    init_params_base(const QVariant& value):value(value){}
//    QVariant value;
//} init_params_t;

typedef QVariant init_params_t;

class BFControlBase : public QObject
{
    Q_OBJECT
public:
    explicit BFControlBase(QObject *parent = 0);
    virtual  ~BFControlBase()=0;
    virtual  int  init           (init_params_t p = init_params_t(QVariant()) )=0;
    virtual  void close          ()=0;
    virtual  int  getRulesSync   (const filter_rule_t& pattern, QList<filter_rule_ptr >& ruleslst,int timeout_ms=3000)=0;
    virtual  int  getRulesAsync  (const filter_rule_t& pattern)=0;
    virtual  int  sendRulesSync  (const QList<filter_rule_ptr >& ruleslst )=0;
    virtual  int  deleteRule     (const filter_rule_t &pattern)=0;
    virtual  int  deleteRules    (const filter_rule_t &pattern)=0;
    virtual  int  addRule        (const filter_rule_t &pattern)=0;
    virtual  int  updateRule     (const filter_rule_t &pattern)=0;
    virtual  int  setChainPolicy (const filter_rule_t &pattern)=0;

signals:
    void data  (filter_rule_t);
    void data  (QByteArray);
    void data  (QList <filter_rule_ptr > );
    void log   (filter_rule_t);
    void done  ();
    void error (quint16);
    void error (msg_err_t);
    
public slots:
    
};

#endif // BFCONTROLBASE_H
