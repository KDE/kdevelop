/*
 * This file is part of KDevelop
 *
 * Copyright 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>
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

#include "pathsmodel.h"

#include "clangsettingsmanager.h"

#include <KLocalizedString>

#include <QFileInfo>

using namespace KDevelop;

namespace
{
const QString projectRoot = QStringLiteral("(project root)");
const QString dotPath = QStringLiteral(".");

QString toRelativePath(const QString& path, const Path& projectPath)
{
    auto relativePath = projectPath.relativePath(KDevelop::Path(path));
    return relativePath.isEmpty() ? dotPath : relativePath;
}
}

PathsModel::PathsModel(QObject* parent)
    : QAbstractListModel(parent)
{}

QVariant PathsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || (role != Qt::DisplayRole && role != Qt::EditRole && role != ParserOptionsRole)) {
        return {};
    }

    if (index.row() < 0 || index.row() >= rowCount() || index.column() != 0) {
        return {};
    }

    if (index.row() == m_paths.count()) {
        return {};
    }

    if(role == ParserOptionsRole){
        return QVariant::fromValue(m_paths.at(index.row()).settings);
    }

    const auto path = m_paths.at(index.row()).path;
    return path == dotPath ? projectRoot : path;
}

int PathsModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_paths.count();
}

bool PathsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || (role != Qt::EditRole && role != ParserOptionsRole)) {
        return false;
    }

    if (index.row() < 0 || index.row() >= rowCount() || index.column() != 0) {
        return false;
    }

    if (role == ParserOptionsRole) {
        auto settings = value.value<ParserSettings>();
        m_paths[index.row()].settings = settings;
    } else {
        auto path = value.toString().trimmed();
        if (path.isEmpty()) {
            return false;
        }

        m_paths[index.row()].path = (path == projectRoot) ? dotPath : path;
    }

    emit dataChanged(index, index);
    emit changed();
    return true;
}

Qt::ItemFlags PathsModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return 0;
    }

    return Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

QVector<ParserSettingsEntry> PathsModel::paths() const
{
    return m_paths;
}

void PathsModel::setPaths(const QVector<ParserSettingsEntry>& paths)
{
    beginResetModel();
    m_paths = paths;
    endResetModel();
}

bool PathsModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (row < 0 || count <= 0 || row >= m_paths.count()) {
        return false;
    }

    beginRemoveRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        if (m_paths[row].path != QStringLiteral(".")) {
            m_paths.removeAt(row);
        }
    }

    endRemoveRows();
    emit changed();
    return true;
}

void PathsModel::addPath(const QString& path)
{
    QFileInfo info(path);
    auto canonicalPath = info.canonicalFilePath();
    if (!m_projectPath.isParentOf(KDevelop::Path(canonicalPath))) {
        return;
    }

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    addPathInternal(canonicalPath);
    endInsertRows();
    emit changed();
}

void PathsModel::addPathInternal(const QString& path)
{
    if (path.isEmpty()) {
        return;
    }

    auto relativePath = toRelativePath(path, m_projectPath);

    for (const auto& existingConfig : m_paths) {
        if (relativePath == existingConfig.path) {
            return;
        }
    }

    m_paths.append({ClangSettingsManager::self()->defaultParserSettings(), relativePath});
}

void PathsModel::setProjectPath(const KDevelop::Path& path)
{
    m_projectPath = path;
}
