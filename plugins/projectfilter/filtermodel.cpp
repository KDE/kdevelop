/*
 * This file is part of KDevelop
 * Copyright 2013 Milian Wolff <mail@milianw.de>
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
 */

#include "filtermodel.h"

#include <QDebug>

#include <KLocalizedString>

#include "tests/modeltest.h"

using namespace KDevelop;

FilterModel::FilterModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    new ModelTest(this, this);
}

FilterModel::~FilterModel()
{

}

Filters FilterModel::filters() const
{
    return m_filters;
}

void FilterModel::setFilters(const Filters& filters)
{
    beginResetModel();
    m_filters = filters;
    endResetModel();
}

void FilterModel::addFilter()
{
    beginInsertRows(QModelIndex(), m_filters.size(), m_filters.size());
    m_filters << Filter();
    endInsertRows();
}

void FilterModel::removeFilter(int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    m_filters.remove(row);
    endRemoveRows();
}

void FilterModel::moveFilterUp(int row)
{
    beginMoveRows(QModelIndex(), row, row, QModelIndex(), row - 1);
    qSwap(m_filters[row], m_filters[row - 1]);
    endMoveRows();
}

void FilterModel::moveFilterDown(int row)
{
    beginMoveRows(QModelIndex(), row, row, QModelIndex(), row + 2);
    qSwap(m_filters[row], m_filters[row + 1]);
    endMoveRows();
}

int FilterModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_filters.size();
}

int FilterModel::columnCount(const QModelIndex& /*parent*/) const
{
    return NUM_COLUMNS;
}

QVariant FilterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QVariant();
    }

    Q_ASSERT(section >= 0 && section < NUM_COLUMNS);

    if (section == Pattern) {
        return i18n("Pattern");
    } else if (section == Targets) {
        return i18n("Targets");
    } else if (section == MatchOn) {
        return i18n("Match On");
    } else if (section == Inclusive) {
        return i18n("Type");
    }

    return QVariant();
}

QVariant FilterModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    Q_ASSERT(!index.parent().isValid());
    Q_ASSERT(index.row() >= 0 && index.row() < m_filters.size());
    Q_ASSERT(index.column() >= 0 && index.column() < NUM_COLUMNS);

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    const Filter& filter = m_filters.at(index.row());
    const int column = index.column();

    if (column == Pattern) {
        return filter.pattern.pattern();
    } else if (column == Targets) {
        if (filter.targets & Filter::Files && filter.targets & Filter::Folders) {
            return i18n("files and folders");
        } else if (filter.targets & Filter::Folders) {
            return i18n("folders");
        } else {
            return i18n("files");
        }
    } else if (column == MatchOn) {
        if (filter.matchOn == Filter::Basename) {
            return i18n("basename");
        } else {
            return i18n("relative path");
        }
    } else if (column == Inclusive) {
        if (filter.inclusive) {
            return i18n("inclusive");
        } else {
            return i18n("exclusive");
        }
    }

    return QVariant();
}

bool FilterModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid()) {
        return false;
    }
    Q_ASSERT(!index.parent().isValid());
    Q_ASSERT(index.row() >= 0 && index.row() < m_filters.size());
    Q_ASSERT(index.column() >= 0 && index.column() < NUM_COLUMNS);
    if (role != Qt::EditRole) {
        return false;
    }
    Filter& filter = m_filters[index.row()];
    const int column = index.column();
    if (column == Pattern) {
        filter.pattern.setPattern(value.toString());
    } else if (column == MatchOn) {
        filter.matchOn = static_cast<Filter::MatchOn>(value.toInt());
    } else if (column == Targets) {
        filter.targets = static_cast<Filter::Targets>(value.toInt());
    } else if (column == Inclusive) {
        filter.inclusive = value.toBool();
    }
    dataChanged(index, index);
    return true;
}

Qt::ItemFlags FilterModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags baseFlags = QAbstractTableModel::flags(index);
    if (index.isValid() && !index.parent().isValid()) {
        return baseFlags | Qt::ItemIsEditable;
    }
    return baseFlags;
}

#include "filtermodel.moc"
