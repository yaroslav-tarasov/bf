#include "netlinksocket.h"
#include <netlink/netlink.h>
#include <netlink/handlers.h>
#include <netlink/msg.h>
#ifdef HAVENL3
#include <netlink/version.h>
#endif
#include <thread>
#include <linux/if_ether.h>

#include <QDebug>
#include <QSocketNotifier>

#include "trx_data.h"

//There have been some changes starting with 3.2 regarding where and how libnl
//is being installed on the system in order to allow multiple libnl versions
//to be installed in parallel:

//   - Headers will be installed in ${prefix}/include/libnl3, therefore
//     you will need to add "-I/usr/include/libnl3" to CFLAGS

//   - The library basename was renamed to libnl-3, i.e. the SO names become
//     libnl-3.so., libnl-route-3.so, etc.

//   - libtool versioning was assumed, to ease detection of compatible library
//     versions.

//If you are using pkg-config for detecting and linking against the library
//things will continue magically as if nothing every happened. If you are
//linking manually you need to adapt your Makefiles or switch to using
//pkg-config files.


class NetlinkSocketPrivate
{
public:
    explicit NetlinkSocketPrivate(NetlinkSocket * pp):
        nlSocket(NULL),
        mpp(pp),
        init(false)
#ifdef NL_THREAD
        ,mRunning(true)
#endif
        {}
    ~NetlinkSocketPrivate(){destroy();}

    inline int  create ()
    {
#if  LIBNL_VER_MAJ>=3
     nlSocket = nl_socket_alloc();
#else
     nlSocket = nl_handle_alloc();
#endif

     struct nl_cb * p = nl_cb_alloc(NL_CB_DEFAULT);
#if  LIBNL_VER_MAJ>=3
     nlSocket = nl_socket_alloc_cb(p);
#else
     nlSocket = nl_handle_alloc_cb(p);
#endif


      return nlSocket?0:-1;
    }


    inline int  connect (int protocol)
    {
        if(!nlSocket) return -1;
        return nl_connect(nlSocket, protocol);
    }

    inline int send_simple( int type, int flags, void *buf, size_t size)
    {
        if(!nlSocket) return -1;
        return nl_send_simple(nlSocket, type, flags, buf, size);

    }

    inline int recv ( struct sockaddr_nl *nla, unsigned char **buf, struct ucred **creds)
    {
        if(!nlSocket) return -1;
        return nl_recv(nlSocket, nla, buf,creds);
    }
    inline void destroy()
    {
        if(!nlSocket) return ;
#if  LIBNL_VER_MAJ>=3
        nl_socket_free(nlSocket);
#else
        nl_handle_destroy(nlSocket);
#endif
    }

#if  LIBNL_VER_MAJ>=3
    struct nl_sock *nlSocket;
#else
    struct nl_handle *nlSocket;
#endif


    int             mProto;
    NetlinkSocket*  mpp;
#ifdef NL_THREAD
    bool            mRunning;
    std::thread*    t;
#else
    int             nlFd;
    QSocketNotifier* nlSn;
#endif
    bool init;
};


NetlinkSocket::NetlinkSocket(QObject *parent) :
    QObject(parent)
{
    d.reset(new NetlinkSocketPrivate(this));


}

NetlinkSocket::~NetlinkSocket()
{
    NetlinkSocket::close();
};

int NetlinkSocket::create(int proto)
{
    d->mProto = proto;
    int ret=0;
    if(!d->init && (ret = d->create())==0)
    {

       if((ret = d->connect(d->mProto))==0)
       {
#ifdef NL_THREAD
           d->t =  new std::thread(threadStart,this);
#else
           d->nlFd = nl_socket_get_fd(d->nlSocket);
           d->nlSn = new QSocketNotifier(d->nlFd,QSocketNotifier::Read,this);
           connect(d->nlSn,SIGNAL(activated(int)), this, SLOT(readyRead(void)));
#endif
           d->init = true;
       }
       else
       {
            emit  error(QString("Can't connect to netlink socket, connect return: %1").arg(ret));
       }

    }
   return ret;
}

void NetlinkSocket::close()
{
#ifdef NL_THREAD
    d->mRunning = false;
#endif
    if(d->nlSocket)
    {
#if  LIBNL_VER_MAJ>=3
#else
        nl_close(d->nlSocket);
        nl_handle_destroy(d->nlSocket);
#endif
        d->nlSocket = NULL;
    }
}

#ifdef NL_THREAD
void NetlinkSocket::runListener()
{
    int ret=0;
    unsigned char *nl_msg;
    while (d->mRunning)
    {
        if( (ret = d->recv (NULL, &nl_msg, NULL)) > 0)
        {
            int n = ret;
            struct nlmsghdr * hdr = (struct nlmsghdr *) nl_msg;
            unsigned char *msg;

            while (nlmsg_ok(hdr, n)) {
                msg = static_cast<unsigned char *>(nlmsg_data(hdr));
                if(hdr->nlmsg_len > 0)
                {
                    QByteArray ba(reinterpret_cast<const char*>(hdr),hdr->nlmsg_len);
                    //qDebug() << "nlmsg_type: " <<  hdr->nlmsg_type << "nlmsg_len: " << hdr->nlmsg_len ;
                    emit  data(ba);
                }
                hdr = nlmsg_next(hdr, &n);
            }
        }
        else
        {
            if(d->mRunning)
                emit error(QString("Got error on recieve: %1").arg(ret));
        }
    }
    return;
}
#endif

int  NetlinkSocket::sendMsg(int type,void* msg,size_t size)
{
    return d->send_simple( type, 0, msg, size);
}

#ifdef NL_THREAD
void NetlinkSocket::threadStart(NetlinkSocket *p)
{
   p->runListener();
}
#endif

void NetlinkSocket::readyRead()
{
    int ret=0;
    unsigned char *nl_msg;
    if( (ret = d->recv (NULL, &nl_msg, NULL)) > 0)
    {
        int n = ret;
        struct nlmsghdr * hdr = (struct nlmsghdr *) nl_msg;
        unsigned char *msg;

        while (nlmsg_ok(hdr, n)) {
            msg = static_cast<unsigned char *>(nlmsg_data(hdr));
            if(hdr->nlmsg_len > 0)
            {
                QByteArray ba(reinterpret_cast<const char*>(hdr),hdr->nlmsg_len);
                //qDebug() << "nlmsg_type: " <<  hdr->nlmsg_type << "nlmsg_len: " << hdr->nlmsg_len ;
                emit  data(ba);
            }
            hdr = nlmsg_next(hdr, &n);
        }
        // qDebug() <<  __PRETTY_FUNCTION__ << "  d->recv (NULL, &nl_msg, NULL))";
    }
    else
    {
#ifdef NL_THREAD
        if(d->mRunning)
#endif
            emit error(QString("Got error on recieve: %1").arg(ret));
    }
}
