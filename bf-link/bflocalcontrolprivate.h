#ifndef BFLOCALCONTROLPRIVATE_H
#define BFLOCALCONTROLPRIVATE_H

#include <QLocalSocket>
#include <QSharedPointer>
#include "bflocalcontrol.h"


namespace {

static inline quint32 generateSeq()
{
    return qrand();
}

}

class BFLocalControl::BFLocalControlPrivate : public QObject
{
    Q_OBJECT
public:

    explicit BFLocalControlPrivate(QObject *parent = 0);
    virtual ~BFLocalControlPrivate();

    int         init           (const QString& serverName);
    // int         create();
    void        close();
    int         getRulesSync   (const filter_rule_t& pattern, QList<filter_rule_ptr >& ruleslst,int timeout_ms=3000);
    int         getRulesAsync  (const filter_rule_t& pattern);
    int         sendRulesSync  (const QList<filter_rule_ptr >& ruleslst );
    int         deleteRule     (const filter_rule_t &pattern);
    int         deleteRules    (const filter_rule_t &pattern);
    int         addRule        (const filter_rule_t &pattern);
    int         updateRule     (const filter_rule_t &pattern);
    int         setChainPolicy (const filter_rule_t &pattern);
    int         subscribeLog   (/*pid_t pid*/);
private:
    inline int  sendMsg(int type,const filter_rule_t* msg,size_t size);
    int         sendCommand(const bf::bf_cmd_ptr_t& cmd);

signals:

public slots:

private slots:
    void        onConnected();
    void        onReadyRead();
    void        onDisconnected();
    void        onSocketError(QLocalSocket::LocalSocketError err);

private:
    int           mSocketWantedData;
private:
    QLocalSocket *mLocalSocket;
    QHash<quint32, bf::bf_cmd_ptr_t> mSentCommands;
    QList<bf::bf_cmd_ptr_t> mCommandQueue;
    QList<BFLocalControl::filter_rule_ptr >* ruleslst;
};

#endif // BFLOCALCONTROLPRIVATE_H
