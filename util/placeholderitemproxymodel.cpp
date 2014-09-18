/*
 * Copyright 2013 Kevin Funk <kfunk@kde.org>
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

#include "placeholderitemproxymodel.h"

#include <KColorScheme>

using namespace KDevelop;

struct PlaceholderItemProxyModel::Private
{
    Private(PlaceholderItemProxyModel* qq)
        : q(qq)
    {}

    inline int sourceRowCount()
    {
        return q->sourceModel() ? q->sourceModel()->rowCount() : 0;
    }

    inline bool isPlaceholderRow(const QModelIndex& index) const
    {
        if (!q->sourceModel()) {
            return false;
        }
        return index.row() == q->sourceModel()->rowCount();
    }

    PlaceholderItemProxyModel* const q;

    /// column -> hint mapping
    QMap<int, QVariant> m_columnHints;
};

PlaceholderItemProxyModel::PlaceholderItemProxyModel(QObject* parent)
    : QIdentityProxyModel(parent)
    , d(new Private(this))
{}

PlaceholderItemProxyModel::~PlaceholderItemProxyModel()
{
}

QVariant PlaceholderItemProxyModel::columnHint(int column) const
{
    return d->m_columnHints.value(column);
}

void PlaceholderItemProxyModel::setColumnHint(int column, const QVariant& hint)
{
    if (column < 0) {
        return;
    }

    d->m_columnHints[column] = hint;

    const int row = d->sourceRowCount();
    emit dataChanged(index(row, 0), index(row, columnCount()));
}

Qt::ItemFlags PlaceholderItemProxyModel::flags(const QModelIndex& index) const
{
    if (d->isPlaceholderRow(index)) {
        Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
        const int column = index.column();
        // if the column doesn't provide a hint we assume that we can't edit this field
        if (d->m_columnHints.contains(column)) {
            flags |= Qt::ItemIsEditable;
        }
        return flags;
    }

    return QIdentityProxyModel::flags(index);
}

void PlaceholderItemProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
{
    QIdentityProxyModel::setSourceModel(sourceModel);
    // TODO: Listen to layoutDataChanged signals?
}

int PlaceholderItemProxyModel::rowCount(const QModelIndex& parent) const
{
    if (!sourceModel())
        return 0;

    // only flat models supported for now, assert early in case that's not true
    Q_ASSERT(!parent.isValid());
    Q_UNUSED(parent);
    return sourceModel()->rowCount() + 1;
}

QVariant PlaceholderItemProxyModel::data(const QModelIndex& proxyIndex, int role) const
{
    const int column = proxyIndex.column();
    if (d->isPlaceholderRow(proxyIndex)) {
        switch (role) {
        case Qt::DisplayRole:
            return columnHint(column);
        case Qt::ForegroundRole: {
            const KColorScheme scheme(QPalette::Normal);
            return scheme.foreground(KColorScheme::InactiveText);
        }
        default:
            return QVariant();
        }
    }
    return QIdentityProxyModel::data(proxyIndex, role);
}

QModelIndex PlaceholderItemProxyModel::parent(const QModelIndex& child) const
{
    if (d->isPlaceholderRow(child)) {
        return QModelIndex();
    }

    return QIdentityProxyModel::parent(child);
}

QModelIndex PlaceholderItemProxyModel::buddy(const QModelIndex& index) const
{
    if (d->isPlaceholderRow(index)) {
        return index;
    }
    return QIdentityProxyModel::buddy(index);
}

QModelIndex PlaceholderItemProxyModel::sibling(int row, int column, const QModelIndex& idx) const
{
    const bool isPlaceHolderRow = (sourceModel() ? row == sourceModel()->rowCount() : false);
    if (isPlaceHolderRow) {
        return index(row, column, QModelIndex());
    }
    return QIdentityProxyModel::sibling(row, column, idx);
}


QModelIndex PlaceholderItemProxyModel::mapToSource(const QModelIndex& proxyIndex) const
{
    if (d->isPlaceholderRow(proxyIndex)) {
        return QModelIndex();
    }
    return QIdentityProxyModel::mapToSource(proxyIndex);
}

bool PlaceholderItemProxyModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    const int column = index.column();
    if (d->isPlaceholderRow(index) && role == Qt::EditRole && d->m_columnHints.contains(column)) {
        const bool accept = validateRow(index, value);
        // if validation fails, clear the complete line
        if (!accept) {
            emit dataChanged(index, index);
            return false;
        }

        // update view
        emit dataChanged(index, index);

        // notify observers
        emit dataInserted(column, value);
        return true;
    }
    return QIdentityProxyModel::setData(index, value, role);
}

QModelIndex PlaceholderItemProxyModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_ASSERT(!parent.isValid());
    Q_UNUSED(parent);

    const bool isPlaceHolderRow = (sourceModel() ? row == sourceModel()->rowCount() : false);
    if (isPlaceHolderRow) {
        return createIndex(row, column);
    }
    return QIdentityProxyModel::index(row, column, parent);
}

bool PlaceholderItemProxyModel::validateRow(const QModelIndex& index, const QVariant& value) const
{
    Q_UNUSED(index);
    return !value.toString().isEmpty();
}

#include "moc_placeholderitemproxymodel.cpp"
