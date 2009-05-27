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

#include <interfaces/iproject.h>
#include <interfaces/icore.h>
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
    Q_ASSERT(item->project());
    if( item && item->project() && item->project()->buildSystemManager() && item->project()->buildSystemManager()->builder( item->project()->projectItem() ) )
    {
        KJob* j = 0;
        switch( t )
        {
            case BuilderJob::Build:
                j = item->project()->buildSystemManager()->builder( item->project()->projectItem() )->build( item );
                break;
            case BuilderJob::Clean:
                j = item->project()->buildSystemManager()->builder( item->project()->projectItem() )->clean( item );
                break;
            case BuilderJob::Install:
                j = item->project()->buildSystemManager()->builder( item->project()->projectItem() )->install( item );
                break;
            case BuilderJob::Prune:
                j = item->project()->buildSystemManager()->builder( item->project()->projectItem() )->prune( item->project() );
                break;
            case BuilderJob::Configure:
                j = item->project()->buildSystemManager()->builder( item->project()->projectItem() )->configure( item->project() );
                break;
            default:
                break;
        }
        if( j )
        {
            q->addSubjob( j );
        }
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

void BuilderJob::start()
{
    // Automatically save all documents before starting to build
    // might need an option to turn off at some point
    // Also should be moved into the builder and there try to find target(s) for the given item and then just save the documents of that target -> list??
    if( KGlobal::config()->group("Project Manager").readEntry( "SaveAllDocumentsBeforeBuilding", true ) ) 
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
