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
