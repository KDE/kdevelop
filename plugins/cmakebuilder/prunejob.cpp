/*
    SPDX-FileCopyrightText: 2013 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "prunejob.h"
#include <cmakeutils.h>
#include <outputview/outputmodel.h>
#include <interfaces/iproject.h>

#include <KLocalizedString>
#include <KIO/DeleteJob>

#include <QDir>

using namespace KDevelop;

PruneJob::PruneJob(KDevelop::IProject* project)
    : OutputJob(project, Verbose)
    , m_project(project)
    , m_job(nullptr)
{
    setCapabilities( Killable );
    setToolTitle( i18n("CMake") );
    setStandardToolView( KDevelop::IOutputView::BuildView );
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll );
}

void PruneJob::start()
{
    auto* output = new OutputModel(this);
    setModel(output);
    startOutput();

    Path builddir = CMake::currentBuildDir( m_project );
    if( builddir.isEmpty() )
    {
        output->appendLine(i18n("No build directory configured, cannot clear the build directory"));
        emitResult();
        return;
    }
    else if (!builddir.isLocalFile() || QDir(builddir.toLocalFile()).exists(QStringLiteral("CMakeLists.txt")))
    {
        output->appendLine(i18n("Wrong build directory, cannot clear the build directory"));
        emitResult();
        return;
    }

    QDir d( builddir.toLocalFile() );
    QList<QUrl> urls;
    const auto entries = d.entryList(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden);
    urls.reserve(entries.size());
    for (const auto& entry : entries) {
        urls << Path(builddir, entry).toUrl();
    }
    output->appendLine(i18n("%1> rm -rf %2", m_project->path().pathOrUrl(), builddir.toLocalFile()));
    m_job = KIO::del( urls );
    m_job->start();
    connect(m_job, &KJob::finished, this, &PruneJob::jobFinished);
}

bool PruneJob::doKill()
{
    return m_job->kill();
}

void PruneJob::jobFinished(KJob* job)
{
    auto* output = qobject_cast<OutputModel*>(model());
    if(job->error()==0)
        output->appendLine(i18n("** Prune successful **"));
    else
        output->appendLine(i18n("** Prune failed: %1 **", job->errorString()));
    emitResult();
    m_job = nullptr;
}
