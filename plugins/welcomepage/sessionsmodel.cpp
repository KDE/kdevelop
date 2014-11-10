/* This file is part of KDevelop
    Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "sessionsmodel.h"
#include <shell/core.h>
#include <shell/sessioncontroller.h>

using namespace KDevelop;

SessionsModel::SessionsModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_sessions(KDevelop::SessionController::availableSessionInfo())
{
    connect(Core::self()->sessionController(), &SessionController::sessionDeleted, this, &SessionsModel::sessionDeleted);
}

QHash< int, QByteArray > SessionsModel::roleNames() const
{
    QHash< int, QByteArray > roles = QAbstractListModel::roleNames();
    roles.insert(Uuid, "uuid");
    roles.insert(Projects, "projects");
    roles.insert(ProjectNames, "projectNames");
    roles.insert(VisibleIdentifier, "identifier");
    return roles;
}

QVariant SessionsModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid() || index.row()>m_sessions.count()) {
        return QVariant();
    }
    
    switch(role) {
        case Qt::DisplayRole:
            return m_sessions[index.row()].name;
        case Qt::ToolTip:
            return m_sessions[index.row()].description;
        case Uuid:
            return m_sessions[index.row()].uuid.toString();
        case Projects:
            return QVariant::fromValue(m_sessions[index.row()].projects);
        case VisibleIdentifier: {
            const KDevelop::SessionInfo& s = m_sessions[index.row()];
            return s.name.isEmpty() && !s.projects.isEmpty() ? s.projects.first().fileName() : s.name;
        }
        case ProjectNames: {
            QVariantList ret;
            foreach(const QUrl& project, m_sessions[index.row()].projects) {
                ret += project.fileName();
            }
            return ret;
        }
    }
    return QVariant();
}

int SessionsModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_sessions.size();
}

void SessionsModel::loadSession(const QString& nameOrId) const
{
    KDevelop::Core::self()->sessionController()->loadSession(nameOrId);
}

void SessionsModel::sessionDeleted(const QString& id)
{
    for(int i=0; i<m_sessions.size(); i++) {
        if(m_sessions[i].uuid.toString()==id) {
            beginRemoveRows(QModelIndex(), i, i);
            m_sessions.removeAt(i);
            endRemoveRows();
            break;
        }
    }
}
