/*
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
