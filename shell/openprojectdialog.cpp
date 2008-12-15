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
#include "projectfileselectionpage.h"

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
    page = new ProjectFileSelectionPage( this );
    connect( page, SIGNAL( fileSelected( const QString& ) ), this, SLOT( validateProjectFile( const QString& ) ) );
    filePage = addPage( page, "Select Project file" );
    page = new ProjectInfoPage( this );
    connect( page, SIGNAL( projectNameChanged( const QString& ) ), this, SLOT( validateProjectName( const QString& ) ) );
    connect( page, SIGNAL( projectManagerChanged( const QString& ) ), this, SLOT( validateProjectManager( const QString& ) ) );
    projectInfoPage = addPage( page, "Project Information" );
    setValid( openPage, false );
    setValid( projectInfoPage, false);
    setValid( filePage, false );
    setAppropriate( projectInfoPage, false );
    setAppropriate( filePage, false );
    showButton( KDialog::Help, false );
}

void OpenProjectDialog::validateOpenUrl( const KUrl& url )
{
    entriesList.clear();
    if( url.isLocalFile() )
    {
        QDir d( url.toLocalFile() );
        entriesList = d.entryList( QStringList() << "*."+ShellExtension::getInstance()->projectFileExtension(), QDir::Files | QDir::Readable );
    } else
    {
        KIO::ListJob* job = KIO::listDir( url );
        connect( job, SIGNAL( entries( KIO::Job*, const KIO::UDSEntryList& ) ), SLOT( gotFileList( KIO::Job*, const KIO::UDSEntryList& ) ) );
        KIO::NetAccess::synchronousRun( job, Core::self()->uiControllerInternal()->defaultMainWindow() );
    }
    if( entriesList.isEmpty() )
    {
        setAppropriate( projectInfoPage, true );
        ProjectInfoPage* page = dynamic_cast<ProjectInfoPage*>( projectInfoPage->widget() );
        if( !page )
        {
            kFatal() << "Oops, project info page item doesn't contain a ProjectInfoPage. Serious problem.";
        }
        page->setProjectDir( url );
    } else
    {
        setAppropriate( projectInfoPage, false );
        if( entriesList.count() > 1 )
        {
            setAppropriate( filePage, true );
            ProjectFileSelectionPage* page = dynamic_cast<ProjectFileSelectionPage*>( filePage->widget() );
            if( !page )
            {
                kFatal() << "Ooops, project file selection page item doesn't contain ProjectFileSelectionPage. Serious problem";
            }
            page->setEntries( entriesList );
        } else 
        {
            setAppropriate( filePage, false );
        }
    }
    setValid( filePage, false );
    setValid( projectInfoPage, false );
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

void OpenProjectDialog::validateProjectName( const QString& name )
{
    projectName = name;
    validateProjectInfo();
}

void OpenProjectDialog::validateProjectInfo()
{
    setValid( projectInfoPage, !projectName.isEmpty() && !projectManager.isEmpty() );
}

void OpenProjectDialog::validateProjectManager( const QString& manager )
{
    projectManager = manager;
    validateProjectInfo();
}

void OpenProjectDialog::validateProjectFile( const QString& file )
{
    setAppropriate( projectInfoPage, false );
    setValid( filePage, true );
    projectFile = file;
}

}

#include "openprojectdialog.moc"

