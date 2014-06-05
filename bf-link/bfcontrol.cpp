#include "bfcontrol.h"
#include <QDebug>
#include <QEventLoop>
#include <QTimer>

#include "netlinksocket.h"
#include "errorreceiver.h"
#include "timerproxy.h"
#include "qwaitfordone.h"


struct BFControlPrivate
{
    explicit BFControlPrivate(NetlinkSocket *pNS):mNS(pNS){}
    NetlinkSocket *mNS;
    QList<filter_rule_ptr >* ruleslst;
};


BFControl::BFControl(QObject *parent) :
    BFControlBase(parent)
{

    registerBfTypes();

    d.reset(new BFControlPrivate(new NetlinkSocket(/*this*/)));
    QObject::connect( d->mNS,SIGNAL(data(QByteArray)),this,SLOT(process(QByteArray)));
    QObject::connect( d->mNS,SIGNAL(data(QByteArray)),this,SIGNAL(data(QByteArray)));

}

void BFControl::process(QByteArray ba)
{
    //qDebug()<< "Enter BFControl::process";
    struct nlmsghdr * hdr = (struct nlmsghdr *) ba.data();
    if (hdr->nlmsg_type==MSG_DONE)
    {
        emit data (*d->ruleslst);
        emit done();
        d->ruleslst = NULL;
    }
    else  if(hdr->nlmsg_type==MSG_LOG)
    {
        unsigned char *msg = static_cast<unsigned char *>(NLMSG_DATA((struct nlmsghdr *)ba.data()));
        if (msg)
        // qDebug() << "Got log message:" << reinterpret_cast<filter_rule_t*>(msg)->base_rule.src_port;
        emit log (*reinterpret_cast<filter_rule_t*>(msg));
    }
    else   if (hdr->nlmsg_type==MSG_DATA && d->ruleslst)
    {
       unsigned char *msg = static_cast<unsigned char *>(NLMSG_DATA((struct nlmsghdr *)ba.data()));
       d->ruleslst->append(filter_rule_ptr(new filter_rule_t(*reinterpret_cast<filter_rule_t*>(msg))));
       ///qDebug() << "BFControl::process(QByteArray ba)" << *reinterpret_cast<filter_rule_t*>(msg);
    }
    else   if (hdr->nlmsg_type==MSG_DATA )
    {
       char *msg = static_cast<char *>(NLMSG_DATA((struct nlmsghdr *)ba.data()));
       emit data(filter_rule_t(*reinterpret_cast<filter_rule_t*>(msg)));
       ///qDebug() << "BFControl::process(QByteArray ba)" << *reinterpret_cast<filter_rule_t*>(msg);
    }
    else  if (hdr->nlmsg_type==NLMSG_ERROR)
    {
        struct nlmsgerr *nlerr = (struct nlmsgerr*)NLMSG_DATA(hdr);
        if(nlerr->error)
            printf("Error message with code: %d \n",nlerr->error);
    }
    else if(hdr->nlmsg_type==MSG_ERR)
    {
        msg_err_t *err = (msg_err_t*)NLMSG_DATA(hdr);

        if(err)
            emit error(quint16(err->code));
    }

    if(hdr->nlmsg_flags&NLM_F_ACK)
    {
        filter_rule_t msg;
        memset(&msg,0,sizeof(filter_rule_t));
        d->mNS->sendMsg(MSG_OK,&msg,sizeof(filter_rule_t));
        // qDebug() << "process   Send ACK" ;
    }


    //qDebug()<< "Leave BFControl::process";
    //qDebug() << "process" << hdr->nlmsg_type;
}


int BFControl::init(init_params_t p)
{
    Q_UNUSED(p);
    return d->mNS->create(NETLINK_USERSOCK);
}

int  BFControl::sendMsg(int type,const void* msg,size_t size)
{
    return d->mNS->sendMsg(type,const_cast<void *>(msg),size);
}


void BFControl::close()
{
    /*return*/ d->mNS->close();
}

