/*  This file is part of KDevelop
    Copyright 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "sessionlistmodel.h"

// KDevelopSessionsWatch
#include <kdevelopsessionswatch.h>

SessionListModel::SessionListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    KDevelopSessionsWatch::registerObserver(this);
}

SessionListModel::~SessionListModel()
{
    KDevelopSessionsWatch::unregisterObserver(this);
}


QHash<int, QByteArray> SessionListModel::roleNames() const
{
    auto roleNames = QAbstractListModel::roleNames();
    roleNames.insert(SessionIdRole, QByteArrayLiteral("sessionId"));
    return roleNames;
}

int SessionListModel::rowCount(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return m_sessionDataList.size();
    }

    return 0;
}

QVariant SessionListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_sessionDataList.size()) {
        return QVariant();
    }

    const KDevelopSessionData sessionData = m_sessionDataList.at(index.row());

    switch (role) {
        case Qt::DisplayRole: {
            return sessionData.description;
        case SessionIdRole:
            return sessionData.id;
        }
    }

    return QVariant();
}

void SessionListModel::setSessionDataList(const QVector<KDevelopSessionData>& sessionDataList)
{
    beginResetModel();
    m_sessionDataList = sessionDataList;
    endResetModel();
}


void SessionListModel::openSession(const QString& sessionId)
{
    KDevelopSessionsWatch::openSession(sessionId);
}
