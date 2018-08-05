/*
 * This file is part of KDevelop
 *
 * Copyright 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "checklistmodel.h"

// plugin
#include <checkset.h>
#include <debug.h>

namespace ClangTidy
{

CheckListModel::CheckListModel(QObject* parent)
    : QAbstractItemModel(parent)
{
}

CheckListModel::~CheckListModel() = default;

QVariant CheckListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    const int c = index.row();
    if (0 > c || c >= m_checkSet->all().size()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return m_checkSet->all().at(c);
    }
    if (role == Qt::CheckStateRole) {
        const QString check = m_checkSet->all().at(c);
        return m_selectedChecks.contains(check) ? Qt::Checked : Qt::Unchecked;
    }

    return {};
}

bool CheckListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid()) {
        return false;
    }
    const int c = index.row();
    if (0 > c || c >= m_checkSet->all().size()) {
        return false;
    }

    if (role == Qt::CheckStateRole) {
        const QString check = m_checkSet->all().at(c);

        const bool selected = value.toBool();

        if (selected) {
            m_selectedChecks.append(check);
        } else {
            m_selectedChecks.removeAll(check);
        }
        m_isDefault = false;

        emit dataChanged(index, index);

        emit selectedChecksChanged();

        return true;
    }
    return true;
}

int CheckListModel::columnCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        return 1;
    }
    return 0;
}

int CheckListModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid()) {
        return m_checkSet ? m_checkSet->all().count() : 0;
    }

    return 0;
}

QModelIndex CheckListModel::parent(const QModelIndex& child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }
    return QModelIndex();
}

QModelIndex CheckListModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }
    return createIndex(row, column);
}

Qt::ItemFlags CheckListModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return  Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
}

void CheckListModel::setCheckSet(const CheckSet* checkSet)
{
    beginResetModel();

    m_checkSet = checkSet;

    if (m_isDefault) {
        m_selectedChecks = m_checkSet->defaults();
    }

    endResetModel();
}


QStringList CheckListModel::selectedChecks() const
{
    if (m_isDefault) {
        return QStringList();
    }

    // return normalized by sorting
    auto sortedChecks = m_selectedChecks;
    sortedChecks.sort();
    return sortedChecks;
}

void CheckListModel::setSelectedChecks(const QStringList& enabledChecks)
{
    beginResetModel();
    if (enabledChecks.isEmpty() && m_checkSet) {
        m_selectedChecks = m_checkSet->defaults();
        m_isDefault = true;
    } else {
        m_selectedChecks = enabledChecks;
    }

    endResetModel();
}

}
