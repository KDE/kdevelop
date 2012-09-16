/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "builderjob.h"

#include <kconfiggroup.h>
#include <KLocalizedString>

#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/isession.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/idocument.h>
#include <project/projectmodel.h>
#include <project/interfaces/iprojectbuilder.h>
#include <project/interfaces/ibuildsystemmanager.h>

namespace KDevelop
{

class BuilderJobPrivate
{
public:
    BuilderJobPrivate( BuilderJob* job ) {
        q = job;
        failOnFirstError = true;
    }
    BuilderJob* q;
    void addJob( BuilderJob::BuildType, KDevelop::ProjectBaseItem* );
    bool failOnFirstError;
};


void BuilderJobPrivate::addJob( BuilderJob::BuildType t, KDevelop::ProjectBaseItem* item )
{
    Q_ASSERT(item);
    kDebug() << "adding build job for item:" << item->text();
    Q_ASSERT(item->project());
    kDebug() << "project for item:" << item->project()->name();
    Q_ASSERT(item->project()->projectItem());
    kDebug() << "project item for the project:" << item->project()->projectItem()->text();
    if( !item->project()->buildSystemManager() )
    {
        kWarning() << "no buildsystem manager for:" << item->text() << item->project()->name();
        return;
    }
    kDebug() << "got build system manager";
    Q_ASSERT(item->project()->buildSystemManager()->builder());
    KJob* j = 0;
    switch( t )
    {
        case BuilderJob::Build:
            j = item->project()->buildSystemManager()->builder()->build( item );
            break;
        case BuilderJob::Clean:
            j = item->project()->buildSystemManager()->builder()->clean( item );
            break;
        case BuilderJob::Install:
            j = item->project()->buildSystemManager()->builder()->install( item );
            break;
        case BuilderJob::Prune:
            j = item->project()->buildSystemManager()->builder()->prune( item->project() );
            break;
        case BuilderJob::Configure:
            j = item->project()->buildSystemManager()->builder()->configure( item->project() );
            break;
        default:
            break;
    }
    if( j )
    {
        q->addCustomJob( t, j, item );
    }
}
BuilderJob::BuilderJob() 
    : d( new BuilderJobPrivate( this ) )
{
}

void BuilderJob::addItems( BuildType t, const QList<KDevelop::ProjectBaseItem*>& items )
{
    foreach( KDevelop::ProjectBaseItem* item, items )
    {
        d->addJob( t, item );
    }
}

void BuilderJob::addProjects( BuildType t, const QList<KDevelop::IProject*>& projects )
{
    foreach( KDevelop::IProject* project, projects )
    {
        d->addJob( t, project->projectItem() );
    }
}

void BuilderJob::addItem( BuildType t, ProjectBaseItem* item )
{
    d->addJob( t, item );
}

void BuilderJob::addCustomJob( BuilderJob::BuildType type, KJob* job, ProjectBaseItem* item )
{
    if( BuilderJob* builderJob = dynamic_cast<BuilderJob*>( job ) ) {
        // If a subjob is a builder job itself, re-own its job list to avoid having recursive composite jobs.
        QList< BuilderJob::SubJobData > subjobs = builderJob->takeJobList();
        builderJob->deleteLater();
        foreach( const BuilderJob::SubJobData& subjob, subjobs ) {
            addSubjob( subjob.job );
        }
        m_metadata << subjobs;
    } else {
        addSubjob( job );

        SubJobData data;
        data.type = type;
        data.job = job;
        data.item = item;
        m_metadata << data;
    }
}

QList< BuilderJob::SubJobData > BuilderJob::takeJobList()
{
    QList< SubJobData > ret = m_metadata;
    m_metadata.clear();
    clearSubjobs();
    setObjectName( QString() );
    return ret;
}

void BuilderJob::updateJobName()
{
    // Which items are mentioned in the set
    // Make it a list to preserve ordering; search overhead (n^2) isn't too big
    QList< ProjectBaseItem* > registeredItems;
    // Whether there are jobs without any specific item
    bool hasNullItems = false;
    // Which build types are mentioned in the set
    QSet< BuildType >  buildTypes;

    foreach( const SubJobData& subjob, m_metadata ) {
        if( subjob.item ) {
            if( !registeredItems.contains( subjob.item ) ) {
                registeredItems.append( subjob.item );
            }
            buildTypes.insert( subjob.type );
        } else {
            hasNullItems = true;
        }
    }

    QString itemNames;
    if( !hasNullItems ) {
        QStringList itemNamesList;
        foreach( ProjectBaseItem* item, registeredItems ) {
            itemNamesList << item->text();
        }
        itemNames = itemNamesList.join(", ");
    } else {
        itemNames = i18nc( "Unspecified set of build items (e. g. projects, targets)", "Various items" );
    }

    QString methodNames;
    QStringList methodNamesList;
    foreach( BuildType type, buildTypes ) {
        methodNamesList << buildTypeToString( type );
    }
    methodNames = methodNamesList.join( ", " );

    QString jobName = QString( "%1: %2" ).arg( itemNames ).arg( methodNames );
    setObjectName( jobName );
}

QString BuilderJob::buildTypeToString( BuilderJob::BuildType type )
{
    switch( type ) {
        case Build:
            return i18nc( "@info:status", "build" );
        case Clean:
            return i18nc( "@info:status", "clean" );
        case Configure:
            return i18nc( "@info:status", "configure" );
        case Install:
            return i18nc( "@info:status", "install" );
        case Prune:
            return i18nc( "@info:status", "prune" );
        default:
            return QString();
    }
}

void BuilderJob::start()
{
    #if 0
    ///Running the same builder twice may result in serious problems, so kill jobs already running on the same project
    QList<QPointer<KJob> > jobs;
    foreach(KJob* job, KDevelop::ICore::self()->runController()->currentJobs()) {
        kDebug() << "running" << job;
        jobs << job;
    }
    
    for(QList< QPointer< KJob > >::iterator it = jobs.begin(); it != jobs.end(); ++it)
    {
        QPointer< KJob > job = *it;
        if(!job)
            continue;
        
        BuilderJob* bJob = dynamic_cast<BuilderJob*>(job.data());
        if( bJob && bJob != this )
        {
            kDebug() << "killing running builder job, due to new started build";

            //Copy the subjobs into QPointers first, as we never know what is deleted when
            QList<QPointer<KJob> > subJobs;
            foreach(KJob* subJob, bJob->subjobs())
                subJobs << subJob;
            
//             while(!subJobs.empty()) {
//                 if(subJobs.back() && subJobs.back()->capabilities() & KJob::Killable)
//                     subJobs.back()->kill(EmitResult);
//                 subJobs.pop_front();
//             }
            if(job && job->capabilities() & KJob::Killable)
                job->kill(EmitResult);
        }
    }
    #endif
    
    // Automatically save all documents before starting to build
    // might need an option to turn off at some point
    // Also should be moved into the builder and there try to find target(s) for the given item and then just save the documents of that target -> list??
    if( ICore::self()->activeSession()->config()->group("Project Manager").readEntry( "Save All Documents Before Building", true ) ) 
    {
        KDevelop::ICore::self()->documentController()->saveAllDocuments( KDevelop::IDocument::Silent );
    }

    if(hasSubjobs())
        KDevelop::ICore::self()->runController()->registerJob( subjobs().first() );
    else
        emitResult();
}

void BuilderJob::slotResult( KJob* job )
{
    //call parent implementation for default behaviour
    KCompositeJob::slotResult( job );
    if( ( !error() || !d->failOnFirstError ) && hasSubjobs() ) 
    {
        // start next build;
        KDevelop::ICore::self()->runController()->registerJob( subjobs().first() );
    } else 
    {
        emitResult();
    }
}

void BuilderJob::setStopOnFail( bool stopOnFail )
{
    d->failOnFirstError = stopOnFail;
}

bool BuilderJob::stopOnFail() const
{
    return d->failOnFirstError;
}

}

#include "builderjob.moc"
