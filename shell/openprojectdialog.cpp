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
#include <KLocalizedString>
#include <KColorScheme>

#include <kio/netaccess.h>
#include <kio/udsentry.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <KJobWidgets/KJobWidgets>

#include <KDialog>

#include "core.h"
#include "uicontroller.h"
#include "mainwindow.h"
#include "shellextension.h"
#include "projectsourcepage.h"
#include <interfaces/iprojectcontroller.h>

namespace KDevelop
{

OpenProjectDialog::OpenProjectDialog( bool fetch, const QUrl& startUrl, QWidget* parent )
    : KAssistantDialog( parent )
    , sourcePage(nullptr)
    , openPage(nullptr)
    , projectInfoPage(nullptr)
{
    resize(QSize(700, 500));
    
    QUrl start = startUrl.isValid() ? startUrl : Core::self()->projectController()->projectsBaseDirectory();
    start = start.adjusted(QUrl::NormalizePathSegments);
    KPageWidgetItem* currentPage = 0;

    if( fetch ) {
        sourcePageWidget = new ProjectSourcePage( start, this );
        connect( sourcePageWidget, SIGNAL(isCorrect(bool)), this, SLOT(validateSourcePage(bool)) );
        sourcePage = addPage( sourcePageWidget, i18n("Select Source") );
        currentPage = sourcePage;
    }
    
    openPageWidget = new OpenProjectPage( start, this );
    connect( openPageWidget, SIGNAL(urlSelected(QUrl)), this, SLOT(validateOpenUrl(QUrl)) );
    connect( openPageWidget, SIGNAL(accepted()), this, SLOT(openPageAccepted()) );
    openPage = addPage( openPageWidget, i18n("Select a build system setup file, existing KDevelop project, "
                                             "or any folder to open as a project") );
    
    if( !fetch ) {
        currentPage = openPage;
    }

    ProjectInfoPage* page = new ProjectInfoPage( this );
    connect( page, SIGNAL(projectNameChanged(QString)), this, SLOT(validateProjectName(QString)) );
    connect( page, SIGNAL(projectManagerChanged(QString)), this, SLOT(validateProjectManager(QString)) );
    projectInfoPage = addPage( page, i18n("Project Information") );
    
    setValid( sourcePage, false );
    setValid( openPage, false );
    setValid( projectInfoPage, false);
    setAppropriate( projectInfoPage, false );
//     showButton( KDialog::Help, false );
    
    setCurrentPage( currentPage );
    setWindowTitle(i18n("Open Project"));
}

void OpenProjectDialog::validateSourcePage(bool valid)
{
    setValid(sourcePage, valid);
    openPageWidget->setUrl(sourcePageWidget->workingDir());
}

void OpenProjectDialog::validateOpenUrl( const QUrl& url )
{
    bool isDir = false;
    QString extension;
    bool isValid = false;

    if( url.isLocalFile() )
    {
        QFileInfo info( url.toLocalFile() );
        isValid = info.exists();
        if ( isValid ) {
            isDir = info.isDir();
            extension = info.suffix();
        }
    } else
    {
        KIO::StatJob* statJob = KIO::stat( url, KIO::HideProgressInfo );
        KJobWidgets::setWindow(statJob, Core::self()->uiControllerInternal()->defaultMainWindow() );
        isValid = statJob->exec(); // TODO: do this asynchronously so that the user isn't blocked while typing every letter of the hostname in sftp://hostname
        if ( isValid ) {
            KIO::UDSEntry entry = statJob->statResult();
            isDir = entry.isDir();
            extension = QFileInfo( entry.stringValue( KIO::UDSEntry::UDS_NAME ) ).suffix();
        }
    }

    if ( isValid ) {
        // reset header
        openPage->setHeader(i18n("Open \"%1\" as project", url.fileName()));
    } else {
        // report error
        KColorScheme scheme(palette().currentColorGroup());
        const QString errorMsg = i18n("Selected URL is invalid");
        openPage->setHeader(QString("<font color='%1'>%2</font>")
            .arg(scheme.foreground(KColorScheme::NegativeText).color().name())
            .arg(errorMsg)
        );
        setAppropriate( projectInfoPage, false );
        setAppropriate( openPage, true );
        setValid( openPage, false );
        return;
    }

    if( isDir || extension != ShellExtension::getInstance()->projectFileExtension() ) 
    {
        setAppropriate( projectInfoPage, true );
        m_url = url;
        if( isDir )
        {
            m_url = m_url.adjusted(QUrl::StripTrailingSlash );
        }
        else
        {
            m_url = m_url.adjusted(QUrl::StripTrailingSlash | QUrl::RemoveFilename);
        }
        ProjectInfoPage* page = qobject_cast<ProjectInfoPage*>( projectInfoPage->widget() );
        if( page )
        {
            page->setProjectName( m_url.fileName() );
            OpenProjectPage* page2 = qobject_cast<OpenProjectPage*>( openPage->widget() );
            if( page2 )
            {
                // Default manager
                page->setProjectManager( "Generic Project Manager" );
                // clear the filelist
                m_fileList.clear();

                if( isDir ) {
                    // If a dir was selected fetch all files in it
                    KIO::ListJob* job = KIO::listDir( m_url );
                    connect( job, SIGNAL(entries(KIO::Job*,KIO::UDSEntryList)), 
                                  SLOT(storeFileList(KIO::Job*,KIO::UDSEntryList)));
                    KIO::NetAccess::synchronousRun( job, Core::self()->uiController()->activeMainWindow() );
                } else {
                    // Else we'lll just take the given file
                    m_fileList << url.fileName();
                }
                // Now find a manager for the file(s) in our filelist.
                bool managerFound = false;
                foreach( const QString& manager, page2->projectFilters().keys() )
                {
                    foreach( const QString& filterexp, page2->projectFilters()[manager] )
                    {
                        if( !m_fileList.filter( QRegExp( filterexp, Qt::CaseSensitive, QRegExp::Wildcard ) ).isEmpty() ) 
                        {
                            managerFound = true;
                            break;
                        }
                    }
                    if( managerFound )
                    {
                        page->setProjectManager( manager );
                        break;
                    }
                }
            }
        }
        m_url.setPath( m_url.path() + '/' + m_url.fileName() + '.' + ShellExtension::getInstance()->projectFileExtension() );
    } else
    {
        setAppropriate( projectInfoPage, false );
        m_url = url;
    }
    validateProjectInfo();
    setValid( openPage, true );
}

void OpenProjectDialog::openPageAccepted()
{
    if ( isValid( openPage ) ) {
        next();
    }
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

QUrl OpenProjectDialog::projectFileUrl()
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

void OpenProjectDialog::storeFileList(KIO::Job*, const KIO::UDSEntryList& list)
{
    foreach( const KIO::UDSEntry& entry, list )
    {
        QString name = entry.stringValue( KIO::UDSEntry::UDS_NAME );
        if( name != "." && name != ".." && !entry.isDir() )
        {
            m_fileList << name;
        }
    }
}

}


