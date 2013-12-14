/*
 * Copyright 2013 Kevin Funk <kevin@kfunk.org>
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
        , m_hintColumn(0)
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

    QVariant m_hint;
    int m_hintColumn;
};

PlaceholderItemProxyModel::PlaceholderItemProxyModel(QObject* parent)
    : KIdentityProxyModel(parent)
    , d(new Private(this))
{}

PlaceholderItemProxyModel::~PlaceholderItemProxyModel()
{
}

QVariant PlaceholderItemProxyModel::hint() const
{
    return d->m_hint;
}

void PlaceholderItemProxyModel::setHint(const QVariant& hint)
{
    d->m_hint = hint;

    const int row = d->sourceRowCount();
    emit dataChanged(index(row, 0), index(row, columnCount()));
}

int PlaceholderItemProxyModel::hintColumn() const
{
    return d->m_hintColumn;
}

void PlaceholderItemProxyModel::setHintColumn(int column)
{
    d->m_hintColumn = column;

    const int row = d->sourceRowCount();
    emit dataChanged(index(row, 0), index(row, columnCount()));
}

Qt::ItemFlags PlaceholderItemProxyModel::flags(const QModelIndex& index) const
{
    if (d->isPlaceholderRow(index)) {
        Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
        const int column = index.column();
        // if the column doesn't provide a hint we assume that we can't edit this field
        if (column == d->m_hintColumn) {
            flags |= Qt::ItemIsEditable;
        }
        return flags;
    }

    return KIdentityProxyModel::flags(index);
}

void PlaceholderItemProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
{
    KIdentityProxyModel::setSourceModel(sourceModel);
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
            if (column == d->m_hintColumn) {
                return d->m_hint;
            }
        case Qt::ForegroundRole: {
            const KColorScheme scheme(QPalette::Normal);
            return scheme.foreground(KColorScheme::InactiveText);
        }
        }
    }
    return KIdentityProxyModel::data(proxyIndex, role);
}

QModelIndex PlaceholderItemProxyModel::parent(const QModelIndex& child) const
{
    if (d->isPlaceholderRow(child)) {
        return QModelIndex();
    }

    return KIdentityProxyModel::parent(child);
}

QModelIndex PlaceholderItemProxyModel::buddy(const QModelIndex& index) const
{
    if (d->isPlaceholderRow(index)) {
        return index;
    }
    return KIdentityProxyModel::buddy(index);
}

QModelIndex PlaceholderItemProxyModel::mapToSource(const QModelIndex& proxyIndex) const
{
    if (d->isPlaceholderRow(proxyIndex)) {
        return QModelIndex();
    }
    return KIdentityProxyModel::mapToSource(proxyIndex);
}

bool PlaceholderItemProxyModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    const int column = index.column();
    if (d->isPlaceholderRow(index) && role == Qt::EditRole && column == d->m_hintColumn) {
        const bool accept = validateRow(index, value);
        // if validation fails, clear the complete line
        if (!accept) {
            emit dataChanged(index, index);
            return false;
        }

        // update view
        emit dataChanged(index, index);

        // notify observers
        emit dataInserted(value);
        return true;
    }
    return KIdentityProxyModel::setData(index, value, role);
}

QModelIndex PlaceholderItemProxyModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_ASSERT(!parent.isValid());
    Q_UNUSED(parent);

    const bool isPlaceHolderRow = (sourceModel() ? row == sourceModel()->rowCount() : false);
    if (isPlaceHolderRow) {
        return createIndex(row, column);
    }
    return KIdentityProxyModel::index(row, column, parent);
}

#include "moc_placeholderitemproxymodel.cpp"
