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

#ifndef SESSIONSMODEL_H
#define SESSIONSMODEL_H

#include <QModelIndex>

class SessionsModel : public QAbstractListModel
{
    Q_OBJECT
    public:
        enum Roles { Uuid = Qt::UserRole+1, Projects, ProjectNames, VisibleIdentifier };
        explicit SessionsModel(QObject* parent = 0);
        virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
        virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
        Q_SCRIPTABLE void loadSession(const QString& nameOrId) const;
    private:
        QList<KDevelop::SessionInfo> m_sessions;

    private slots:
        void sessionDeleted(const QString& id);
};

#endif // SESSIONSMODEL_H
