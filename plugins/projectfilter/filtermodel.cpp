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
#include <KIcon>

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
        return i18n("Match Against");
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

    if (role != Qt::DisplayRole && role != Qt::DecorationRole
        && role != Qt::EditRole && role != Qt::ToolTipRole)
    {
        return QVariant();
    }

    const Filter& filter = m_filters.at(index.row());
    const int column = index.column();

    if (column == Pattern) {
        if (role == Qt::DecorationRole) {
            return QVariant();
        } else if (role == Qt::ToolTipRole) {
            return i18n(
                "The wildcard pattern defines whether a file or folder is included in a project or not.<br />"
                "The pattern is matched case-sensitively either against the items path relative to the project root"
                " or against the items basename. Relative paths start with a forward slash, trailing slashes of folders are removed.<br />"
                "To match any path ending on a given pattern e.g., use a filter matching the pattern \"*/foo/bar\" on the relative path."
            );
        } else if (role == Qt::EditRole && filter.pattern.isEmpty() && filter.matchOn == Filter::RelativePath) {
            // a sane default where the user can append to
            return QString("*/");
        }
        return filter.pattern.pattern();
    } else if (column == Targets) {
        if (role == Qt::EditRole) {
            return static_cast<int>(filter.targets);
        } else if (role == Qt::ToolTipRole) {
            return i18n("The target defines what type of item the filter is matched against.<br />Filters either apply only to files, only to folders or to both.");
        }
        if (filter.targets & Filter::Files && filter.targets & Filter::Folders) {
            if (role == Qt::DecorationRole) {
                return KIcon("document-open");
            }
            return i18n("files and folders");
        } else if (filter.targets & Filter::Folders) {
            if (role == Qt::DecorationRole) {
                return KIcon("folder");
            }
            return i18n("folders");
        } else {
            if (role == Qt::DecorationRole) {
                return KIcon("text-plain");
            }
            return i18n("files");
        }
    } else if (column == MatchOn) {
        if (role == Qt::EditRole) {
            return static_cast<int>(filter.matchOn);
        } else if (role == Qt::ToolTipRole) {
            return i18n(
                "The pattern can be matched either against the basename or the the path relative to the project root.<br/>"
                "Relative paths start with a forward slash. Trailing slashes are removed."
                "In most cases it is sufficient to match against the basename. For more fine grained control use the relative path.<br/>"
                "To match for example <code>\"foo\"</code> in a folder called <code>\"bar\"</code> anywhere in the project"
                " use a relative path filter with the pattern <code>\"*/bar/foo\"</code>."
            );
        }
        if (filter.matchOn == Filter::Basename) {
            return i18n("basename");
        } else {
            return i18n("relative path");
        }
    } else if (column == Inclusive) {
        if (role == Qt::EditRole) {
            return static_cast<int>(filter.type);
        } else if (role == Qt::ToolTipRole) {
            return i18n("Filters by default exclude items from the project. Inclusive patterns can be used to include items which where matched by previous exclusive patterns.<br />E.g. to only include files ending on <code>\".cpp\"</code> in your project, you could exclude all files via <code>\"*\"</code> and then apply an inclusive <code>\"*.cpp\"</code> pattern.");
        }
        if (filter.type == Filter::Inclusive) {
            if (role == Qt::DecorationRole) {
                return KIcon("list-add");
            }
            return i18n("inclusive");
        } else {
            if (role == Qt::DecorationRole) {
                return KIcon("list-remove");
            }
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
        filter.type = static_cast<Filter::Type>(value.toInt());
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
