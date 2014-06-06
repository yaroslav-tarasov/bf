#include "rulestablemodel.h"

RulesTableModel::RulesTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

RulesTableModel::~RulesTableModel()
{

}

int RulesTableModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return mItems.size();
}

int RulesTableModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 8;
}

QVariant RulesTableModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid()) {
        return QVariant();
    }
    if(index.row() < 0 || index.row() >= mItems.size()) {
        return QVariant();
    }

    const RuleEntry& item = *mItems[index.row()];
    if(Qt::DisplayRole == role || Qt::ToolTipRole == role || role == Qt::EditRole) {
        switch(index.column()) {
            case SRCIP:   return QHostAddress(static_cast<quint32>(htonl(item.base.s_addr.addr))).toString();
            case SRCPORT: return item.base.src_port;
            case DSTIP:   return QHostAddress(static_cast<quint32>(htonl(item.base.d_addr.addr))).toString();
            case DSTPORT: return item.base.dst_port;
            case PROTO:   return item.base.proto; // get_proto_name(item.base.proto);
            case CHAIN:   return item.base.chain; // get_chain_name(static_cast<bf_chain_t>(item.base.chain));
            case POLICY:  return item.policy; // get_policy_name(static_cast<bf_policy_t>(item.policy));
            case OFF:     return item.off;// get_sw_name(static_cast<bf_switch_rules_t>(item.off));

            default: return QVariant();
        }
    }

    return QVariant();
}

bool RulesTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        // записываем данные из каждого столбца
        if(index.column()==OFF){

           if (mItems.at(index.row())->off != value.toInt())
           {
               mItems.at(index.row())->off = value.toInt();
               emit dataChanged(index, index);
           }

        }
        else if (index.column()==POLICY){

          if (mItems.at(index.row())->policy != value.toInt())
          {
              mItems.at(index.row())->policy = value.toInt();
              emit dataChanged(index, index);

          }
        }

        return true;
    }
    return false;
}

QVariant RulesTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role != Qt::DisplayRole && role != Qt::ToolTipRole) {
        return QVariant();
    }
    if(orientation == Qt::Horizontal) {
        switch(section) {
            case SRCIP:       return tr("Source address");
            case SRCPORT:     return tr("Source port");
            case DSTIP:       return tr("Destination address");
            case DSTPORT:     return tr("Destination port");
            case PROTO:       return tr("Protocol");
            case CHAIN:       return tr("Chain");
            case POLICY:      return tr("Policy");
            case OFF:         return tr("Off");

            default: return "";
        }
    }
    return QVariant();
}


RulesTableModel::rules_list_t RulesTableModel::getDirty() const
{
    rules_list_t as;
    const QModelIndexList& indexes = persistentIndexList();
    foreach(const QModelIndex& index, indexes) {
        if(!index.isValid()) continue;
        if(index.row() < 0) continue;
        if(index.row() >= mItems.size()) continue;
        if(!index.data(Qt::UserRole).isValid() || !index.data(Qt::UserRole).toBool()) continue;
        as << mItems[index.row()];
    }

    return as;
}

RulesTableModel::rules_list_t RulesTableModel::rules(const QModelIndexList& indexes) const
{
    rules_list_t as;

    foreach(const QModelIndex& index, indexes) {
        if(!index.isValid()) continue;
        if(index.row() < 0) continue;
        if(index.row() >= mItems.size()) continue;
        as << mItems[index.row()];
    }

    return as;
}

const RuleEntry &RulesTableModel::rule(const QModelIndex& index) const
{
    static RuleEntry invalidAcl;
    if(!index.isValid()) {
        return invalidAcl;
    }
    if(index.row() < 0 || index.row() >= mItems.size()) {
        return invalidAcl;
    }
    return *mItems[index.row()];
}

void RulesTableModel::removeItems(const QModelIndexList& indexes)
{
    QList<int> rowsToRemove;
    foreach(const QModelIndex& idx, indexes) {
        if(!idx.isValid()) continue;
        if(idx.row() < 0) continue;
        if(idx.row() >= mItems.size()) continue;
        rowsToRemove << idx.row();
    }

    if(rowsToRemove.size() == 0) return;
    qStableSort(rowsToRemove);

    beginResetModel();
    while(rowsToRemove.size() > 0) {
        mItems.removeAt(rowsToRemove.takeFirst());
        for(int i = 0; i < rowsToRemove.size(); i++) {
            rowsToRemove[i]--;
        }
    }
    endResetModel();
}

void RulesTableModel::removeItem(const QModelIndex& index)
{
    if(!index.isValid()) return;
    if(index.row() < 0 || index.row() >= mItems.size()) return;
    beginRemoveRows(QModelIndex(), index.row(), index.row());
    mItems.removeAt(index.row());
    endRemoveRows();
}

void RulesTableModel::setRules(const rules_list_t& items) {
    beginResetModel();
    mItems = items;
    endResetModel();
}

Qt::ItemFlags RulesTableModel::flags(const QModelIndex &index) const
{
    if(!index.isValid()) {
        return QAbstractTableModel::flags(index);
    }

    if(index.column() == POLICY || index.column() == OFF) {
        return (QAbstractTableModel::flags(index) | Qt::ItemIsEditable);
    }
    return QAbstractTableModel::flags(index);
}
