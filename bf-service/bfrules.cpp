#include "bfrules.h"
#include <QDataStream>
#include <QFile>

QHash<char*,BFControl::filter_rule_ptr> BfRules::sRules;

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
        sRules[reinterpret_cast<char*>(&fr.base_rule)]= p;

    }
    file.close();
    return ruleslst;
}


bool BfRules::saveToFile(const QString &thename)
{
    QList<BFControl::filter_rule_ptr > ruleslst;
    QFile  file(thename) ;
    if (!file.open(QIODevice::WriteOnly))  return false;

    QDataStream out(&file);

//    out << sRules;

    foreach (BFControl::filter_rule_ptr p,sRules)
    {
        out << *(p.data());
    }


    file.flush();
    file.close();

    return true;
}

 void BfRules::getFromList(const QList<BFControl::filter_rule_ptr > &list)
 {
     foreach(BFControl::filter_rule_ptr p,list)
     {
         sRules[reinterpret_cast<char*>(&p->base_rule)]= p;
     }
 }
