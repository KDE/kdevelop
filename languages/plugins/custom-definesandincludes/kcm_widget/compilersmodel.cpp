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

#include <KLocale>

namespace
{
enum Columns {
    NameColumn,
    PathColumn,
    NUM_COLUMNS
};
}

CompilersModel::CompilersModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

QVariant CompilersModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || (role != Qt::DisplayRole && role != Qt::EditRole)) {
        return QVariant();
    }

    if (index.row() < 0 || index.row() >= rowCount() || index.column() < 0 || index.column() >= columnCount()) {
        return QVariant();
    }

    if (index.row() < m_compilers.count()) {
        switch (index.column()) {
        case NameColumn:
            return m_compilers.at(index.row())->name();
        case PathColumn:
            return m_compilers.at(index.row())->path();
        default:
            Q_ASSERT(0);
            break;
        }
    }
    return QVariant();
}

int CompilersModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_compilers.count();
}

int CompilersModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return 2;
}

QVariant CompilersModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case NameColumn:
            return i18n("Name");
        case PathColumn:
            return i18n("Path");
        default:
            Q_ASSERT(0);
            break;
        }
    }
    return QVariant();
}

Qt::ItemFlags CompilersModel::flags(const QModelIndex& index) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return 0;
    }

    if (index.row() == m_compilers.count() && index.column() == 1) {
        return 0;
    }
    auto flgs = Qt::ItemIsSelectable;

    return m_compilers[index.row()]->editable() ? Qt::ItemFlags(flgs | Qt::ItemIsEditable | Qt::ItemIsEnabled) : flgs;
}

QVector< CompilerPointer > CompilersModel::compilers() const
{
    QVector<CompilerPointer> compilers;
    for (auto c: m_compilers) {
        if (!c->name().isEmpty() && !c->path().isEmpty()) {
            compilers.append(c);
        }
    }
    return compilers;
}

void CompilersModel::setCompilers(const QVector< CompilerPointer >& compilers)
{
    beginResetModel();
    m_compilers.clear();
    for (auto c: compilers) {
        if (c->factoryName().isEmpty()) {
            continue;
        }
        m_compilers.append(c);
    }
    endResetModel();
}

bool CompilersModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (row >= 0 && count > 0 && row < m_compilers.count()) {
        beginRemoveRows(parent, row, row + count - 1);
        for (int i = 0; i < count; ++i) {
            m_compilers.remove(row);
        }
        endRemoveRows();
        return true;
    }
    return false;
}

bool CompilersModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || role != Qt::EditRole) {
        return false;
    }
    if (index.row() < 0 || index.row() >= rowCount() || index.column() < 0 || index.column() >= columnCount()) {
        return false;
    }

    switch (index.column()) {
    case NameColumn:
        m_compilers[index.row()]->setName(value.toString());
        break;
    case PathColumn:
        m_compilers[index.row()]->setPath(value.toString());
        break;
    default:
        Q_ASSERT(0);
        return false;
    }

    emit dataChanged(index, index);
    return true;
}

void CompilersModel::addCompiler(const CompilerPointer& compiler)
{
    beginInsertRows({}, m_compilers.size(), m_compilers.size());
    Q_ASSERT(!compiler->factoryName().isEmpty());
    m_compilers.append(compiler);
    endInsertRows();
}
