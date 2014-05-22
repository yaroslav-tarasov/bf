#include "bflocalserver.h"

#include <QBuffer>
#include <QFile>

BFLocalServer::BFLocalServer(QObject *parent) :
    QObject(parent)
{
    m_LocalServer = new QLocalServer(this);
}

int BFLocalServer::run(QString serverName) {
    QFile::remove(serverName);
    if(!m_LocalServer->listen(serverName)) {
        qWarning() << "!!!WARNING!!! Can't start server on"
                   << serverName << ":" << m_LocalServer->errorString();
        return -1;
    }
    connect(m_LocalServer, SIGNAL(newConnection()), SLOT(onLocalNewConnection()));

    qDebug() << "Listening on path:" << serverName;
    return 0;
}

void BFLocalServer::onLocalNewConnection() {
    if(m_LocalServer->hasPendingConnections()) {
        QLocalSocket *s = m_LocalServer->nextPendingConnection();
        connect(s, SIGNAL(readyRead()), SLOT(onLocalReadyRead()));
        connect(s, SIGNAL(disconnected()), SLOT(onLocalDisconnected()));
        connect(s, SIGNAL(error(QLocalSocket::LocalSocketError)), SLOT(onLocalError(QLocalSocket::LocalSocketError)));
        m_ReadSizes[s] = -1;
    }
}

void BFLocalServer::onLocalReadyRead() {
    QObject *o = sender();
    if(o->inherits("QLocalSocket")) {
        QLocalSocket *s = qobject_cast<QLocalSocket*>(o);
        if(s) {
            if(m_ReadSizes[s] <= 0) {
                if(s->bytesAvailable() >= (qint64)sizeof(int)) {
                    int msgSize;
                    s->read((char *)&msgSize, sizeof(int));
                    m_ReadSizes[s] = msgSize;
                }
            }
            if(m_ReadSizes[s] > 0 && s->bytesAvailable() >= m_ReadSizes[s]) {
                QByteArray ba = s->read(m_ReadSizes[s]);

                QBuffer bu(&ba);
                bu.open(QIODevice::ReadOnly);
                QDataStream stream(&bu);

                //TODO// BacsNS::RemoteDaemonCommand cmd;
                //TODO// stream >> cmd;

                //TODO// m_UnixClientCommands[cmd.m_Sequence] = s;

                //TODO// processMessage(cmd);
                m_ReadSizes[s] = 0;
            }
        }
    }
}

void BFLocalServer::onLocalDisconnected() {
    QObject *o = sender();
    if(o->inherits("QLocalSocket")) {
        QLocalSocket *s = qobject_cast<QLocalSocket*>(o);
        if(s) {
            destroySocket(s);
        }
    }
}

void BFLocalServer::destroySocket(QLocalSocket *s) {
    bool found;
    while(true) {
        QList<quint32> keys = m_UnixClientCommands.keys();
        found = false;
        foreach(quint32 key, keys) {
            if(m_UnixClientCommands[key] == s) {
                m_UnixClientCommands.remove(key);
                s->disconnectFromServer();
                s->deleteLater();
                found = true;
                break;
            }
        }
        if(!found) {
            break;
        }
    }
}

void BFLocalServer::onLocalError(QLocalSocket::LocalSocketError err) {
    QObject *o = sender();
    if(o->inherits("QLocalSocket")) {
        QLocalSocket *s = qobject_cast<QLocalSocket*>(o);
        if(s) {
            qWarning() << "Local socket error:" << s->errorString()
                       << "(" << err << ")";
            destroySocket(s);
        }
    }
}
