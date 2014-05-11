#include "bfrules.h"
#include <QDataStream>
#include <QFile>

#include "hash_function.h"

QHash<filter_rule_base,BFControl::filter_rule_ptr> BfRules::sRules;
QMutex BfRules::mMux;

uint qHash(const filter_rule_base& s)
{
    return utils::__hash(reinterpret_cast<const char*>(&s),sizeof(filter_rule_base));
}

BfRules::BfRules(QObject *parent) :
    QObject(parent)
{

}

QList<BFControl::filter_rule_ptr > BfRules::getByPattern(const filter_rule_t  &fr)
{
    QList<BFControl::filter_rule_ptr > ruleslst;
    QMutexLocker lock_(&mMux);
    //qDebug()<< "Enter BfRules::getByPattern";
    //qDebug() << fr;
    foreach (BFControl::filter_rule_ptr p,sRules)
    {
        //qDebug() << "rule: "<< *p;
        if(fr_pattern(&*p,&fr) )
        {
            ruleslst.append(p);
            //qDebug() << "append "<< *p;
        }
    }
    //qDebug()<< "Leave BfRules::getByPattern";
    return ruleslst;
}

QList<BFControl::filter_rule_ptr > BfRules::loadFromFile(const QString &thename)
{
    //qDebug()<< "Enter BfRules::getFromFile";
    QList<BFControl::filter_rule_ptr > ruleslst;
    QFile  file(thename) ;
    if (!file.open(QIODevice::ReadOnly))  return ruleslst;
    QDataStream in(&file);

    QMutexLocker lock_(&mMux);
    sRules.clear();
    while(!in.atEnd()){
        filter_rule_t fr;
        in >>  fr;
        BFControl::filter_rule_ptr p = BFControl::filter_rule_ptr(new filter_rule_t(fr));
        ruleslst.append(p);
        sRules[fr.base]= p;
    }
    file.close();

    //qDebug()<< "Leave BfRules::getFromFile";
    return ruleslst;
}


bool BfRules::saveToFile(const QString &thename)
{
    QList<BFControl::filter_rule_ptr > ruleslst;
    QFile  file(thename) ;
    if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate))  return false;

    QDataStream out(&file);

    QMutexLocker lock_(&mMux);
    foreach (BFControl::filter_rule_ptr p,sRules)
    {
        out << *(p.data());
        // qDebug() << *(p.data()) << "size" << sRules.size();
    }


    file.flush();
    file.close();

    return true;
}

 void BfRules::loadFromList(const QList<BFControl::filter_rule_ptr > &list)
 {
     //qDebug()<< "Enter BfRules::getFromList";
     QMutexLocker lock_(&mMux);
     sRules.clear();
     foreach(BFControl::filter_rule_ptr p,list)
     {
         sRules[p->base]= p;

     }
     //qDebug()<< "Leave BfRules::getFromList";
 }
