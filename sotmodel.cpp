#include "sotmodel.h"
#include "sotfile.h"
#include "Value.h"
#include "Variable.h"
#include "amf3vars.h"

SotModel::SotModel(SotFile & f, QObject *parent) :
    m_data(f),
    QAbstractItemModel(parent)
{
}

QVariant SotModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    //if (role != Qt::DisplayRole)
            return QVariant();

    //Variable * v = dynamic_cast<Variable*>(index.internalPointer());
    //return v->toString();
}

Qt::ItemFlags SotModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant SotModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return QString("%1 - amfv%2").arg(m_data.rootName(), m_data.version());

    return QVariant();
}

QModelIndex SotModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
             return QModelIndex();
#if 0
         TreeItem *parentItem;

         if (!parent.isValid())
             parentItem = rootItem;
         else
             parentItem = static_cast<TreeItem*>(parent.internalPointer());

         TreeItem *childItem = parentItem->child(row);
         if (childItem)
             return createIndex(row, column, childItem);
         else
#endif
             return QModelIndex();
}

QModelIndex SotModel::parent(const QModelIndex &index) const
{
 //   if (!index.isValid())
        return QModelIndex();
#if 0
    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
#endif
}

int SotModel::rowCount(const QModelIndex &parent) const
{
//    if (!parent.isValid()) { // root
//        return m_data.getTree().count();
//    } else { //child
//        Variable * v = dynamic_cast<Variable*>(parent.internalPointer());
//        if (v->isComplex()) {
//            amf3::array_type *at = dynamic_cast<amf3::array_type *>(v->value());
//            if (at)
//                return at->value().count();
//        }
        return 0;
//    }
}

int SotModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}
