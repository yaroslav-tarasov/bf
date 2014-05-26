#include "bflocalcontrolprivate.h"

#include <QBuffer>
#include <QDataStream>
#include <QDateTime>
#include <QLocalSocket>

#include "errorreceiver.h"
#include "timerproxy.h"
#include "qwaitfordone.h"


using namespace bf;

BFLocalControl::BFLocalControlPrivate::BFLocalControlPrivate(QObject *parent) :
    QObject(parent)
{
    mLocalSocket = new QLocalSocket(this);
}

BFLocalControl::BFLocalControlPrivate::~BFLocalControlPrivate() {
    mLocalSocket->disconnectFromServer();
}

int BFLocalControl::BFLocalControlPrivate::init(const QString& serverName) {

    mLocalSocket->connectToServer(serverName);

    if (mLocalSocket->waitForConnected(20*commandWaitTime))
    {
        mSocketWantedData = 0;
        connect( mLocalSocket, SIGNAL(connected()), SLOT(onConnected()));
        connect( mLocalSocket, SIGNAL(readyRead()), SLOT(onReadyRead()));
        connect( mLocalSocket, SIGNAL(disconnected()), SLOT(onDisconnected()));
        connect( mLocalSocket, SIGNAL(error(QLocalSocket::LocalSocketError)), SLOT(onSocketError(QLocalSocket::LocalSocketError)));
        return 0;
    }

    QLocalSocket::LocalSocketError e = mLocalSocket->error();

    return -e;
}

void BFLocalControl::BFLocalControlPrivate::close()
{
    /*return*/ mLocalSocket->disconnectFromServer();
}

////////////////////////////////////
//
//  Обработчики событий
//
void BFLocalControl::BFLocalControlPrivate::onConnected() {
    if(!mCommandQueue.isEmpty()) {
        sendCommand(mCommandQueue.takeFirst());
    }
}

void BFLocalControl::BFLocalControlPrivate::onReadyRead() {

    if(mSocketWantedData <= 0) {
        if( mLocalSocket->bytesAvailable() >= (qint64)sizeof(int)) {
             mLocalSocket->read((char *)&mSocketWantedData, sizeof(int));
        }
    }

    if(mSocketWantedData > 0 &&  mLocalSocket->bytesAvailable() >= mSocketWantedData) {
        QByteArray ba =  mLocalSocket->read(mSocketWantedData);
        QBuffer bu(&ba);
        bu.open(QIODevice::ReadOnly);
        QDataStream s(&bu);

//        RemoteDaemonResponse response;

//        s >> response;


//        if(m_SentCommands.contains(response.m_Sequence)) {
//            emit commandResponse(m_SentCommands[response.m_Sequence], response);
//        }

        mSocketWantedData = 0;

        if( mLocalSocket->bytesAvailable() > 0) {
            onReadyRead();
        }
    }
}

void BFLocalControl::BFLocalControlPrivate::onDisconnected() {
     mSocketWantedData = 0;
}

void BFLocalControl::BFLocalControlPrivate::onSocketError(QLocalSocket::LocalSocketError err) {
    qWarning() << "WARNING: Socket error:"
               <<  mLocalSocket->errorString() << "(" << err << ")";
     mLocalSocket->disconnectFromServer();
    mSocketWantedData = 0;
}


////////////////////////////////////////////////
//
//   Взаимодействие с сервисом
//
////////////////////////////////////////////////


///////////////////////////////////////
//   Удаление правила
//

int BFLocalControl::BFLocalControlPrivate::deleteRule(const filter_rule_t &pattern)
{
    QWaitForDone w(this,QWaitForDone::DISCONNECT_DONE);
    ErrorReciever errr(this);
    QObject::connect(this, SIGNAL(error(quint16)), &errr, SLOT(setError(quint16)));
    QObject::connect(this, SIGNAL(error(quint16)), &w, SLOT(quit()));
    int ret = this->sendMsg(MSG_DELETE_RULE, &pattern, sizeof(filter_rule_t));
    if(ret<0)
    {
        qWarning() << "Can't send command MSG_DELETE_RULE socket error:"
                   << "(" << ret << ")";
               // <<  mNS->errorString()
               // << "(" <<  mNS->error() << ")";
        return -BF_ERR_SOCK;
    }

    w.start(commandWaitTime);

    return -errr.getError();
}

///////////////////////////////////////
//   Удаление всех правил
//

int BFLocalControl::BFLocalControlPrivate::deleteRules(const filter_rule_t &pattern)
{
    int ret = this->sendMsg(MSG_DELETE_ALL_RULES, &pattern, sizeof(filter_rule_t));
    if(ret<0)
    {
        qWarning() << "Can't send command MSG_DELETE_ALL_RULES socket error:"
                   << "(" << ret << ")";
               // <<  mNS->errorString()
               // << "(" <<  mNS->error() << ")";
        return -BF_ERR_SOCK;
    }

    return BF_ERR_OK;
}

///////////////////////////////////////
//  Добавление правила
//

int BFLocalControl::BFLocalControlPrivate::addRule(const filter_rule_t &pattern)
{
    QWaitForDone w(this,QWaitForDone::DISCONNECT_DONE);
    ErrorReciever errr(this);
    QObject::connect(this, SIGNAL(error(quint16)), &errr, SLOT(setError(quint16)));
    QObject::connect(this, SIGNAL(error(quint16)), &w, SLOT(quit()));
    int ret = this->sendMsg(MSG_ADD_RULE, &pattern, sizeof(filter_rule_t));
    if(ret<0)
    {
        qWarning() << "Can't send command MSG_ADD_RULE socket error:"
                   << "(" << ret << ")";
               // <<  mNS->errorString()
               // << "(" <<  mNS->error() << ")";
        return -BF_ERR_SOCK;
    }

    w.start(commandWaitTime);

    return -errr.getError();
}

