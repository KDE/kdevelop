/* KDevelop CMake Support
 *
 * Copyright 2013 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "prunejob.h"
#include <cmakeutils.h>
#include <outputview/outputmodel.h>
#include <interfaces/iproject.h>
#include <KLocalizedString>
#include <kio/deletejob.h>
#include <QDir>

using namespace KDevelop;

PruneJob::PruneJob(KDevelop::IProject* project)
    : OutputJob(project, Verbose)
    , m_project(project)
    , m_job(0)
{
    setCapabilities( Killable );
    setToolTitle( i18n("CMake") );
    setStandardToolView( KDevelop::IOutputView::BuildView );
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll );
}

void PruneJob::start()
{
    OutputModel* output = new OutputModel(this);
    setModel(output);
    startOutput();

    Path builddir = CMake::currentBuildDir( m_project );
    if( builddir.isEmpty() )
    {
        output->appendLine(i18n("No Build Directory configured, cannot clear builddir"));
        emitResult();
        return;
    }
    else if (!builddir.isLocalFile() || QDir(builddir.toLocalFile()).exists("CMakeLists.txt"))
    {
        output->appendLine(i18n("Wrong build directory, cannot clear the build directory"));
        emitResult();
        return;
    }

    QDir d( builddir.toLocalFile() );
    QList<QUrl> urls;
    foreach( const QString& entry, d.entryList( QDir::NoDotAndDotDot | QDir::AllEntries ) )
    {
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
    OutputModel* output = qobject_cast<OutputModel*>(model());
    if(job->error()==0)
        output->appendLine(i18n("** Prune successful **"));
    else
        output->appendLine(i18n("** Prune failed: %1 **", job->errorString()));
    emitResult();
    m_job = 0;
}
