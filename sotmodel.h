#ifndef SOTMODEL_H
#define SOTMODEL_H

#include <QAbstractItemModel>

class SotFile;
class SotModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit SotModel(SotFile & f, QObject *parent = 0);

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
private:
    SotFile & m_data;
};

#endif // SOTMODEL_H
