/*
    SPDX-FileCopyrightText: 2024 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "sessionlistmodel.h"

// KDevPlatform
#include <shell/core.h>
#include <shell/sessioncontroller.h>
// Std
#include <algorithm>

SessionListModel::SessionListModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_sessions(KDevelop::SessionController::availableSessionInfos())
{
    // TODO: SessionController misses a signal for new sessions, Sessions main menu would also want that
    connect(KDevelop::Core::self()->sessionController(), &KDevelop::SessionController::sessionDeleted, this,
            &SessionListModel::onSessionDeleted);
}

int SessionListModel::size() const
{
    return m_sessions.size();
}

QHash<int, QByteArray> SessionListModel::roleNames() const
{
    auto roleNames = QAbstractListModel::roleNames();
    roleNames.insert(SessionIdRole, QByteArrayLiteral("sessionId"));
    return roleNames;
}

int SessionListModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_sessions.size();
}

QVariant SessionListModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= m_sessions.size()) {
        return QVariant();
    }

    const auto& sessionInfo = m_sessions[index.row()];

    switch (role) {
    case Qt::DisplayRole:
        return sessionInfo.description;
    case SessionIdRole:
        return sessionInfo.uuid.toString();
    }

    return QVariant();
}

void SessionListModel::onSessionDeleted(const QString& id)
{
    auto it = std::find_if(m_sessions.cbegin(), m_sessions.cend(), [id](const KDevelop::SessionInfo& info) {
        return (info.uuid.toString() == id);
    });
    if (it == m_sessions.cend()) {
        return;
    }

    const auto i = static_cast<int>(std::distance(m_sessions.cbegin(), it));
    beginRemoveRows(QModelIndex(), i, i);
    m_sessions.removeAt(i);
    endRemoveRows();

    Q_EMIT sizeChanged(m_sessions.size());
}

#include "moc_sessionlistmodel.cpp"
