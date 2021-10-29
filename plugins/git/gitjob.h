/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_GITJOB_H
#define KDEVPLATFORM_PLUGIN_GITJOB_H

#include <vcs/dvcs/dvcsjob.h>

class GitJob : public KDevelop::DVcsJob
{
    Q_OBJECT
    public:
        explicit GitJob(const QDir& workingDir, KDevelop::IPlugin* parent = nullptr, KDevelop::OutputJob::OutputJobVerbosity verbosity = KDevelop::OutputJob::Verbose);
        
};

#endif // KDEVPLATFORM_PLUGIN_GITJOB_H
