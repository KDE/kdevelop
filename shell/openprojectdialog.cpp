/***************************************************************************
 *   Copyright (C) 2008 by Andreas Pakulat <apaku@gmx.de                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "openprojectdialog.h"
#include "openprojectpage.h"
#include "projectinfopage.h"

#include <QtCore/QFileInfo>
#include <QtCore/QDir>

#include <kpagewidgetmodel.h>

#include <kio/netaccess.h>
#include <kio/udsentry.h>
#include <kio/job.h>

#include <kdebug.h>

#include "core.h"
#include "uicontroller.h"
#include "mainwindow.h"
#include "shellextension.h"

namespace KDevelop
{

OpenProjectDialog::OpenProjectDialog( QWidget* parent )
    : KAssistantDialog( parent )
{
    QWidget* page = new OpenProjectPage( this );
    connect( page, SIGNAL( urlSelected( const KUrl& ) ), this, SLOT( validateOpenUrl( const KUrl& ) ) );
    openPage = addPage( page, "Select Directory/Project File" );
    page = new ProjectInfoPage( this );
    connect( page, SIGNAL( projectNameChanged( const QString& ) ), this, SLOT( validateProjectName( const QString& ) ) );
    connect( page, SIGNAL( projectManagerChanged( const QString& ) ), this, SLOT( validateProjectManger( const QString& ) ) );
    projectInfoPage = addPage( page, "Project Information" );
    setValid( openPage, false );
    setValid( projectInfoPage, false);
    setAppropriate( projectInfoPage, false );
    showButton( KDialog::Help, false );
}

void OpenProjectDialog::validateOpenUrl( const KUrl& url )
{
    entriesList.clear();
    kDebug() << "fetching files for:" << url;
    if( url.isLocalFile() )
    {
        QDir d( url.toLocalFile() );
        kDebug() << "filtering for:" << QStringList() << "*."+ShellExtension::getInstance()->projectFileExtension();
        entriesList = d.entryList( QStringList() << "*."+ShellExtension::getInstance()->projectFileExtension(), QDir::Files | QDir::Readable );
        kDebug() << "ok, got entries:" << entriesList;
    } else
    {
        KIO::ListJob* job = KIO::listDir( url );
        connect( job, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList& ) ), SLOT( gotFileList( KIO::Job*, const KIO::UDSEntryList& ) ) );
        KIO::NetAccess::synchronousRun( job, Core::self()->uiControllerInternal()->defaultMainWindow() );
    }
    kDebug() << "got entries:" << entriesList;
    setAppropriate( projectInfoPage, entriesList.isEmpty() );
    ProjectInfoPage* page = dynamic_cast<ProjectInfoPage*>( projectInfoPage->widget() );
    if( !page )
    {
        kFatal() << "Oops, project info page item doesn't contain a ProjectInfoPage. Serious problem.";
    }
    page->setProjectUrl( url );
    setValid( openPage, true );
    directory = url;
}

void OpenProjectDialog::gotFileList( KIO::Job*, const KIO::UDSEntryList& list )
{
    foreach( const KIO::UDSEntry& entry, list )
    {
        kWarning() << "got child:" << entry.stringValue( KIO::UDSEntry::UDS_NAME );
        QString name = entry.stringValue( KIO::UDSEntry::UDS_NAME );
        if( !entry.isDir() && name.endsWith( ShellExtension::getInstance()->projectFileExtension() ) )
        {
            entriesList << name;
        }
    }
}

}

#include "openprojectdialog.moc"

