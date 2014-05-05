#include "bfrules.h"
#include <QDataStream>
#include <QFile>

//QHash<char*,BFControl::filter_rule_ptr> BfRules::sRules;
QHash<filter_rule_base,BFControl::filter_rule_ptr> BfRules::sRules;

uint qHash(const filter_rule_base& s)
{
    uint result = s.proto & 0xFF;

    result <<=(sizeof(quint8)*8);
    result = result|s.src_port;

    result <<=(sizeof(quint8)*8);
    result = result|s.dst_port;

    result <<=(sizeof(quint8)*8);
    result = result|s.s_addr.addr;
    return result;
}

BfRules::BfRules(QObject *parent) :
    QObject(parent)
{
}

QList<BFControl::filter_rule_ptr > BfRules::getFromFile(const QString &thename)
{
    QList<BFControl::filter_rule_ptr > ruleslst;
    QFile  file(thename) ;
    if (!file.open(QIODevice::ReadOnly))  return ruleslst;
    QDataStream in(&file);

    sRules.clear();
    while(!in.atEnd()){
        filter_rule_t fr;
        in >>  fr;
        BFControl::filter_rule_ptr p = BFControl::filter_rule_ptr(new filter_rule_t(fr));
        ruleslst.append(p);
        sRules[fr.base_rule]= p;//reinterpret_cast<char*>(&fr.base_rule)

    }
    file.close();
    return ruleslst;
}


bool BfRules::saveToFile(const QString &thename)
{
    QList<BFControl::filter_rule_ptr > ruleslst;
    QFile  file(thename) ;
    if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate))  return false;

    QDataStream out(&file);

//    out << sRules;

    foreach (BFControl::filter_rule_ptr p,sRules)
    {
        out << *(p.data());
        qDebug() << *(p.data()) << "size" << sRules.size();
    }


    file.flush();
    file.close();

    return true;
}

 void BfRules::getFromList(const QList<BFControl::filter_rule_ptr > &list)
 {
     foreach(BFControl::filter_rule_ptr p,list)
     {
         sRules[p->base_rule]= p;//reinterpret_cast<char*>(&p->base_rule)
     }
 }
