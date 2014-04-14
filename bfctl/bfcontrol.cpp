#include "bfcontrol.h"
#include <QDebug>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <linux/if_ether.h>
#include <linux/netlink.h>
#include "netlinksocket.h"
#include "trx_data.h"

struct BFControlPrivate
{
    explicit BFControlPrivate(NetlinkSocket *pNS):mNS(pNS){};
    NetlinkSocket *mNS;
};



BFControl::BFControl(QObject *parent) :
    QObject(parent)
{
    d.reset(new BFControlPrivate(new NetlinkSocket(this)));
    QObject::connect( d->mNS,SIGNAL(data(QByteArray)),this,SLOT(process(QByteArray))/*,Qt::DirectConnection*/);
    QObject::connect( d->mNS,SIGNAL(data(QByteArray)),this,SIGNAL(data(QByteArray))/*,Qt::DirectConnection*/);
}


void BFControl::process(QByteArray ba)
{
    struct nlmsghdr * hdr = (struct nlmsghdr *) ba.data();
    if (hdr->nlmsg_type==MSG_DONE)
    {
        emit done();
    }

    if(hdr->nlmsg_flags&NLM_F_ACK)
    {
        filter_rule_t msg;
        d->mNS->sendMsg(MSG_OK,&msg,sizeof(filter_rule_t));
    }

    qDebug() << "process" << hdr->nlmsg_type;
}


void BFControl::onData()
{

    qDebug() << "onDataArrives";

}

int BFControl::create()
{
    return d->mNS->create(NETLINK_USERSOCK,sizeof(filter_rule_t));
}

int  BFControl::sendMsg(int type,void* msg,size_t size)
{
    return d->mNS->sendMsg(type,msg,size);
}


void BFControl::close()
{
    /*return*/ d->mNS->close();
}


