/*
 * This file is part of KDevelop
 *
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "compilersmodel.h"

#include <KLocalizedString>
#include <QVariant>

#include "../compilerprovider/icompilerprovider.h"


//Represents a single row in the table
class TreeItem
{
public:
    TreeItem(const QList<QVariant> &data, TreeItem *parent = nullptr)
        :m_itemData(data)
        ,m_parentItem(parent)
    {}

    virtual ~TreeItem()
    {
        removeChilds();
    }

    void appendChild(TreeItem *item)
    {
        m_childItems.append(item);
    }

    void removeChild(int row)
    {
        m_childItems.removeAt(row);
    }

    TreeItem *child(int row)
    {
        return m_childItems.value(row);
    }

    int childCount() const
    {
        return m_childItems.count();
    }

    int columnCount() const
    {
        return m_itemData.count();
    }

    virtual QVariant data(int column) const
    {
        return m_itemData.value(column);
    }

    TreeItem *parent()
    {
        return m_parentItem;
    }

    int row() const
    {
        if (m_parentItem) {
            return m_parentItem->m_childItems.indexOf(const_cast<TreeItem*>(this));
        }

        return 0;
    }

    void removeChilds()
    {
        qDeleteAll(m_childItems);
        m_childItems.clear();
    }

private:
    QList<TreeItem*> m_childItems;
    QList<QVariant> m_itemData;
    TreeItem *m_parentItem;
};

class CompilerItem : public TreeItem {
public:
    CompilerItem(const CompilerPointer& compiler, TreeItem* parent)
        : TreeItem(QList<QVariant>{compiler->name(), compiler->factoryName()}, parent)
    , m_compiler(compiler)
    {}

    CompilerPointer compiler()
    {
        return m_compiler;
    }

    virtual QVariant data(int column) const override
    {
        return !column ? m_compiler->name() : m_compiler->factoryName();
    }

private:
    CompilerPointer m_compiler;
};

namespace {
TreeItem* autoDetectedRootItem(TreeItem* root)
{
    return root->child(0);
}

TreeItem* manualRootItem(TreeItem* root)
{
    return root->child(1);
}
}

CompilersModel::CompilersModel(QObject* parent)
    : QAbstractItemModel(parent)
    , m_rootItem(new TreeItem( QList<QVariant>{i18n("Name"), i18n("Type")}))
{
    m_rootItem->appendChild(new TreeItem( QList<QVariant>{i18n("Auto-detected"), QString()}, m_rootItem));
    m_rootItem->appendChild(new TreeItem( QList<QVariant>{i18n("Manual"), QString()}, m_rootItem));
}

QVariant CompilersModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || (role != Qt::DisplayRole && role != CompilerDataRole)) {
        return QVariant();
    }

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    if (role == CompilerDataRole) {
        QVariant v;
        if (auto c = dynamic_cast<CompilerItem*>(item)) {
            if (item->parent() == manualRootItem(m_rootItem)) {
                v.setValue<CompilerPointer>(c->compiler());
            }
        }
        return v;
    }

    return item->data(index.column());
}

int CompilersModel::rowCount(const QModelIndex& parent) const
{
    TreeItem *parentItem;
    if (parent.column() > 0) {
        return 0;
    }

    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = static_cast<TreeItem*>(parent.internalPointer());
    }

    return parentItem->childCount();
}

int CompilersModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
    } else {
        return m_rootItem->columnCount();
    }
}

QVariant CompilersModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return m_rootItem->data(section);
    }
    return QVariant();
}

Qt::ItemFlags CompilersModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return 0;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex CompilersModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    TreeItem *parentItem;

    if (!parent.isValid()) {
        parentItem = m_rootItem;
    } else {
        parentItem = static_cast<TreeItem*>(parent.internalPointer());
    }

    TreeItem* childItem = parentItem->child(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    } else {
        return QModelIndex();
    }
}

QModelIndex CompilersModel::parent(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parent();

    if (parentItem == m_rootItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

QVector< CompilerPointer > CompilersModel::compilers() const
{
    QVector<CompilerPointer> compilers;
    for (int idx = 0; idx < 2; idx++) {
        for (int i = 0; i< m_rootItem->child(idx)->childCount(); i++) {
            auto compiler = static_cast<CompilerItem*>(m_rootItem->child(idx)->child(i))->compiler();
            if (!compiler->name().isEmpty() && !compiler->path().isEmpty()) {
                compilers.append(compiler);
            }
        }
    }

    return compilers;
}

void CompilersModel::setCompilers(const QVector< CompilerPointer >& compilers)
{
    beginResetModel();
    autoDetectedRootItem(m_rootItem)->removeChilds();
    manualRootItem(m_rootItem)->removeChilds();

    for (auto compiler: compilers) {
        if (compiler->factoryName().isEmpty()) {
            continue;
        }
        TreeItem* parent = autoDetectedRootItem(m_rootItem);
        if (compiler->editable()) {
            parent = manualRootItem(m_rootItem);
        }
        parent->appendChild(new CompilerItem(compiler, parent));
    }

    endResetModel();
}

QModelIndex CompilersModel::addCompiler(const CompilerPointer& compiler)
{
    beginInsertRows(index(1, 0), manualRootItem(m_rootItem)->childCount(), manualRootItem(m_rootItem)->childCount());
    Q_ASSERT(!compiler->factoryName().isEmpty());
    manualRootItem(m_rootItem)->appendChild(new CompilerItem(compiler, manualRootItem(m_rootItem)));
    endInsertRows();

    emit compilerChanged();
    return index(manualRootItem(m_rootItem)->childCount()-1, 0, index(1, 0));
}

bool CompilersModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (row >= 0 && count > 0 &&  parent.isValid() && static_cast<TreeItem*>(parent.internalPointer()) == manualRootItem(m_rootItem)) {
        beginRemoveRows(parent, row, row + count - 1);
        for (int i = 0; i < count; ++i) {
            manualRootItem(m_rootItem)->removeChild(row);
        }
        endRemoveRows();

        emit compilerChanged();
        return true;
    }
    return false;
}

void CompilersModel::updateCompiler(const QItemSelection& compiler)
{
    for (const auto& idx: compiler.indexes()) {
        emit dataChanged(idx, idx);
    }
    emit compilerChanged();
}
