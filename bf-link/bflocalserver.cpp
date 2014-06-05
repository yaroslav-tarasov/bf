#include "bflocalserver.h"
#include "bfcontrol.h"

#include <QBuffer>
#include <QFile>


BFLocalServer::BFLocalServer(QObject *parent) :
    QObject(parent)
{
    mLocalServer = new QLocalServer(this);
    mBfc = new BFControl();
}

BFLocalServer::~BFLocalServer()
{
    mBfc->close();
}

int BFLocalServer::run(QString serverName) {

    QFile::remove("/tmp/"+serverName);
    if(!mLocalServer->listen(serverName)) {
        qWarning() << "!!!WARNING!!! Can't start server on"
                   << serverName << ":" << mLocalServer->errorString();
        return -1;
    }

    connect(mLocalServer, SIGNAL(newConnection()), SLOT(onLocalNewConnection()));

    qDebug() << "Listening on path:" << mLocalServer->fullServerName();


    mBfc->init();

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
                if(s->bytesAvailable() >= (qint64)sizeof(qint32)) {
                    int msgSize;
                    s->read((char *)&msgSize, sizeof(qint32));
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
                if(s->bytesAvailable() > 0) {
                    qDebug() << "There are some data (" << s->bytesAvailable() << "bytes after reading. Read again...";
                    onLocalReadyRead();
                }
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

//BF_CMD_ADD_RULE,                 //!< Добавление правила
//BF_CMD_DATA,                     //!< При пересылке данных из модуля ядра в  userspace
//BF_CMD_DONE,                     //!< По окончании пересылки данных из ядра
//BF_CMD_DELETE_RULE ,             //!< Удаление конкретного правила
//BF_CMD_DELETE_ALL_RULES,         //!< Удаление всех правил
//BF_CMD_UPDATE_RULE,              //!< (не реализовано)
//BF_CMD_CHAIN_POLICY,             //!< Конечное правило для цепочки
//BF_CMD_GET_RULES,                //!< Получение правил из модуля ядра
//BF_CMD_OK,                       //!< Подтверждние
//BF_CMD_ERR,                      //!< Ошибка
//BF_CMD_LOG,                      //!< Лог из модуля ядра
//BF_CMD_LOG_SUBSCRIBE             //!< Подписка на лог (реализован только один подписчик)

void BFLocalServer::processMessage(bf::BfCmd& cmd) {
    using namespace bf;

    qDebug() << "Get command.  Value" << cmd.mType
             << "seq." << cmd.mSequence;

    switch(cmd.mType) {
        case BF_CMD_ADD_RULE:
        {
            filter_rule_t fr = cmd.mValue.value<filter_rule_t>();
            int rv = mBfc->addRule(fr);

            msg_err_t errr(-rv);
            sendMsg(BF_CMD_ERR, cmd.mSequence, errr);

            return;
        }
        case BF_CMD_UPDATE_RULE:
        {
            filter_rule_t fr = cmd.mValue.value<filter_rule_t>();
            int rv = mBfc->updateRule(fr);

            msg_err_t errr(-rv);
            sendMsg(BF_CMD_ERR, cmd.mSequence, errr);

            return;
        }
        case BF_CMD_DELETE_RULE:
        {
            filter_rule_t fr = cmd.mValue.value<filter_rule_t>();
            int rv = mBfc->deleteRule(fr);

            msg_err_t errr(-rv);
            sendMsg(BF_CMD_ERR, cmd.mSequence, errr);

            return;
        }
        case BF_CMD_DELETE_ALL_RULES:
        {
            filter_rule_t fr = cmd.mValue.value<filter_rule_t>();
            int rv = mBfc->deleteRules(fr);

            msg_err_t errr(-rv);
            sendMsg(BF_CMD_ERR, cmd.mSequence, errr);

            return;
        }
        case BF_CMD_CHAIN_POLICY:
        {
            filter_rule_t fr = cmd.mValue.value<filter_rule_t>();
            int rv = mBfc->setChainPolicy(fr);

            msg_err_t errr(-rv);
            sendMsg(BF_CMD_ERR, cmd.mSequence, errr);

            return;
        }
        case BF_CMD_GET_RULES:
        {
            filter_rule_t fr = cmd.mValue.value<filter_rule_t>();
            QList<filter_rule_ptr > ruleslst;
            int rv =  mBfc->getRulesSync(fr,  ruleslst);

            if(rv>=0)
            {
                msg_done_t md;
                foreach (filter_rule_ptr rule,ruleslst){
                    sendMsg(BF_CMD_DATA, cmd.mSequence, *rule);
                    md.counter++;
                }

                sendMsg(BF_CMD_DONE, cmd.mSequence, md);
            }
            else
            {
                msg_err_t errr(-rv);
                sendMsg(BF_CMD_ERR, cmd.mSequence, errr);
            }

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

template<typename T>
int  BFLocalServer::sendMsg(bf::bf_cmd_t type,int seq,const T& msg)
{
    // bf_cmd_ptr_t cmd(new BfCmd);
    bf::BfCmd cmd;
    cmd.mType = bf::bf_cmd_t(type);
    cmd.mSequence = seq;
    cmd.mValue = QVariant::fromValue(msg);
    return sendResponse(cmd);
}

int BFLocalServer::sendResponse(bf::BfCmd& res)
{
    QByteArray ba;
    QBuffer bu(&ba);
    bu.open(QIODevice::WriteOnly);
    QDataStream s(&bu);

    s << res;

    quint32 baSize = ba.size();

    if(mClientCommands.contains(res.mSequence)) {
        QLocalSocket *s = mClientCommands[res.mSequence];
        int rv = s->write((char*)&baSize, sizeof(baSize));
        if(rv < 0) {
            qWarning() << "Can't send response size: socket error:"
                       << s->errorString()
                       << "(" << s->error() << ")";
            return rv;
        }
        rv = s->write(ba);
        if(rv < 0) {
            qWarning() << "Can't send response size: socket error:"
                       << s->errorString()
                       << "(" << s->error() << ")";
            return rv;
        }

        s->flush();

        return 0;

    } else {
        qWarning() << "WARNING: No sender local socket available for cmd seq." << res.mSequence;
    }

// TODO
    return -1;
}
