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
#include <QIcon>

#include <KLocalizedString>

using namespace KDevelop;

FilterModel::FilterModel(QObject* parent)
    : QAbstractTableModel(parent)
    , m_ignoredLastInsert(false)
{
}

FilterModel::~FilterModel()
{

}

SerializedFilters FilterModel::filters() const
{
    return m_filters;
}

void FilterModel::setFilters(const SerializedFilters& filters)
{
    beginResetModel();
    m_filters = filters;
    endResetModel();
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
    } else if (section == Inclusive) {
        return i18n("Action");
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

    if (role != Qt::DisplayRole && role != Qt::DecorationRole
        && role != Qt::EditRole && role != Qt::ToolTipRole)
    {
        return QVariant();
    }

    const SerializedFilter& filter = m_filters.at(index.row());
    const int column = index.column();

    if (column == Pattern) {
        if (role == Qt::DecorationRole) {
            return QVariant();
        } else if (role == Qt::ToolTipRole) {
            return i18n(
                "The wildcard pattern defines whether a file or folder is included in a project or not.<br />"
                "The pattern is matched case-sensitively against the items relative path to the project root. "
                "The relative path starts with a forward slash, trailing slashes of folders are removed.<br />"
                "Patterns ending on <code>\"/\"</code> are implicitly considered to match against folders only.<br />"
                "Patterns which do not explicitly start with either <code>\"/\"</code> or <code>\"*\"</code> implicitly get <code>\"*/\"</code> prepended and thus match any item with a relative path ending on the given pattern."
            );
        }
        return filter.pattern;
    } else if (column == Targets) {
        if (role == Qt::EditRole) {
            return static_cast<int>(filter.targets);
        } else if (role == Qt::ToolTipRole) {
            return i18n("The target defines what type of item the filter is matched against.<br />Filters either apply only to files, only to folders or to both.");
        }
        if (filter.targets & Filter::Files && filter.targets & Filter::Folders) {
            if (role == Qt::DecorationRole) {
                return QIcon::fromTheme("document-open");
            }
            return i18n("Files and Folders");
        } else if (filter.targets & Filter::Folders) {
            if (role == Qt::DecorationRole) {
                return QIcon::fromTheme("folder");
            }
            return i18n("Folders");
        } else {
            if (role == Qt::DecorationRole) {
                return QIcon::fromTheme("text-plain");
            }
            return i18n("Files");
        }
    } else if (column == Inclusive) {
        if (role == Qt::EditRole) {
            return static_cast<int>(filter.type);
        } else if (role == Qt::ToolTipRole) {
            return i18n("Filters by default exclude items from the project. Inclusive patterns can be used to include items which where matched by previous exclusive patterns.<br />E.g. to only include files ending on <code>\".cpp\"</code> in your project, you could exclude all files via <code>\"*\"</code> and then apply an inclusive <code>\"*.cpp\"</code> pattern.");
        }
        if (filter.type == Filter::Inclusive) {
            if (role == Qt::DecorationRole) {
                return QIcon::fromTheme("list-add");
            }
            return i18n("Include");
        } else {
            if (role == Qt::DecorationRole) {
                return QIcon::fromTheme("list-remove");
            }
            return i18n("Exclude");
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
    if (role != Qt::EditRole && role != Qt::DisplayRole) {
        return false;
    }
    SerializedFilter& filter = m_filters[index.row()];
    const int column = index.column();
    if (column == Pattern) {
        filter.pattern = value.toString();
    } else if (column == Targets) {
        filter.targets = static_cast<Filter::Targets>(value.toInt());
    } else if (column == Inclusive) {
        filter.type = static_cast<Filter::Type>(value.toInt());
    }
    dataChanged(index, index);
    return true;
}

Qt::DropActions FilterModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

Qt::ItemFlags FilterModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags baseFlags = QAbstractTableModel::flags(index);
    if (index.isValid() && !index.parent().isValid()) {
        return baseFlags | Qt::ItemIsEditable | Qt::ItemIsDragEnabled;
    }
    return baseFlags | Qt::ItemIsDropEnabled;
}

bool FilterModel::insertRows(int row, int count, const QModelIndex& parent)
{
    Q_ASSERT(!parent.isValid());
    Q_ASSERT(count == 1);
    if (row == -1) {
        // after end of list and we cannot just append either as then the
        // later setData events will fails...
        m_ignoredLastInsert = true;
        return false;
    }
    m_ignoredLastInsert = false;
    Q_ASSERT(row >= 0 && row <= m_filters.size());
    Q_ASSERT(row + count - 1 <= m_filters.size());

    beginInsertRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        m_filters.insert(row + i, SerializedFilter());
    }
    endInsertRows();
    return true;
}

bool FilterModel::removeRows(int row, int count, const QModelIndex& parent)
{
    Q_ASSERT(!parent.isValid());
    Q_ASSERT(count == 1);
    Q_ASSERT(row >= 0 && row < m_filters.size());
    Q_ASSERT(row + count <= m_filters.size());

    if (m_ignoredLastInsert) {
        return false;
    }

    beginRemoveRows(parent, row, row + count - 1);
    m_filters.remove(row, count);
    endRemoveRows();

    return true;
}

QMap<int, QVariant> FilterModel::itemData(const QModelIndex& index) const
{
    QMap<int, QVariant> ret;
    if (!index.isValid()) {
        return ret;
    }
    Q_ASSERT(!index.parent().isValid());
    Q_ASSERT(index.row() >= 0 && index.row() < m_filters.size());
    const SerializedFilter& filter = m_filters.at(index.row());
    ret.insert(Qt::UserRole + Pattern, filter.pattern);
    ret.insert(Qt::UserRole + Inclusive, static_cast<int>(filter.type));
    ret.insert(Qt::UserRole + Targets, static_cast<int>(filter.targets));
    return ret;
}

bool FilterModel::setItemData(const QModelIndex& index, const QMap< int, QVariant >& roles)
{
    Q_ASSERT(index.isValid());
    Q_ASSERT(!index.parent().isValid());
    Q_ASSERT(index.row() >= 0 && index.row() < m_filters.size());
    Q_ASSERT(roles.size() == 3);
    Q_ASSERT(roles.contains(Qt::UserRole + Pattern));
    Q_ASSERT(roles.contains(Qt::UserRole + Inclusive));
    Q_ASSERT(roles.contains(Qt::UserRole + Targets));

    if (m_ignoredLastInsert) {
        return false;
    }

    SerializedFilter& filter = m_filters[index.row()];
    filter.pattern = roles[Qt::UserRole + Pattern].toString();
    filter.type = Filter::Type(roles[Qt::UserRole + Inclusive].toInt());
    filter.targets = Filter::Targets(roles[Qt::UserRole + Targets].toInt());
    return true;
}

