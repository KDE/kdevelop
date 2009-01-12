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
#include <kmessagebox.h>
#include <klocale.h>

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
    connect( page, SIGNAL( projectManagerChanged( const QString& ) ), this, SLOT( validateProjectManager( const QString& ) ) );
    projectInfoPage = addPage( page, "Project Information" );
    setValid( openPage, false );
    setValid( projectInfoPage, false);
    setAppropriate( projectInfoPage, false );
    showButton( KDialog::Help, false );
}

void OpenProjectDialog::validateOpenUrl( const KUrl& url )
{
    bool isDir = false;
    QString extension;
    if( url.isLocalFile() )
    {
        isDir = QFileInfo( url.toLocalFile() ).isDir();
        extension = QFileInfo( url.toLocalFile() ).suffix();
    } else 
    {
        KIO::UDSEntry entry;
        if( KIO::NetAccess::stat( url, entry, Core::self()->uiControllerInternal()->defaultMainWindow() ) ) {
            isDir = entry.isDir();
            extension = QFileInfo( entry.stringValue( KIO::UDSEntry::UDS_NAME ) ).suffix();
        } else {
            KMessageBox::error(Core::self()->uiControllerInternal()->defaultMainWindow(), 
                               i18n("Could not fetch information for remote url: %1", url.prettyUrl()), 
                               i18n("Error fetching remote url") );
            setAppropriate( projectInfoPage, false );
            setAppropriate( openPage, true );
            setValid( openPage, false );
            return;
        }
    }
    if( isDir || extension != ShellExtension::getInstance()->projectFileExtension() ) 
    {
        setAppropriate( projectInfoPage, true );
        m_url = url;
        if( !isDir )
        {
             m_url = m_url.upUrl();
        }
        ProjectInfoPage* page = qobject_cast<ProjectInfoPage*>( projectInfoPage->widget() );
        if( page )
        {
            page->setProjectName( m_url.fileName() );
            OpenProjectPage* page2 = qobject_cast<OpenProjectPage*>( openPage->widget() );
            if( page2 )
            {
                if( !isDir )
                {
                    bool managerFound = false;
                    foreach( const QString& manager, page2->projectFilters().keys() )
                    {
                        foreach( const QString& filter, page2->projectFilters()[manager] )
                        {
                            QRegExp r( filter );
                            r.setPatternSyntax( QRegExp::Wildcard );
                            if( r.exactMatch( url.fileName() ) )
                            {
                                page->setProjectManager( manager );
                                managerFound = true;
                                break;
                            }
                        }
                        if( managerFound ) 
                        {
                            break;
                        }
                    }
                } else
                {
                    page->setProjectManager( "Generic Project Manager" );
                }
            }
        }
        m_url.addPath( m_url.fileName()+'.'+ShellExtension::getInstance()->projectFileExtension() );
    } else
    {
        setAppropriate( projectInfoPage, false );
        m_url = url;
    }
    validateProjectInfo();
    setValid( openPage, true );
}

void OpenProjectDialog::validateProjectName( const QString& name )
{
    m_projectName = name;
    validateProjectInfo();
}

void OpenProjectDialog::validateProjectInfo()
{
    setValid( projectInfoPage, (!projectName().isEmpty() && !projectManager().isEmpty()) );
}

void OpenProjectDialog::validateProjectManager( const QString& manager )
{
    m_projectManager = manager;
    validateProjectInfo();
}

KUrl OpenProjectDialog::projectFileUrl()
{
    return m_url;
}

QString OpenProjectDialog::projectName()
{
    return m_projectName;
}

QString OpenProjectDialog::projectManager()
{
    return m_projectManager;
}

}

#include "openprojectdialog.moc"

