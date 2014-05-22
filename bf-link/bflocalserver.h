#ifndef BFLOCALSERVER_H
#define BFLOCALSERVER_H

#include "bf_defs.h"

#include <QUdpSocket>
#include <QAbstractSocket>
#include <QLocalServer>
#include <QLocalSocket>
#include <QHash>

class BFLocalServer : public QObject
{
    Q_OBJECT
public:
    explicit BFLocalServer(QObject *parent = 0);
    int run(QString path = BARRIER_BF_LOCAL_SOCK);

signals:
    
public slots:

private slots:
    void onLocalNewConnection();
    void onLocalReadyRead();
    void onLocalDisconnected();
    void onLocalError(QLocalSocket::LocalSocketError err);

private:
    void destroySocket(QLocalSocket *s);
private:
     QHash<quint32, QLocalSocket*> m_UnixClientCommands;
     QLocalServer *m_LocalServer;
     QHash<QLocalSocket*, int> m_ReadSizes;
};

#endif // BFLOCALSERVER_H
