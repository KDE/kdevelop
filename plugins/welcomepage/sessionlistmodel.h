/*
    SPDX-FileCopyrightText: 2024 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SESSIONLISTMODEL_H
#define SESSIONLISTMODEL_H

// KDevPlatform
#include <shell/session.h>
// Qt
#include <QAbstractListModel>

class SessionListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum {
        SessionIdRole = Qt::UserRole,
    };

    explicit SessionListModel(QObject* parent = nullptr);

public: // QAbstractListModel API
    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& index) const override;
    QHash<int, QByteArray> roleNames() const override;

public:
    int size() const;

Q_SIGNALS:
    void sizeChanged(int size);

private Q_SLOTS:
    void onSessionDeleted(const QString& id);

private:
    KDevelop::SessionInfos m_sessions;
};

#endif
