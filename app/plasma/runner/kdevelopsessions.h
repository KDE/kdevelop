/*
 *   Copyright 2008, 2011 Sebastian Kügler <sebas@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
    KDevelopSessions(QObject *parent, const QVariantList& args);
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
