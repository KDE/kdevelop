/* This file is part of KDevelop
    Copyright 2012 Aleix Pol Gonzalez <aleixpol@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kdevninjabuilderplugin.h"
#include "ninjajob.h"
#include <KAboutData>
#include <KPluginFactory>
#include <KDebug>
#include <KConfigGroup>
#include <KShell>
#include <project/projectmodel.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/builderjob.h>
#include <interfaces/iproject.h>
#include <QFile>

K_PLUGIN_FACTORY(NinjaBuilderFactory, registerPlugin<KDevNinjaBuilderPlugin>(); )
// K_EXPORT_PLUGIN(NinjaBuilderFactory(KAboutData("kdevninja", "kdevninja", ki18n("Ninja Builder"), "0.1", ki18n("Support for building Ninja projects"), KAboutData::License_GPL)))

KDevNinjaBuilderPlugin::KDevNinjaBuilderPlugin(QObject* parent, const QVariantList& )
    : KDevelop::IPlugin("kdevninja", parent)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectBuilder )
    if(hasError())
        kWarning() << "Ninja plugin installed but ninja is not installed.";
}

bool KDevNinjaBuilderPlugin::hasError() const
{
    return NinjaJob::ninjaBinary().isEmpty();
}

static QStringList targetsInFolder(KDevelop::ProjectFolderItem* item)
{
    QStringList ret;
    foreach(KDevelop::ProjectTargetItem* target, item->targetList()) {
        ret += target->text();
    }
    return ret;
}

/**
 * Returns the first non-empty list of targets in folder @p item
 * or any of its ancestors if possible
 */
static QStringList closestTargetsForFolder(KDevelop::ProjectFolderItem* item)
{
    KDevelop::ProjectFolderItem* current = item;
    while (current) {
        const QStringList targets = targetsInFolder(current);
        if (!targets.isEmpty()) {
            return targets;
        }
        current = (current->parent() ? current->parent()->folder() : 0);
    }
    return QStringList();
}

static QStringList argumentsForItem(KDevelop::ProjectBaseItem* item)
{
    if(!item->parent() &&
        QFile::exists(item->project()->buildSystemManager()->buildDirectory(item->project()->projectItem()).toLocalFile()))
      return QStringList();

    switch(item->type()) {
        case KDevelop::ProjectBaseItem::File:
          return QStringList(item->path().toLocalFile()+'^');
        case KDevelop::ProjectBaseItem::Target:
        case KDevelop::ProjectBaseItem::ExecutableTarget:
        case KDevelop::ProjectBaseItem::LibraryTarget:
          return QStringList(item->target()->text());
        case KDevelop::ProjectBaseItem::Folder:
        case KDevelop::ProjectBaseItem::BuildFolder:
          return closestTargetsForFolder(item->folder());
    }
    return QStringList();
}

NinjaJob* KDevNinjaBuilderPlugin::runNinja(KDevelop::ProjectBaseItem* item, const QStringList& args, const QByteArray& signal)
{
    ///Running the same builder twice may result in serious problems,
    ///so kill jobs already running on the same project
    foreach (NinjaJob* ninjaJob, m_activeNinjaJobs.data())
    {
        if(item && ninjaJob->item() && ninjaJob->item()->project() == item->project() ) {
            kDebug() << "killing running ninja job, due to new started build on same project:" << ninjaJob;
            ninjaJob->kill(KJob::EmitResult);
        }
    }

    // Build arguments using data from KCM
    QStringList jobArguments;
    KSharedConfig::Ptr config = item->project()->projectConfiguration();
    KConfigGroup group = config->group( "NinjaBuilder" );

    if( !group.readEntry( "Abort on First Error", true ) ) {
        jobArguments << "-k";
    }
    if( group.readEntry( "Override Number Of Jobs", false ) ) {
        int jobCount = group.readEntry( "Number Of Jobs", 1 );
        if( jobCount > 0 ) {
            jobArguments << QString( "-j%1" ).arg( jobCount );
        }
    }
    int errorCount = group.readEntry( "Number Of Errors", 1 );
    if( errorCount > 1 ) {
        jobArguments << QString( "-k%1" ).arg( errorCount );
    }
    if( group.readEntry( "Display Only", false ) ) {
        jobArguments << "-n";
    }
    QString extraOptions = group.readEntry( "Additional Options", QString() );
    if( !extraOptions.isEmpty() ) {
        foreach(const QString& option, KShell::splitArgs( extraOptions ) ) {
            jobArguments << option;
        }
    }
    jobArguments << args;

    NinjaJob* job = new NinjaJob(item, jobArguments, signal, this);
    m_activeNinjaJobs.append(job);
    return job;
}

KJob* KDevNinjaBuilderPlugin::build(KDevelop::ProjectBaseItem* item)
{
    return runNinja(item, argumentsForItem(item), "built");
}

KJob* KDevNinjaBuilderPlugin::clean(KDevelop::ProjectBaseItem* item)
{
    return runNinja(item, QStringList("-t") << "clean", "cleaned");
}

KJob* KDevNinjaBuilderPlugin::install(KDevelop::ProjectBaseItem* item)
{
    NinjaJob* installJob = runNinja( item, QStringList( "install" ), "installed" );
    installJob->setIsInstalling( true );

    KSharedConfig::Ptr configPtr = item->project()->projectConfiguration();
    KConfigGroup builderGroup( configPtr, "NinjaBuilder" );
    bool installAsRoot = builderGroup.readEntry("Install As Root", false);
    if(installAsRoot) {
        KDevelop::BuilderJob* job = new KDevelop::BuilderJob;
        job->addCustomJob( KDevelop::BuilderJob::Build, build( item ), item );
        job->addCustomJob( KDevelop::BuilderJob::Install, installJob, item );
        job->updateJobName();
        return job;
    } else {
        return installJob;
    }
}

#include "kdevninjabuilderplugin.moc"
