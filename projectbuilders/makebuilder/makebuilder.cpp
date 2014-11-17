/* This file is part of KDevelop
    Copyright 2004 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2007 Andreas Pakulat <apaku@gmx.de>
    Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>

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

#include "makebuilder.h"
#include "debug.h"

#include <project/projectmodel.h>
#include <project/builderjob.h>

#include <interfaces/iproject.h>

#include <KPluginFactory>
#include <KAboutData>
#include <KConfigGroup>

Q_LOGGING_CATEGORY(MAKEBUILDER, "kdevelop.projectbuilders.makebuilder")
K_PLUGIN_FACTORY(MakeBuilderFactory, registerPlugin<MakeBuilder>(); )
// K_EXPORT_PLUGIN(MakeBuilderFactory(KAboutData("kdevmakebuilder","kdevmakebuilder", ki18n("Make Builder"), "0.1", ki18n("Support for building Make projects"), KAboutData::License_GPL)))

MakeBuilder::MakeBuilder(QObject *parent, const QVariantList &)
    : KDevelop::IPlugin("kdevmakebuilder", parent)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectBuilder )
    KDEV_USE_EXTENSION_INTERFACE( IMakeBuilder )
}

MakeBuilder::~MakeBuilder()
{
}

KJob* MakeBuilder::build( KDevelop::ProjectBaseItem *dom )
{
    return runMake( dom, MakeJob::BuildCommand );
}

KJob* MakeBuilder::clean( KDevelop::ProjectBaseItem *dom )
{
    return runMake( dom, MakeJob::CleanCommand, QStringList("clean") );
}

KJob* MakeBuilder::install( KDevelop::ProjectBaseItem *dom )
{
    KSharedConfigPtr configPtr = dom->project()->projectConfiguration();
    KConfigGroup builderGroup( configPtr, "MakeBuilder" );
    bool installAsRoot = builderGroup.readEntry("Install As Root", false);
    if(installAsRoot) {
        KDevelop::BuilderJob* job = new KDevelop::BuilderJob;
        job->addCustomJob( KDevelop::BuilderJob::Build, build(dom), dom );
        job->addCustomJob( KDevelop::BuilderJob::Install, runMake( dom, MakeJob::InstallCommand, QStringList("install") ), dom );
        job->updateJobName();
        return job;
    } else
        return runMake( dom, MakeJob::InstallCommand, QStringList("install") );
}

void MakeBuilder::jobFinished(KJob* job)
{
    MakeJob* mj = dynamic_cast<MakeJob*>(job);

    if( !mj )
        return;

    if (mj->error())
    {
        emit failed( mj->item() );

    } else
    {
        switch( mj->commandType() )
        {
            case MakeJob::BuildCommand:
                emit built( mj->item() );
                break;
            case MakeJob::InstallCommand:
                emit installed( mj->item() );
                break;
            case MakeJob::CleanCommand:
                emit cleaned( mj->item() );
                break;
            case MakeJob::CustomTargetCommand:
                foreach( const QString& target, mj->customTargets() ) {
                    emit makeTargetBuilt( mj->item(), target );
                }
                break;
        }
    }
}

KJob* MakeBuilder::executeMakeTarget(KDevelop::ProjectBaseItem* item,
                                    const QString& targetname )
{
    return executeMakeTargets( item, QStringList(targetname) );
}

KJob* MakeBuilder::executeMakeTargets(KDevelop::ProjectBaseItem* item,
                                    const QStringList& targetnames,
                                    const MakeVariables& variables )
{
    return runMake( item, MakeJob::CustomTargetCommand, targetnames, variables );
}

KJob* MakeBuilder::runMake( KDevelop::ProjectBaseItem* item, MakeJob::CommandType c,
                            const QStringList& overrideTargets,
                            const MakeVariables& variables )
{
    ///Running the same builder twice may result in serious problems,
    ///so kill jobs already running on the same project
    foreach (MakeJob* makeJob, m_activeMakeJobs.data())
    {
        if(item && makeJob->item() && makeJob->item()->project() == item->project()) {
            qCDebug(MAKEBUILDER) << "killing running ninja job, due to new started build on same project:" << makeJob;
            makeJob->kill(KJob::EmitResult);
        }
    }

    MakeJob* job = new MakeJob(this, item, c, overrideTargets, variables);
    m_activeMakeJobs.append(job);

    connect(job, &MakeJob::finished, this, &MakeBuilder::jobFinished);
    return job;
}

#include "makebuilder.moc"
