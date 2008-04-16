#ifndef TREEMODEL_H
#define TREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QVector>
#include <QString>

namespace GDBDebugger {

class TreeItem;

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    TreeModel(const QVector<QString>& headers,  QObject *parent = 0);
    void setRootItem(TreeItem *item);
    ~TreeModel();

    void expanded(const QModelIndex &index);
    void collapsed(const QModelIndex &index);
    void clicked(const QModelIndex &index);

    void setEditable(bool);

public: // QAbstractItemModel overrides
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    bool setData(const QModelIndex& index, const QVariant& value,
                 int role);

public:
    TreeItem* itemForIndex(const QModelIndex& index) const;
    QModelIndex indexForItem(TreeItem *item, int column) const;

    using QAbstractItemModel::beginInsertRows;
    using QAbstractItemModel::endInsertRows;
    using QAbstractItemModel::beginRemoveRows;
    using QAbstractItemModel::endRemoveRows;
    using QAbstractItemModel::dataChanged;

private:
    QVector<QString> headers_;
    TreeItem *root_;
    bool editable_;
};

}

#endif
