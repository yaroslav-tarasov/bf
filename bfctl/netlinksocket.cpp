#include "netlinksocket.h"
#include <netlink/netlink.h>
#include <netlink/handlers.h>
#include <QDebug>
#include <thread>
#include <linux/if_ether.h>
#include "trx_data.h"



class NetlinkSocketPrivate
{
public:
    explicit NetlinkSocketPrivate(NetlinkSocket * pp): mpp(pp),nls(NULL){};
    ~NetlinkSocketPrivate(){destroy();};

//    void Create()
//    {
//        thread = new QThread;
//        mpp->moveToThread(thread);
//        QObject::connect(mpp, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
//        QObject::connect(thread, SIGNAL(started()), mpp, SLOT(process()));
//        QObject::connect(mpp, SIGNAL(finished()), thread, SLOT(quit()));
//        QObject::connect(mpp, SIGNAL(finished()), mpp, SLOT(deleteLater()));
//        QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
//        thread->start();

//    };

    inline int  create ()
    {
#ifdef HAVE_LIBNL3
     nls = nl_socket_alloc();
#else
     nls = nl_handle_alloc();
#endif

     struct nl_cb * p = nl_cb_alloc(NL_CB_DEFAULT);
     nls = nl_handle_alloc_cb(p);

      return nls?0:1;
    }

    inline int  connect (int protocol)
    {
        if(!nls) return -1;
        return nl_connect(nls, protocol);
    }

    inline int send_simple( int type, int flags, void *buf, size_t size)
    {
        if(!nls) return -1;
        return nl_send_simple(nls, type, flags, buf, size);
    }

    inline int recv ( struct sockaddr_nl *nla, unsigned char **buf, struct ucred **creds)
    {
        if(!nls) return -1;
        return nl_recv(nls, nla, buf,creds);
    }
    inline void destroy()
    {
        if(!nls) return ;
    #ifdef HAVE_LIBNL3
        nl_socket_free(nls);
    #else
        nl_handle_destroy(nls);
    #endif
    }


    NetlinkSocket* mpp;

#ifdef HAVE_LIBNL3
    struct nl_sock *nls;
#else
    struct nl_handle *nls;
#endif



    std::thread*  t;
    //    QThread* thread;
};


NetlinkSocket::NetlinkSocket(QObject *parent) :
    QObject(parent)
{
    d.reset(new NetlinkSocketPrivate(this));

}

void NetlinkSocket::startListener(int proto,int buffsize)
{
    mProto = proto;
    mBuffSize = buffsize;
    mBuff.reset(new unsigned char[mBuffSize]);
    memset(mBuff.get(),0,mBuffSize);
    d->t =  new std::thread(threadStart,this);
}

void NetlinkSocket::runListener()
{
     int ret;
     if((ret = d->create())==0)
     {
        if((ret = d->connect(mProto))==0)
        {
            unsigned char* b =  mBuff.get();
            unsigned char *nl_msg;

            filter_rule_t emsg;
            memset(&emsg,0,sizeof(filter_rule_t));
            ret = d->send_simple( MSG_GET_RULES, 0, &emsg, sizeof(filter_rule_t));

            if( ret = d->recv (NULL, &nl_msg, NULL))
            {
                QByteArray ba(reinterpret_cast<const char*>(nl_msg),mBuffSize);
                struct nlmsghdr * hdr = (struct nlmsghdr *) nl_msg;
                qDebug() << hdr->nlmsg_type ;//==NLMSG_ERROR ;
                emit  data(ba);
            }
        }
        else
        {
            qDebug() << ret;
            emit  finished();
        }

     }
     return;
}


void NetlinkSocket::OnReceive(int nErrorCode)
{

}

void NetlinkSocket::threadStart(NetlinkSocket *p)
{
   p->runListener();
}
