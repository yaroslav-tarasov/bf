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

    if (mLocalSocket->waitForConnected(/*commandWaitTime*/))
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

        BfCmd cmd;

        s >> cmd;

        if(cmd.mType == BF_CMD_ERR)
        {
                msg_err_t errr = cmd.mValue.value<msg_err_t>();

//                qDebug() << "Get BF_CMD_ERR.  Code" << errr.code
//                         << "seq." << cmd.mSequence;

                emit error(quint16(errr.code));

//            if(mSentCommands.contains(response.m_Sequence)) {
//                emit commandResponse(mSentCommands[response.m_Sequence], response);
//            }
        }
        else   if (cmd.mType==BF_CMD_DATA )
        {
           filter_rule_t fr = cmd.mValue.value<filter_rule_t>();
           if(BFLocalControlPrivate::ruleslst)
               BFLocalControlPrivate::ruleslst->append(filter_rule_ptr(new filter_rule_t(fr)));

           emit data(fr);
        }
        else   if (cmd.mType==BF_CMD_DONE )
        {
           msg_done_t mdone = cmd.mValue.value<msg_done_t>();
           emit data (*BFLocalControlPrivate::ruleslst);
           emit done();
           BFLocalControlPrivate::ruleslst = NULL;
        }

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
    int ret = this->sendMsg(BF_CMD_DELETE_RULE, pattern, sizeof(filter_rule_t));
    if(ret<0)
    {
        qWarning() << "Can't send command BF_CMD_DELETE_RULE socket error:"
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
    QWaitForDone w(this,QWaitForDone::DISCONNECT_DONE);
    ErrorReciever errr(this);
    QObject::connect(this, SIGNAL(error(quint16)), &errr, SLOT(setError(quint16)));
    QObject::connect(this, SIGNAL(error(quint16)), &w, SLOT(quit()));
    int ret = this->sendMsg(BF_CMD_DELETE_ALL_RULES, pattern, sizeof(filter_rule_t));
    if(ret<0)
    {
        qWarning() << "Can't send command BF_CMD_DELETE_ALL_RULES socket error:"
                   << "(" << ret << ")";
               // <<  mNS->errorString()
               // << "(" <<  mNS->error() << ")";
        return -BF_ERR_SOCK;
    }

    w.start(commandWaitTime);

    return -errr.getError();


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
    int ret = this->sendMsg(BF_CMD_ADD_RULE, pattern, sizeof(filter_rule_t));
    if(ret<0)
    {
        qWarning() << "Can't send command BF_CMD_ADD_RULE socket error:"
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
     int ret = this->sendMsg(BF_CMD_UPDATE_RULE, pattern, sizeof(filter_rule_t));
     if(ret<0)
     {
         qWarning() << "Can't send command BF_CMD_UPDATE_RULE socket error:"
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
    int ret = this->sendMsg(BF_CMD_GET_RULES,pattern,sizeof(filter_rule_t));

    if(ret<0)
    {
        qWarning() << "Can't send command BF_CMD_GET_RULES socket error:"
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
    QWaitForDone w(this,QWaitForDone::DISCONNECT_DONE);
    ErrorReciever errr(this);
    QObject::connect(this, SIGNAL(error(quint16)), &errr, SLOT(setError(quint16)));
    QObject::connect(this, SIGNAL(error(quint16)), &w, SLOT(quit()));

    filter_rule_t p;
    memset(&p,0,sizeof(filter_rule_t));
    p.policy = pattern.policy;
    p.base.chain = pattern.base.chain;
    int ret = this->sendMsg(BF_CMD_CHAIN_POLICY, p, sizeof(filter_rule_t));
    if(ret<0)
    {
        qWarning() << "Can't send command BF_CMD_CHAIN_POLICY socket error:"
                   << "(" << ret << ")";
               // << d->mNS->errorString()
               // << "(" << d->mNS->error() << ")";
        return -BF_ERR_SOCK;
    }

    w.start(commandWaitTime);

    return -errr.getError();
}

///////////////////////////////////////
//    Добавление правил
//
int BFLocalControl::BFLocalControlPrivate::sendRulesSync(const QList<filter_rule_ptr> &ruleslst)
{
    //int i=0;
    foreach (filter_rule_ptr rule,ruleslst){
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

template<typename T>
int  BFLocalControl::BFLocalControlPrivate::sendMsg(bf_cmd_t type,const T& msg,size_t size)
{
    bf_cmd_ptr_t cmd(new BfCmd);
    cmd->mType = bf_cmd_t(type);
    cmd->mSequence = generateSeq();
    cmd->mValue = QVariant::fromValue(msg);
    return sendCommand(cmd);
}


int  BFLocalControl::BFLocalControlPrivate::packCommand(const bf_cmd_ptr_t& cmd, QDataStream& ds)
{
    qint32 baSize =0;
    QByteArray ba;
    QBuffer bu(&ba);
    bu.open(QIODevice::ReadWrite);
    QDataStream s(&bu);

    s << (qint32)baSize;
    s << *(cmd.data());
    baSize = ba.size() - sizeof(baSize);
    s.device()->seek(0);

    s << (qint32)baSize;
    s.device()->seek(0);
    ds << ba;

    return 0;
}

int  BFLocalControl::BFLocalControlPrivate::sendCommand(const bf_cmd_ptr_t& cmd)
{
    if(mLocalSocket->state() != QLocalSocket::ConnectedState)
    {
        mCommandQueue << cmd;
        qWarning() << "Local socket not connected. Command queued and will be send later when connection would be established";
        return -1;
    }

    QByteArray ba;
    QBuffer bu(&ba);
    bu.open(QIODevice::WriteOnly);
    QDataStream s(&bu);

    s << *(cmd.data());

    qint32 baSize = ba.size();

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

    mLocalSocket->flush();
    mSentCommands[cmd->mSequence] = cmd;
    return 0;
}



