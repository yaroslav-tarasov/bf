#include "bflocalcontrol.h"

#include <QBuffer>
#include <QDataStream>
#include <QDateTime>

BFLocalControl::BFLocalControl(QObject *parent) :
    QObject(parent)
{
    mLocalSocket = new QLocalSocket(this);
}



void BFLocalControl::init(const QString& serverName) {
    mLocalSocket->connectToServer(serverName);
    //TODO// m_SocketWantedData = 0;
    connect(mLocalSocket, SIGNAL(connected()), SLOT(onConnected()));
    connect(mLocalSocket, SIGNAL(readyRead()), SLOT(onReadyRead()));
    connect(mLocalSocket, SIGNAL(disconnected()), SLOT(onDisconnected()));
    connect(mLocalSocket, SIGNAL(error(QLocalSocket::LocalSocketError)), SLOT(onSocketError(QLocalSocket::LocalSocketError)));
    // return 0;
}


void BFLocalControl::onConnected() {
//    if(!m_CommandQueue.isEmpty()) {
//        sendCommand(m_CommandQueue.takeFirst());
//    }
}

void BFLocalControl::onReadyRead() {

    if(mSocketWantedData <= 0) {
        if(mLocalSocket->bytesAvailable() >= (qint64)sizeof(int)) {
            mLocalSocket->read((char *)&mSocketWantedData, sizeof(int));
        }
    }

    if(mSocketWantedData > 0 && mLocalSocket->bytesAvailable() >= mSocketWantedData) {
        QByteArray ba = mLocalSocket->read(mSocketWantedData);
        QBuffer bu(&ba);
        bu.open(QIODevice::ReadOnly);
        QDataStream s(&bu);

//        RemoteDaemonResponse response;

//        s >> response;


//        if(m_SentCommands.contains(response.m_Sequence)) {
//            emit commandResponse(m_SentCommands[response.m_Sequence], response);
//        }
        mSocketWantedData = 0;

        if(mLocalSocket->bytesAvailable() > 0) {
            onReadyRead();
        }
    }
}

void BFLocalControl::onDisconnected() {
     mSocketWantedData = 0;
}

void BFLocalControl::onSocketError(QLocalSocket::LocalSocketError err) {
    qWarning() << "WARNING: Socket error:"
               << mLocalSocket->errorString() << "(" << err << ")";
    mLocalSocket->disconnectFromServer();
    mSocketWantedData = 0;
}
