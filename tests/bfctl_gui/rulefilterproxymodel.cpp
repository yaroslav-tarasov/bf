#include "rulefilterproxymodel.h"
#include "bfctl_gui_defs.h"

RuleFilterProxyModel::RuleFilterProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
}

bool	RuleFilterProxyModel::filterAcceptsColumn ( int source_column, const QModelIndex & /*source_parent*/ ) const
{
    if( source_column == bfmodel::CHAIN )
        return false;

    return true;
}

int RuleFilterProxyModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 7;
}
