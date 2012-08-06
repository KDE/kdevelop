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

#include "configureandbuildjob.h"

#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/idocument.h>
#include <KLocalizedString>

ConfigureAndBuildJob::ConfigureAndBuildJob( KJob* configureJob, KJob* buildJob )
{
     addSubjob( configureJob );
     addSubjob( buildJob );
     setObjectName(i18n("'%1' + '%2'", configureJob->objectName(), buildJob->objectName()));
     kDebug() << "added job" << hasSubjobs();
}

void ConfigureAndBuildJob::start()
{
    // Automatically save all documents before starting to build
    // might need an option to turn off at some point
    // Also should be moved into the builder and there try to find target(s) for the given item and then just save the documents of that target -> list??
    KDevelop::ICore::self()->documentController()->saveAllDocuments( KDevelop::IDocument::Silent );

    kDebug() << "configure and build, have subjobs?" << hasSubjobs();
    if(hasSubjobs())
        KDevelop::ICore::self()->runController()->registerJob( subjobs().first() );
    else
        emitResult();
}

void ConfigureAndBuildJob::slotResult( KJob* job )
{
    //call parent implementation for default behaviour
    KCompositeJob::slotResult( job );
    if( !error() && hasSubjobs() ) 
    {
        // start next build;
        KDevelop::ICore::self()->runController()->registerJob( subjobs().first() );
    } else 
    {
        emitResult();
    }
}
#include "configureandbuildjob.moc"
