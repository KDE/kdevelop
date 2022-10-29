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


class KDevelopSessions : public Plasma::AbstractRunner
                       , public KDevelopSessionsObserver
{
    Q_OBJECT
    Q_INTERFACES(KDevelopSessionsObserver)

public:
    KDevelopSessions(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args);
    ~KDevelopSessions() override;

public:
    void match(Plasma::RunnerContext &context) override;
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) override;

protected:
    void init() override;

public Q_SLOTS: // KDevelopSessionsObserver API
    void setSessionDataList(const QVector<KDevelopSessionData>& sessionDataList) override;

private:
    // working copy of the list
    QVector<KDevelopSessionData> m_sessionDataList;
};

#endif
