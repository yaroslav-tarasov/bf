#include "bflocalcontrol.h"
#include "bflocalcontrolprivate.h"

BFLocalControl::BFLocalControl(QObject *parent) :
    QObject(parent),d (new BFLocalControlPrivate)
{
    registerBfTypes();
    connect(d,SIGNAL(error (quint16)),SIGNAL(error (quint16)));
}


int  BFLocalControl::init(const QString& serverName)
{
    return d->init(serverName);
}

void BFLocalControl::close()
{
    /*return*/ d->close();
}



////////////////////////////////////////////////
//
//   Взаимодействие с сервисом
//
////////////////////////////////////////////////


///////////////////////////////////////
//   Удаление правила
//

int BFLocalControl::deleteRule(const filter_rule_t &pattern)
{
    return d->deleteRule(pattern);
}

///////////////////////////////////////
//   Удаление всех правил
//

int BFLocalControl::deleteRules(const filter_rule_t &pattern)
{
    return     d->deleteRules(pattern);
}

///////////////////////////////////////
//  Добавление правила
//

int BFLocalControl::addRule(const filter_rule_t &pattern)
{
    return d->addRule(pattern);
}

///////////////////////////////////////
//  Обновление правила
//  не базовых параметров (off; policy;)

int BFLocalControl::updateRule(const filter_rule_t &pattern)
{
    return d->updateRule(pattern);
}

////////////////////
// Получаем правила синхронно
// Каждое пришедшее сообщение продлевает действие таймера на timeout_ms
// По истечении timeout_ms считаем что передача завершена
// Полученные сообщения складываются в ruleslst
// Состав желаемых для получения правил определяется шаблоном pattern

int BFLocalControl::getRulesSync(const filter_rule_t& pattern, QList<filter_rule_ptr >& ruleslst,int timeout_ms)
{
    return d->getRulesSync(pattern,ruleslst,timeout_ms);
}

///////////////////////////////////////
//  Установка политики для цепочки
//

int BFLocalControl::setChainPolicy(const filter_rule_t &pattern)
{
    return d->setChainPolicy(pattern);
}


///////////////////////////////////////
//    Добавление правил
//
int BFLocalControl::sendRulesSync(const QList<filter_rule_ptr> &ruleslst)
{
    return d->sendRulesSync(ruleslst);
}
