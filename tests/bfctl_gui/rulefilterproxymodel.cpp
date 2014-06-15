#include "rulefilterproxymodel.h"
#include "bfctl_gui_defs.h"

RuleFilterProxyModel::RuleFilterProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
}

bool	RuleFilterProxyModel::filterAcceptsColumn ( int source_column, const QModelIndex & source_parent ) const
{
    if( source_column == bfmodel::CHAIN )
        return false;

    return true;
}

//bool RuleFilterProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
//{
//    if (index.isValid() && role == Qt::EditRole) {
//        // записываем данные из каждого столбца
//        if(index.column()==OFF){
//     return true;
//}

//QVariant RuleFilterProxyModel::data(const QModelIndex& index, int role) const
//{
//    if(!index.isValid()) {
//        return QVariant();
//    }

//    if(index.column()>bfmodel::CHAIN)
//    {
//        return sourceModel()->index(index.row(),index.column()-1).data();
//    }

//    return sourceModel()->index(index.row(),index.column()).data();;
//}

int RuleFilterProxyModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 7;
}
