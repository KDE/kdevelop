/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef SESSIONSMODEL_H
#define SESSIONSMODEL_H

#include <QModelIndex>
#include <shell/session.h>

class SessionsModel : public QAbstractListModel
{
    Q_OBJECT
    public:
        enum Roles { Uuid = Qt::UserRole+1, Projects, ProjectNames, VisibleIdentifier };
        explicit SessionsModel(QObject* parent = nullptr);
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        QHash< int, QByteArray > roleNames() const override;

        Q_SCRIPTABLE void loadSession(const QString& nameOrId) const;
    private:
        KDevelop::SessionInfos m_sessions;

    private Q_SLOTS:
        void sessionDeleted(const QString& id);
};

#endif // SESSIONSMODEL_H
