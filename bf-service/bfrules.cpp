#include <QtGlobal>
#include "reflection/refl_afx.h"
#include "bfrules.h"
#include "hash_function.h"

#include <QDataStream>
#include <QFile>


// QHash<filter_rule_base,filter_rule_ptr> BfRules::sRules;
QList</*filter_rule_base,*/filter_rule_ptr> BfRules::sRules;
QMutex BfRules::mMux;

#if 0
uint qHash(const filter_rule_base& s)
{
    return utils::__hash(reinterpret_cast<const char*>(&s),sizeof(filter_rule_base));
}
#endif

BfRules::BfRules(QObject *parent) :
    QObject(parent)
{

}

QList<filter_rule_ptr > BfRules::getByPattern(const filter_rule_t  &fr)
{
    QList<filter_rule_ptr > ruleslst;
    QMutexLocker lock_(&mMux);
    //qDebug()<< "Enter BfRules::getByPattern";
    //qDebug() << fr;
    foreach (filter_rule_ptr p,sRules)
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

QList<filter_rule_ptr > BfRules::loadFromFile(const QString &thename)
{
    std::ifstream is((thename+".xml").toStdString());

    //qDebug()<< "Enter BfRules::getFromFile";
    QList<filter_rule_ptr > ruleslst;
    QFile  file(thename) ;
    if (!file.open(QIODevice::ReadOnly))  return ruleslst;
    QDataStream in(&file);

    QMutexLocker lock_(&mMux);
    sRules.clear();
    while(!in.atEnd()){
        filter_rule_t fr;
        in >>  fr;
        filter_rule_ptr p = filter_rule_ptr(new filter_rule_t(fr));
        ruleslst.append(p);
        // sRules[fr.base]= p;
        sRules.append(p);
    }

    file.close();

    config_t cfg;

    try
    {
        if(is.good())
            ptree_ser::read_from<ptree_ser::JSON>(is, cfg);
        else
            qWarning() << "Отсутствует файл правил" + thename;
    }
    catch (const boost::property_tree::ptree_error& error)
    {
        // CatchException(error);
        qWarning()  << "Ошибка при чтении локального кеша правил: "<<error.what();
    }

    //qDebug()<< "Leave BfRules::getFromFile";
    return ruleslst;
}


bool BfRules::saveToFile(const QString &thename)
{
    QList<filter_rule_ptr > ruleslst;
    std::ofstream os((thename+".xml").toStdString());

    QFile  file(thename);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate))  return false;

    QDataStream out(&file);

    QMutexLocker lock_(&mMux);

    config_t cfg;

    foreach (filter_rule_ptr p,sRules)
    {
        out << *(p.data());
        cfg.rules.push_back(*p);

    }

    cfg.chain_policy[0]=(POLICY_ACCEPT); // INPUT
    cfg.chain_policy[1]=(POLICY_DROP);   // OUTPUT

    try
    {
        if(os.good())
            ptree_ser::write_to<ptree_ser::JSON>(os, cfg);
    }
    catch (const boost::property_tree::ptree_error& error)
    {
        // CatchException(error);
    }



    file.flush();
    file.close();


    return true;
}

 void BfRules::loadFromList(const QList<filter_rule_ptr > &list)
 {
     //qDebug()<< "Enter BfRules::getFromList";
     QMutexLocker lock_(&mMux);
     sRules.clear();
     foreach(filter_rule_ptr p,list)
     {
         // sRules[p->base]= p;
         sRules.append(p);
     }
     //qDebug()<< "Leave BfRules::getFromList";
 }
