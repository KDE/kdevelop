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

#ifndef PATHSMODEL_H
#define PATHSMODEL_H

#include "clangsettingsmanager.h"
#include <util/path.h>

#include <QAbstractListModel>

class PathsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum SpecialRoles {
        ParserOptionsRole = Qt::UserRole + 1
    };

    PathsModel(QObject* parent = 0);
    void setPaths(const QVector<ParserSettingsEntry>& paths);
    QVector<ParserSettingsEntry> paths() const;
    void addPath(const QString& path);
    void setProjectPath(const KDevelop::Path& path);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

signals:
    void changed();

private:
    void addPathInternal(const QString& path);

    QVector<ParserSettingsEntry> m_paths;
    KDevelop::Path m_projectPath;
};

#endif // PATHSMODEL_H
