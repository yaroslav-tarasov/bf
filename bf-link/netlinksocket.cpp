#include "netlinksocket.h"
#include <netlink/netlink.h>
#include <netlink/handlers.h>
#include <QDebug>
#include <thread>
#include <linux/if_ether.h>
#include "trx_data.h"

//
//   Maybe better?
//   http://stackoverflow.com/questions/11033971/qt-thread-with-movetothread
//

class NetlinkSocketPrivate
{
public:
    explicit NetlinkSocketPrivate(NetlinkSocket * pp): nls(NULL),mpp(pp){};
    ~NetlinkSocketPrivate(){destroy();};

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

#ifdef HAVE_LIBNL3
    struct nl_sock *nls;
#else
    struct nl_handle *nls;
#endif


    NetlinkSocket* mpp;
    std::thread*  t;
    //    QThread* thread;
};


NetlinkSocket::NetlinkSocket(QObject *parent) :
    QObject(parent),mRunning(true)
{
    d.reset(new NetlinkSocketPrivate(this));
    //d->nls->h_fd;
}

NetlinkSocket::~NetlinkSocket()
{
    NetlinkSocket::close();
    // d->t->join();
};

int NetlinkSocket::create(int proto,int buffsize)
{
    mProto = proto;
    mBuffSize = buffsize;
    //mBuff.reset(new unsigned char[mBuffSize]);
    //memset(mBuff.get(),0,mBuffSize);
    int ret;
    if((ret = d->create())==0)
    {
       if((ret = d->connect(mProto))==0)
       {
           d->t =  new std::thread(threadStart,this);

       }
       else
       {
            qDebug() << ret;
            emit  error(QString("Can't connect to netlink socket, connect return: %1").arg(ret));
       }

    }
   return ret;
}

void NetlinkSocket::close()
{
    mRunning = false;
    if(d->nls)
    {
        nl_close(d->nls);
        nl_handle_destroy(d->nls);
        d->nls = NULL;
    }
}

void NetlinkSocket::runListener()
{
    int ret;
    unsigned char *nl_msg;
    while (mRunning)
    {
        if( (ret = d->recv (NULL, &nl_msg, NULL)) > 0)
        {
            struct nlmsghdr * hdr = (struct nlmsghdr *) nl_msg;
            if(hdr->nlmsg_len > 0)
            {
                QByteArray ba(reinterpret_cast<const char*>(nl_msg),hdr->nlmsg_len);
                qDebug() << "nlmsg_type: " <<  hdr->nlmsg_type << "nlmsg_len: " << hdr->nlmsg_len ;
                emit  data(ba);
            }
        }
        else
        {
            if(mRunning)
                emit error(QString("Got error on recieve: %1").arg(ret));
        }
    }
    return;
}

int  NetlinkSocket::sendMsg(int type,void* msg,size_t size)
{
    return d->send_simple( type, 0, msg, size);
}

void NetlinkSocket::threadStart(NetlinkSocket *p)
{
   p->runListener();
}

