#ifndef BFRULES_H
#define BFRULES_H

#include <QObject>
#include <QMutex>
#include <QHash>
#include "bfcontrol.h"

class BfRules : public QObject
{
    Q_OBJECT
public:
    static QList<filter_rule_ptr > loadFromFile (const QString& thename);
    static bool                    saveToFile   (const QString& thename);
    static void                    loadFromList (const QList<filter_rule_ptr > &list);
    static QList<filter_rule_ptr > getByPattern (const filter_rule_t  &fr);

public:
    static QList</*filter_rule_base,*/filter_rule_ptr> sRules;
signals:
    
public slots:

private:
    static   QMutex  mMux;
    explicit BfRules(QObject *parent = 0);
    Q_DISABLE_COPY(BfRules)
};

#endif // BFRULES_H