///////////////////////////////////////
//  Обновление правила
//  не базовых параметров (off; policy;)

int BFLocalControl::BFLocalControlPrivate::updateRule(const filter_rule_t &pattern)
{
     QWaitForDone w(this,QWaitForDone::DISCONNECT_DONE);
     ErrorReciever errr(this);
     QObject::connect(this, SIGNAL(error(quint16)), &errr, SLOT(setError(quint16)));
     QObject::connect(this, SIGNAL(error(quint16)), &w, SLOT(quit()));
     int ret = this->sendMsg(MSG_UPDATE_RULE, &pattern, sizeof(filter_rule_t));
     if(ret<0)
     {
         qWarning() << "Can't send command MSG_UPDATE_RULE socket error:"
                    << "(" << ret << ")";
                // <<  mNS->errorString()
                // << "(" <<  mNS->error() << ")";
         return -BF_ERR_SOCK;
     }

     w.start(commandWaitTime);

     return -errr.getError();
}

////////////////////
// Получаем правила синхронно
// Каждое пришедшее сообщение продлевает действие таймера на timeout_ms
// По истечении timeout_ms считаем что передача завершена
// Полученные сообщения складываются в ruleslst
// Состав желаемых для получения правил определяется шаблоном pattern

int BFLocalControl::BFLocalControlPrivate::getRulesSync(const filter_rule_t& pattern, QList<filter_rule_ptr >& ruleslst,int timeout_ms)
{
    QWaitForDone w(this);
    ErrorReciever errr(this);

    QObject::connect(this, SIGNAL(data(filter_rule_t)), &w, SLOT(restart()));
    QObject::connect(this, SIGNAL(error(quint16)), &errr, SLOT(setError(quint16)));

    BFLocalControlPrivate::ruleslst = &ruleslst;
    int ret = this->sendMsg(MSG_GET_RULES,&pattern,sizeof(filter_rule_t));

    if(ret<0)
    {
        qWarning() << "Can't send command MSG_GET_RULES socket error:"
                   << "(" << ret << ")";
               // << d->mNS->errorString()
               // << "(" << d->mNS->error() << ")";
        return -BF_ERR_SOCK;
    }


    w.start(timeout_ms);

    QObject::disconnect(this, SIGNAL(data(filter_rule_t)), &w, SLOT(restart()));


    if (errr.getError()>0)
        ret = -errr.getError();
    QObject::disconnect(this, SIGNAL(error(quint16)), &errr, SLOT(setError(quint16)));

    BFLocalControlPrivate::ruleslst = NULL;

    return ret;
}

///////////////////////////////////////
//  Установка политики для цепочки
//

int BFLocalControl::BFLocalControlPrivate::setChainPolicy(const filter_rule_t &pattern)
{
    filter_rule_t p;
    memset(&p,0,sizeof(filter_rule_t));
    p.policy = pattern.policy;
    p.base.chain = pattern.base.chain;
    int ret = this->sendMsg(MSG_CHAIN_POLICY, &p, sizeof(filter_rule_t));
    if(ret<0)
    {
        qWarning() << "Can't send command MSG_CHAIN_POLICY socket error:"
                   << "(" << ret << ")";
               // << d->mNS->errorString()
               // << "(" << d->mNS->error() << ")";
        return -BF_ERR_SOCK;
    }

    return BF_ERR_OK;
}

///////////////////////////////////////
//    Добавление правил
//
int BFLocalControl::BFLocalControlPrivate::sendRulesSync(const QList<filter_rule_ptr> &ruleslst)
{
    //int i=0;
    foreach (BFLocalControl::filter_rule_ptr rule,ruleslst){
        //qDebug() << "sendRulesSync  " << "rule #" << i++ << "  " << rule->base.src_port << "  " << rule->base.dst_port << "  " << rule->base.proto;
        filter_rule_t fr = *static_cast<filter_rule_t*>(rule.data());
        int ret = addRule(fr);

        if (ret < 0) {
            BFLocalControlPrivate::ruleslst = NULL;
            return ret;
        }
    }

    return 0;

}


int  BFLocalControl::BFLocalControlPrivate::sendMsg(int type,const filter_rule_t* msg,size_t size)
{
    bf_cmd_ptr_t cmd;
    cmd->mType = bf_cmd_t(type);
    cmd->mSequence = generateSeq();
    cmd->mFr = *msg;
    return sendCommand(cmd);
}


int  BFLocalControl::BFLocalControlPrivate::sendCommand(const bf_cmd_ptr_t& cmd)
{
    if(mLocalSocket->state() != QLocalSocket::ConnectedState)
    {
        mCommandQueue << cmd;
        qWarning() << "Local socket not connected. Command queued and will be send later when connection would be established";
        return 0;
    }

    QByteArray ba;
    QBuffer bu(&ba);
    bu.open(QIODevice::WriteOnly);
    QDataStream s(&bu);

    s << *(cmd.data());

    int baSize = ba.size();
    int ret = mLocalSocket->write((char*)&baSize, sizeof(baSize));
    if(ret < 0) {
        qWarning() << "Can't send command size: socket error:"
                   << mLocalSocket->errorString()
                   << "(" << mLocalSocket->error() << ")";
        return ret;
    }
    ret = mLocalSocket->write(ba);
    if(ret < 0) {
        qWarning() << "Can't send command body: socket error:"
                   << mLocalSocket->errorString()
                   << "(" << mLocalSocket->error() << ")";

        return ret;
    }

    mSentCommands[cmd->mSequence] = cmd;
    return 0;
}



