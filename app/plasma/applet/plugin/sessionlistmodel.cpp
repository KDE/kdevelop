/*
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

#include "moc_sessionlistmodel.cpp"
