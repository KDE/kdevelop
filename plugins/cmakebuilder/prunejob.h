/*
    SPDX-FileCopyrightText: 2013 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PRUNEJOB_H
#define PRUNEJOB_H
#include <outputview/outputjob.h>

namespace KDevelop { class IProject; }

class PruneJob : public KDevelop::OutputJob
{
    Q_OBJECT
    public:
        explicit PruneJob(KDevelop::IProject* project);
        void start() override;
        bool doKill() override;

    private Q_SLOTS:
        void jobFinished(KJob* job);

    private:
        KDevelop::IProject* m_project;
        KJob* m_job;
};

#endif // PRUNEJOB_H