////////////////////
// Получаем правила синхронно
// Каждое пришедшее сообщение продлевает действие таймера на timeout_ms
// По истечении timeout_ms считаем что передача завершена
// Полученные сообщения складываются в ruleslst
// Состав желаемых для получения правил определяется шаблоном pattern

int BFControl::getRulesSync(const filter_rule_t& pattern, QList<filter_rule_ptr >& ruleslst,int timeout_ms)
{
    QWaitForDone w(this);
    ErrorReciever errr(NULL/*this*/);

    QObject::connect(this, SIGNAL(data(filter_rule_t)), &w, SLOT(restart()));
    QObject::connect(this, SIGNAL(error(quint16)), &errr, SLOT(setError(quint16)));

    d->ruleslst = &ruleslst;
    int ret = this->sendMsg(MSG_GET_RULES,&pattern,sizeof(filter_rule_t));

    if(ret<0)
    {
        qWarning() << "Can't send command MSG_GET_RULES socket error:"
                   << "(" << ret << ")";
               // << d->mNS->errorString()
               // << "(" << d->mNS->error() << ")";
        return -BF_ERR_SOCK;
    }


    w.start(timeout_ms);

    QObject::disconnect(this, SIGNAL(data(filter_rule_t)), &w, SLOT(restart()));


    if (errr.getError()>0)
        ret = -errr.getError();

    QObject::disconnect(this, SIGNAL(error(quint16)), &errr, SLOT(setError(quint16)));

    d->ruleslst = NULL;

    return ret;
}

///////////////////////////////////////
//  По окончании процесса получения будет прислано событие rules(QList<filter_rules_t>)
//
int BFControl::getRulesAsync(const filter_rule_t &pattern)
{
    d->ruleslst = new QList<filter_rule_ptr >;
    int ret = this->sendMsg(MSG_GET_RULES,&pattern,sizeof(filter_rule_t));

    if(ret<0)
    {
        d->ruleslst = NULL;
        qWarning() << "Can't send command MSG_GET_RULES socket error:"
                   << "(" << ret << ")";
               // << d->mNS->errorString()
               // << "(" << d->mNS->error() << ")";
        return -BF_ERR_SOCK;
    }


    return ret;

}

///////////////////////////////////////
//   Удаление правила
//

int BFControl::deleteRule(const filter_rule_t &pattern)
{
    QWaitForDone w(this,QWaitForDone::DISCONNECT_DONE);
    ErrorReciever errr(NULL/*this*/);
    QObject::connect(this, SIGNAL(error(quint16)), &errr, SLOT(setError(quint16)));
    QObject::connect(this, SIGNAL(error(quint16)), &w, SLOT(quit()));
    int ret = this->sendMsg(MSG_DELETE_RULE, &pattern, sizeof(filter_rule_t));
    if(ret<0)
    {
        qWarning() << "Can't send command MSG_DELETE_RULE socket error:"
                   << "(" << ret << ")";
               // << d->mNS->errorString()
               // << "(" << d->mNS->error() << ")";
        return -BF_ERR_SOCK;
    }

    w.start(commandWaitTime);

    return -errr.getError();
}

///////////////////////////////////////
//   Удаление правил
//

int BFControl::deleteRules(const filter_rule_t &pattern)
{
    int ret = this->sendMsg(MSG_DELETE_ALL_RULES, &pattern, sizeof(filter_rule_t));
    if(ret<0)
    {
        qWarning() << "Can't send command MSG_DELETE_ALL_RULES socket error:"
                   << "(" << ret << ")";
               // << d->mNS->errorString()
               // << "(" << d->mNS->error() << ")";
        return -BF_ERR_SOCK;
    }

    return BF_ERR_OK;
}

///////////////////////////////////////
//  Добавление правила
//

