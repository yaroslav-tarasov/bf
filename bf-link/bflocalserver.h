#ifndef BFLOCALSERVER_H
#define BFLOCALSERVER_H

#include <QUdpSocket>
#include <QAbstractSocket>
#include <QLocalServer>
#include <QLocalSocket>
#include <QHash>

#include "trx_data.h"
#include "bf_defs.h"

class BFLocalServer : public QObject
{
    Q_OBJECT
public:
    explicit BFLocalServer (QObject *parent = 0);
    int      run           (QString path = BARRIER_BF_LOCAL_SOCK);
private:
    void     processMessage(bf::BfCmd& cmd);
signals:
    
public slots:

private slots:
    void     onLocalNewConnection();
    void     onLocalReadyRead    ();
    void     onLocalDisconnected ();
    void     onLocalError        (QLocalSocket::LocalSocketError err);

private:
    void     destroySocket       (QLocalSocket *s);
private:
     QHash<quint32, QLocalSocket*> mClientCommands;
     QLocalServer *                mLocalServer;
     QHash<QLocalSocket*, int>     mReadSizes;
};

#endif // BFLOCALSERVER_H
