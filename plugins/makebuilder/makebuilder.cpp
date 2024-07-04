/*
    SPDX-FileCopyrightText: 2004 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "makebuilder.h"
#include "debug.h"
#include "makebuilderpreferences.h"

#include <project/projectmodel.h>
#include <project/builderjob.h>

#include <interfaces/iproject.h>

#include <KPluginFactory>
#include <KConfigGroup>

K_PLUGIN_FACTORY_WITH_JSON(MakeBuilderFactory, "kdevmakebuilder.json", registerPlugin<MakeBuilder>(); )

MakeBuilder::MakeBuilder(QObject* parent, const KPluginMetaData& metaData, const QVariantList&)
    : KDevelop::IPlugin(QStringLiteral("kdevmakebuilder"), parent, metaData)
{
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
    return runMake( dom, MakeJob::CleanCommand, QStringList(QStringLiteral("clean")) );
}

KJob* MakeBuilder::install(KDevelop::ProjectBaseItem *dom, const QUrl &installPath)
{
    KSharedConfigPtr configPtr = dom->project()->projectConfiguration();
    KConfigGroup builderGroup(configPtr, QStringLiteral("MakeBuilder"));
    bool installAsRoot = builderGroup.readEntry("Install As Root", false);

    QStringList args(QStringLiteral("install"));
    if (!installPath.isEmpty())
        args << QLatin1String("DESTDIR=") + installPath.toLocalFile();

    if(installAsRoot) {
        auto* job = new KDevelop::BuilderJob;
        job->addCustomJob( KDevelop::BuilderJob::Build, build(dom), dom );
        job->addCustomJob( KDevelop::BuilderJob::Install, runMake( dom, MakeJob::InstallCommand, args ), dom );
        job->updateJobName();
        return job;
    } else
        return runMake( dom, MakeJob::InstallCommand, args );
}

void MakeBuilder::jobFinished(KJob* job)
{
    auto* mj = qobject_cast<MakeJob*>(job);

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
            case MakeJob::CustomTargetCommand: {
                const auto targets = mj->customTargets();
                for (const QString& target : targets) {
                    emit makeTargetBuilt( mj->item(), target );
                }
                break;
            }
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
    const auto makeJobs = m_activeMakeJobs.data();
    for (MakeJob* makeJob : makeJobs) {
        if(item && makeJob->item() && makeJob->item()->project() == item->project() && makeJob->commandType() == c) {
            qCDebug(KDEV_MAKEBUILDER) << "killing running make job, due to new started build on same project:" << makeJob;
            makeJob->kill();
        }
    }

    auto* job = new MakeJob(this, item, c, overrideTargets, variables);
    m_activeMakeJobs.append(job);

    connect(job, &MakeJob::finished, this, &MakeBuilder::jobFinished);
    return job;
}

int MakeBuilder::perProjectConfigPages() const
{
    return 1;
}

KDevelop::ConfigPage* MakeBuilder::perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent)
{
    if (number == 0) {
        return new MakeBuilderPreferences(this, options, parent);
    }
    return nullptr;
}

#include "makebuilder.moc"
#include "moc_makebuilder.cpp"