int BFControl::addRule(const filter_rule_t &pattern)
{
    QWaitForDone w(this,QWaitForDone::DISCONNECT_DONE);
    ErrorReciever errr(NULL/*this*/);
    QObject::connect(this, SIGNAL(error(quint16)), &errr, SLOT(setError(quint16)));
    QObject::connect(this, SIGNAL(error(quint16)), &w, SLOT(quit()));
    int ret = this->sendMsg(MSG_ADD_RULE, &pattern, sizeof(filter_rule_t));
    if(ret<0)
    {
        qWarning() << "Can't send command MSG_ADD_RULE socket error:"
                   << "(" << ret << ")";
               // << d->mNS->errorString()
               // << "(" << d->mNS->error() << ")";
        return -BF_ERR_SOCK;
    }

    w.start(commandWaitTime);

    return -errr.getError();
}

///////////////////////////////////////
//  Обновление правила
//  не базовых параметров (off; policy;)

int BFControl::updateRule(const filter_rule_t &pattern)
{
     QWaitForDone w(this,QWaitForDone::DISCONNECT_DONE);
     ErrorReciever errr(NULL/*this*/);
     QObject::connect(this, SIGNAL(error(quint16)), &errr, SLOT(setError(quint16)));
     QObject::connect(this, SIGNAL(error(quint16)), &w, SLOT(quit()));
     int ret = this->sendMsg(MSG_UPDATE_RULE, &pattern, sizeof(filter_rule_t));
     if(ret<0)
     {
         qWarning() << "Can't send command MSG_UPDATE_RULE socket error:"
                    << "(" << ret << ")";
                // << d->mNS->errorString()
                // << "(" << d->mNS->error() << ")";
         return -BF_ERR_SOCK;
     }

     w.start(commandWaitTime);

     return -errr.getError();
}

///////////////////////////////////////
//  Установка политики для цепочки
//

int BFControl::setChainPolicy(const filter_rule_t &pattern)
{
    QWaitForDone w(this,QWaitForDone::DISCONNECT_DONE);
    ErrorReciever errr(NULL/*this*/);
    QObject::connect(this, SIGNAL(error(quint16)), &errr, SLOT(setError(quint16)));
    QObject::connect(this, SIGNAL(error(quint16)), &w, SLOT(quit()));

    filter_rule_t p;
    memset(&p,0,sizeof(filter_rule_t));
    p.policy = pattern.policy;
    p.base.chain = pattern.base.chain;
    int ret = this->sendMsg(MSG_CHAIN_POLICY, &p, sizeof(filter_rule_t));
    if(ret<0)
    {
        qWarning() << "Can't send command MSG_CHAIN_POLICY socket error:"
                   << "(" << ret << ")";
               // << d->mNS->errorString()
               // << "(" << d->mNS->error() << ")";
        return -BF_ERR_SOCK;
    }

    w.start(commandWaitTime);

    return -errr.getError();
}

///////////////////////////////////////
//  Подписка на получение лога о сработавших правилах
//  PID процесса получателя как аргумент
//

int BFControl::subscribeLog(pid_t pid)
{
    log_subscribe_msg_t msg(pid);
    int ret = this->sendMsg(MSG_LOG_SUBSCRIBE, &msg, sizeof(log_subscribe_msg_t));
    if(ret<0)
    {
        qWarning() << "Can't send command MSG_LOG_SUBSCRIBE socket error:"
                   << "(" << ret << ")";
               // << d->mNS->errorString()
               // << "(" << d->mNS->error() << ")";
        return -BF_ERR_SOCK;
    }

    return BF_ERR_OK;
}




///////////////////////////////////////
//    Добавление правил
//
int BFControl::sendRulesSync(const QList<filter_rule_ptr> &ruleslst)
{
    //int i=0;
    foreach (filter_rule_ptr rule,ruleslst){
        //qDebug() << "sendRulesSync  " << "rule #" << i++ << "  " << rule->base.src_port << "  " << rule->base.dst_port << "  " << rule->base.proto;
        filter_rule_t fr = *static_cast<filter_rule_t*>(rule.data());
        int ret = addRule(fr);

        if (ret < 0 && ret == -BF_ERR_SOCK) {
            d->ruleslst = NULL;
            return ret;
        }
    }

    return 0;

}
