#include "bfcontrol.h"
#include <QDebug>
#include <QEventLoop>
#include <QTimer>
#include "netlinksocket.h"


struct BFControlPrivate
{
    explicit BFControlPrivate(NetlinkSocket *pNS):mNS(pNS){};
    NetlinkSocket *mNS;
    QList<BFControl::filter_rule_ptr >* ruleslst;
};



BFControl::BFControl(QObject *parent) :
    QObject(parent)
{
    qRegisterMetaType<QList<filter_rule_ptr > >(" QList<filter_rule_ptr> ");
    qRegisterMetaType<filter_rule_t >(" filter_rule_t ");

    d.reset(new BFControlPrivate(new NetlinkSocket(this)));
    QObject::connect( d->mNS,SIGNAL(data(QByteArray)),this,SLOT(process(QByteArray)));
    QObject::connect( d->mNS,SIGNAL(data(QByteArray)),this,SIGNAL(data(QByteArray)));
#ifdef TEST_ASYNC_GET_RULES
    QObject::connect( this,SIGNAL(data(QList <filter_rule_ptr >)),this,SLOT(process(QList <filter_rule_ptr >)));
#endif
}

#ifdef TEST_ASYNC_GET_RULES
void BFControl::process(QList <filter_rule_ptr > ruleslst)
{
    int i=0;
    foreach (BFControl::filter_rule_ptr rule,ruleslst){
        qDebug() << "rule #" << i++ << "  " << rule->base_rule.src_port << "  " << rule->base_rule.dst_port << "  " << rule->base_rule.proto;
    }

}
#endif

void BFControl::process(QByteArray ba)
{
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
        qDebug() << "Got log message:" << reinterpret_cast<filter_rule_t*>(msg)->base_rule.src_port;
        emit log (*reinterpret_cast<filter_rule_t*>(msg));
    }
    else   if (hdr->nlmsg_type==MSG_DATA && d->ruleslst)
    {
       unsigned char *msg = static_cast<unsigned char *>(NLMSG_DATA((struct nlmsghdr *)ba.data()));
       d->ruleslst->append(filter_rule_ptr(new filter_rule_t(*reinterpret_cast<filter_rule_t*>(msg))));
    }
    else  if (hdr->nlmsg_type==NLMSG_ERROR)
    {
        struct nlmsgerr *nlerr = (struct nlmsgerr*)NLMSG_DATA(hdr);
        if(nlerr->error)
            printf("Error message with code: %d \n",nlerr->error);
    }

    if(hdr->nlmsg_flags&NLM_F_ACK)
    {
        filter_rule_t msg;
        memset(&msg,0,sizeof(filter_rule_t));
        d->mNS->sendMsg(MSG_OK,&msg,sizeof(filter_rule_t));
        qDebug() << "process   Send ACK" ;
    }



    qDebug() << "process" << hdr->nlmsg_type;
}


int BFControl::create()
{
    return d->mNS->create(NETLINK_USERSOCK);
}

int  BFControl::sendMsg(int type,void* msg,size_t size)
{
    return d->mNS->sendMsg(type,msg,size);
}


void BFControl::close()
{
    /*return*/ d->mNS->close();
}


int BFControl::getRulesSync(filter_rule_t& pattern, QList<filter_rule_ptr >& ruleslst,int timeout_ms)
{
    QEventLoop wait_for_done;
    QTimer timer;

    timer.setSingleShot(true);

    QObject::connect(&timer, SIGNAL(timeout()), &wait_for_done, SLOT(quit()));
    QObject::connect(this, SIGNAL(done()), &wait_for_done, SLOT(quit()));

    d->ruleslst = &ruleslst;
    int ret = this->sendMsg(MSG_GET_RULES,&pattern,sizeof(filter_rule_t));

    if (ret < 0) {
        return ret;
    }

    timer.start(timeout_ms);
    wait_for_done.exec();

    if (timer.isActive()) {
        timer.stop();
    }

    d->ruleslst = NULL;

    return ret;
}

///////////////////////////////////////
//  По окончании процесса получения будет прислано событие rules(QList<filter_rules_t>)
//
int BFControl::getRulesAsync(filter_rule_t &pattern)
{
    d->ruleslst = new QList<filter_rule_ptr >;
    int ret = this->sendMsg(MSG_GET_RULES,&pattern,sizeof(filter_rule_t));

    if (ret < 0) {
        d->ruleslst = NULL;
        return ret;
    }


    return ret;

}

///////////////////////////////////////
//   Удаление правила
//

int BFControl::deleteRule(filter_rule_t &pattern)
{
    return this->sendMsg(MSG_DELETE_RULE, &pattern, sizeof(filter_rule_t));
}

///////////////////////////////////////
//   Удаление правил
//

int BFControl::deleteRules(filter_rule_t &pattern)
{
    return this->sendMsg(MSG_DELETE_ALL_RULES, &pattern, sizeof(filter_rule_t));
}

///////////////////////////////////////
//  Добавление правила
//

int BFControl::addRule(filter_rule_t &pattern)
{
    return this->sendMsg(MSG_ADD_RULE, &pattern, sizeof(filter_rule_t));
}

///////////////////////////////////////
//  Подписка на получение лога о сбработавших правилах
//  PID процесса получателя как аргумент
//

int BFControl::subscribeLog(pid_t pid)
{
    _log_subscribe_msg_t msg(pid);
    return this->sendMsg(MSG_LOG_SUBSCRIBE, &msg, sizeof(_log_subscribe_msg_t));
}




///////////////////////////////////////
//    Добавление правил
//
int BFControl::sendRulesSync(QList<filter_rule_ptr >& ruleslst)
{
    int i=0;
    foreach (BFControl::filter_rule_ptr rule,ruleslst){
        qDebug() << "sendRulesSync  " << "rule #" << i++ << "  " << rule->base_rule.src_port << "  " << rule->base_rule.dst_port << "  " << rule->base_rule.proto;
        filter_rule_t fr = *static_cast<filter_rule_t*>(rule.data());
        int ret = addRule(fr);

        if (ret < 0) {
            d->ruleslst = NULL;
            return ret;
        }
    }

    return 0;

}
