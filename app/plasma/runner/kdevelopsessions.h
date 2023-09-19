/*
    SPDX-FileCopyrightText: 2008, 2011 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVELOPSESSIONSRUNNER_H
#define KDEVELOPSESSIONSRUNNER_H

// KDevelopSessionsWatch
#include <kdevelopsessionsobserver.h>
// KF
#include <KRunner/AbstractRunner>

class KDevelopSessions : public KRunner::AbstractRunner, public KDevelopSessionsObserver
{
    Q_OBJECT
    Q_INTERFACES(KDevelopSessionsObserver)

public:
    KDevelopSessions(QObject* parent, const KPluginMetaData& metaData);
    ~KDevelopSessions() override;

public:
    void match(KRunner::RunnerContext& context) override;
    void run(const KRunner::RunnerContext& context, const KRunner::QueryMatch& match) override;

protected:
    void init() override;

public Q_SLOTS: // KDevelopSessionsObserver API
    void setSessionDataList(const QVector<KDevelopSessionData>& sessionDataList) override;

private:
    // working copy of the list
    QVector<KDevelopSessionData> m_sessionDataList;
};

#endif
