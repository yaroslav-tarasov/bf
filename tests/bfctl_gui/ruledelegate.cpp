#include "ruledelegate.h"

#include <QComboBox>
#include <QApplication>

#include "trx_data.h"

RuleDelegate::RuleDelegate()
{
}


QWidget *RuleDelegate::createEditor(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == 7) {
        QComboBox *pe = new QComboBox(editor);
        int current = index.model()->data(index, Qt::DisplayRole).toUInt();
        QStringList lst;
        lst << "NO" << "YES";
        if (pe) {
            pe->addItems(lst);
            pe->setCurrentIndex(current);
        }
        return pe;
    }
//    else if (index.column() == 1) { // для второго столбца используем QTimeEdit
//        QComboBox *editor = new QComboBox(parent);
//        QString currentText = index.model()->data(index, Qt::DisplayRole).toString();
//        editor->setText(currentText);
//        return editor;
    return editor;
}

void RuleDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (index.column() == 7) {
        QComboBox *pe = qobject_cast<QComboBox *>(editor);
        if (pe) {
            int value = index.model()->data(index, Qt::EditRole).toUInt();
            pe->setCurrentIndex(value);
        }
    }
//    else if (index.column() == 1) {
//        QComboBox *editor = qobject_cast<QComboBox *>(editor);
//        if (editor) {
//            editor->setDate(QDate::fromString(index.model()->data(index, Qt::EditRole).toString(),"d.MM.yyyy"));
//        }
//    }

}

void RuleDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (index.column() == 7) {
        QComboBox *pe = qobject_cast<QComboBox *>(editor);
        if (pe) {
            model->setData(index, pe->currentIndex());
        }
    }

}

void RuleDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

void RuleDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  QStyleOptionViewItemV4 o = option;
  // QString text = Items[index.row()].c_str();
  o.text = index.model()->data(index, Qt::DisplayRole).toString();

  int value = index.model()->data(index, Qt::DisplayRole).toUInt();

  switch(index.column()) {
      // case 0: o.text = QHostAddress(static_cast<quint32>(htonl(value))).toString(); break;
      // case 1: return value;
      // case 2: o.text = QHostAddress(static_cast<quint32>(htonl(value))).toString(); break;
      // case 3: return value;
      case 4: o.text = get_proto_name(value); break;
      case 5: o.text = get_chain_name(static_cast<bf_chain_t>(value));  break;
      case 6: o.text = get_policy_name(static_cast<bf_policy_t>(value));  break;
      case 7: o.text = get_sw_name(static_cast<bf_switch_rules_t>(value));  break;
  }

  QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &o, painter);
}
