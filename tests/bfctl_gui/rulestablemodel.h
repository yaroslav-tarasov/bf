#ifndef RULESTABLEMODEL_H
#define RULESTABLEMODEL_H

#include <QAbstractTableModel>
#include "trx_data.h"

typedef  filter_rule_t  RuleEntry;

class RulesTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:

    typedef  QList<filter_rule_ptr >   rules_list_ptr_t;
    typedef  QList<filter_rule_t >     rules_list_t;

    explicit RulesTableModel(QObject *parent = 0);
    virtual ~RulesTableModel();
    Qt::ItemFlags    flags      (const QModelIndex& index)       const;
    int              rowCount   (const QModelIndex &parent)      const;
    int              columnCount(const QModelIndex &parent)      const;
    QVariant         data       (const QModelIndex &index, int role) const;
    QVariant         headerData (int section, Qt::Orientation orientation, int role) const;
    bool             setData    (const QModelIndex &index, const QVariant &value, int role);


    void             setRules   (const rules_list_ptr_t& items);
    const RuleEntry& rule       (const QModelIndex& index)       const;
    rules_list_ptr_t rules      (const QModelIndexList& indexes) const;
    void             removeItems(const QModelIndexList& indexes);
    void             removeItem (const QModelIndex& index);
    rules_list_ptr_t getDirty   (const QModelIndexList& indexes) const;
    rules_list_ptr_t getDirty   () const;
    void             clearDirty ();
signals:
    
public slots:
private:
    rules_list_ptr_t mItems;
    rules_list_t     mOldItems;
};

#endif // RULESTABLEMODEL_H
