#ifndef BFLOCALSERVER_H
#define BFLOCALSERVER_H

#include <QUdpSocket>
#include <QAbstractSocket>
#include <QLocalServer>
#include <QLocalSocket>
#include <QHash>

#include "trx_data.h"
#include "bf_defs.h"

class BFControl;

class BFLocalServer : public QObject
{
    Q_OBJECT
public:
    explicit BFLocalServer (QObject *parent = 0);
    virtual ~BFLocalServer();

    int      run           (QString path = BARRIER_BF_LOCAL_SOCK);

private:
    void     processMessage(bf::BfCmd& cmd);
    int sendResponse(bf::BfCmd& res);
    void     destroySocket (QLocalSocket *s);
    template<typename T>
    inline   int  sendMsg(bf::bf_cmd_t type,int seq,const T& msg);

signals:
    
public slots:

private slots:
    void     onLocalNewConnection();
    void     onLocalReadyRead    ();
    void     onLocalDisconnected ();
    void     onLocalError        (QLocalSocket::LocalSocketError err);


private:
     QHash<quint32, QLocalSocket*> mClientCommands;
     QLocalServer *                mLocalServer;
     QHash<QLocalSocket*, qint32>     mReadSizes;
     BFControl* mBfc;
};

#endif // BFLOCALSERVER_H
