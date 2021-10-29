/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KDEVPLATFORM_PLUGIN_GITCLONEJOB_H
#define KDEVPLATFORM_PLUGIN_GITCLONEJOB_H

#include "gitjob.h"

class GitCloneJob : public GitJob
{
    Q_OBJECT
    public:
        GitCloneJob(const QDir& d, KDevelop::IPlugin* parent, KDevelop::OutputJob::OutputJobVerbosity verbosity = KDevelop::OutputJob::Verbose);

    public Q_SLOTS:
        void processResult();

    private:
        uint m_steps;
};

#endif // KDEVPLATFORM_PLUGIN_GITCLONEJOB_H
