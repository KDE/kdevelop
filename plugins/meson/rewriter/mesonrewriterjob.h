/*
    SPDX-FileCopyrightText: 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "mesonactionbase.h"

#include <KJob>
#include <QFutureWatcher>
#include <QVector>

namespace KDevelop
{
class IProject;
}

class MesonRewriterJob : public KJob
{
    Q_OBJECT
public:
    explicit MesonRewriterJob(KDevelop::IProject* project, const QVector<MesonRewriterActionPtr>& actions,
                              QObject* parent);

    void start() override;
    bool doKill() override;

private:
    QString execute();
    void finished();

private:
    KDevelop::IProject* m_project = nullptr;
    QVector<MesonRewriterActionPtr> m_actions;

    QFutureWatcher<QString> m_futureWatcher;
};
