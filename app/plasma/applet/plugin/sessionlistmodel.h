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

#ifndef SESSIONLISTMODEL_H
#define SESSIONLISTMODEL_H

// KDevelopSessionsWatch
#include <kdevelopsessionsobserver.h>
// Qt
#include <QAbstractListModel>


class SessionListModel : public QAbstractListModel
                       , public KDevelopSessionsObserver
{
    Q_OBJECT
    Q_INTERFACES(KDevelopSessionsObserver)

public:
    enum {
        SessionIdRole = Qt::UserRole,
    };

public:
    explicit SessionListModel(QObject* parent = nullptr);
    ~SessionListModel() override;

public: // QAbstractListModel API
    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& index) const override;
    QHash<int, QByteArray> roleNames() const override;

public Q_SLOTS: // KDevelopSessionsObserver API
    void setSessionDataList(const QVector<KDevelopSessionData>& sessionDataList) override;

public:
    Q_INVOKABLE void openSession(const QString& sessionId);

private:
    // working copy of the list
    QVector<KDevelopSessionData> m_sessionDataList;
};

#endif
