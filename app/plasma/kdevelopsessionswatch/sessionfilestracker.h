/*
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVELOPSESSIONSWATCH_SESSIONFILESTRACKER_H
#define KDEVELOPSESSIONSWATCH_SESSIONFILESTRACKER_H

// lib
#include "kdevelopsessiondata.h"
// Qt
#include <QObject>
#include <QVector>
#include <QMutex>

class KDirWatch;

class SessionFilesTracker : public QObject
{
    Q_OBJECT

public:
    static SessionFilesTracker *instance();

public:
    SessionFilesTracker();
    ~SessionFilesTracker() override;

public:
    void registerObserver(QObject* observer);
    void unregisterObserver(QObject* observer);

    void cleanup();

Q_SIGNALS:
    void sessionDataListChanged(const QVector<KDevelopSessionData>& sessionDataList);

private Q_SLOTS:
    void sessionSourceChanged(const QString& path);

private:
    void updateSessions();
    QVector<KDevelopSessionData> readSessionDataList() const;

private:
    QVector<KDevelopSessionData> m_sessionDataList;

    QMutex m_mutex;
    QVector<QObject*> m_observers;

    QString m_sessionDir;
    KDirWatch* m_dirWatch;
};

#endif
