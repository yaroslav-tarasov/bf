#include "bflocalserver.h"

#include <QBuffer>
#include <QFile>

BFLocalServer::BFLocalServer(QObject *parent) :
    QObject(parent)
{
    mLocalServer = new QLocalServer(this);
}

int BFLocalServer::run(QString serverName) {
    QFile::remove(serverName);
    if(!mLocalServer->listen(serverName)) {
        qWarning() << "!!!WARNING!!! Can't start server on"
                   << serverName << ":" << mLocalServer->errorString();
        return -1;
    }
    connect(mLocalServer, SIGNAL(newConnection()), SLOT(onLocalNewConnection()));

    qDebug() << "Listening on path:" << serverName;
    return 0;
}

void BFLocalServer::onLocalNewConnection() {
    if(mLocalServer->hasPendingConnections()) {
        QLocalSocket *s = mLocalServer->nextPendingConnection();
        connect(s, SIGNAL(readyRead()), SLOT(onLocalReadyRead()));
        connect(s, SIGNAL(disconnected()), SLOT(onLocalDisconnected()));
        connect(s, SIGNAL(error(QLocalSocket::LocalSocketError)), SLOT(onLocalError(QLocalSocket::LocalSocketError)));
        mReadSizes[s] = -1;
    }
}

void BFLocalServer::onLocalReadyRead() {
    QObject *o = sender();
    if(o->inherits("QLocalSocket")) {
        QLocalSocket *s = qobject_cast<QLocalSocket*>(o);
        if(s) {
            if(mReadSizes[s] <= 0) {
                if(s->bytesAvailable() >= (qint64)sizeof(int)) {
                    int msgSize;
                    s->read((char *)&msgSize, sizeof(int));
                    mReadSizes[s] = msgSize;
                }
            }
            if(mReadSizes[s] > 0 && s->bytesAvailable() >= mReadSizes[s]) {
                QByteArray ba = s->read(mReadSizes[s]);

                QBuffer bu(&ba);
                bu.open(QIODevice::ReadOnly);
                QDataStream stream(&bu);

                bf::BfCmd cmd;
                stream >> cmd;

                mClientCommands[cmd.mSequence] = s;

                processMessage(cmd);
                mReadSizes[s] = 0;
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
        QList<quint32> keys = mClientCommands.keys();
        found = false;
        foreach(quint32 key, keys) {
            if(mClientCommands[key] == s) {
                mClientCommands.remove(key);
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

void BFLocalServer::processMessage(bf::BfCmd& cmd) {
    using namespace bf;

    switch(cmd.mType) {
        case BF_CMD_ADD_RULE: {
            qDebug() << "Get command.  Value" << cmd.mType
                     << "seq." << cmd.mSequence;

//TODO            RemoteDaemonResponse res;
//TODO            res.m_Code = 0;
//TODO            res.m_Sequence = cmd.m_Sequence;
//TODO            res.m_Type = RD_RES_OK;
//TODO            sendResponse(res);
            return;
        }
        default:
            qWarning() << "Unhandled command type:" << cmd.mType;
    }
    // Удалим клиента из отправителей команды
    if(mClientCommands.contains(cmd.mSequence)) {
        mClientCommands[cmd.mSequence]->flush();
        mClientCommands.remove(cmd.mSequence);
    }
}
