/***************************************************************************
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>

#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <kdevpartcontroller.h>
#include <kdevproject.h>
#include <kdevmainwindow.h>
#include <urlutil.h>

#include "cvsprocesswidget.h"
#include "cvsdir.h"
#include "cvsentry.h"
#include "cvspart.h"
#include "cvspartimpl.h"

///////////////////////////////////////////////////////////////////////////////
// class Constants
///////////////////////////////////////////////////////////////////////////////

// Nice name (relative to projectDirectory()) ;-)
const QString CvsServicePartImpl::changeLogFileName( "ChangeLog" );
// Four spaces for every log line (except the first, which includes the
// developers name)
const QString CvsServicePartImpl::changeLogPrependString( "    " );

///////////////////////////////////////////////////////////////////////////////
// class CvsServicePartImpl
///////////////////////////////////////////////////////////////////////////////

CvsServicePartImpl::CvsServicePartImpl( CvsServicePart *part, const char *name )
    : QObject( this, name? name : "cvspartimpl" ),
    m_part( part ), m_widget( 0 )
{
}

///////////////////////////////////////////////////////////////////////////////

CvsServicePartImpl::~CvsServicePartImpl()
{
    if (processWidget())
    {
        // Inform toplevel, that the output view is gone
        mainWindow()->removeView( m_widget );
        delete m_widget;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool CvsServicePartImpl::prepareOperation( const KURL::List &someUrls, CvsOperation op )
{
    kdDebug() << "===> CvsServicePartImpl::prepareOperation(const KURL::List &, CvsOperation)" << endl;
    KURL::List urls = someUrls;

    if (!m_part->project())
    {
        kdDebug(9000) << "CvsServicePartImpl::prepareOperation(): No project???" << endl;
        KMessageBox::sorry( 0, i18n("Open a project first.\nOperation will be aborted.") );
        return false;
    }

    if (m_widget->isAlreadyWorking())
    {
        if (KMessageBox::warningYesNo( 0,
            i18n("Another CVS operation is executing: do you want to cancel it \n"
                "and start this new one?"),
            i18n("CVS: Operation already pending ")) == KMessageBox::Yes)
        {
            m_widget->cancelJob();
        }
        else // Operation canceled
        {
            kdDebug() << "===> Operation canceled by user request" << endl;
            return false;
        }
    }

    validateURLs( projectDirectory(),  urls, op );
    if (urls.count() <= 0) // who knows? ;)
    {
        kdDebug(9000) << "CvsServicePartImpl::prepareOperation(): No valid document URL selected!!!" << endl;
        KMessageBox::sorry( 0, i18n("None of the file(s) you selected seems to be valid for repository.") );
        return false;
    }

    URLUtil::dump( urls );

    m_fileList = URLUtil::toRelativePaths( projectDirectory(), urls );

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::doneOperation()
{
    // If some kind of clean-up is needed ...
}

///////////////////////////////////////////////////////////////////////////////

bool CvsServicePartImpl::isRegisteredInRepository( const QString &projectDirectory, const KURL &url )
{
    kdDebug(9000) << "===> CvsServicePartImpl::isRegisteredInRepository() here! " << endl;

    CVSDir cvsdir( url.directory() );
    if (!cvsdir.isValid())
    {
        kdDebug(9000) << "===> Error: " << url.directory() << " is not a valid CVS directory " << endl;
        return false;
    }
    CVSEntry entry = cvsdir.fileState( url.fileName() );
    return entry.isValid();
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::validateURLs( const QString &projectDirectory, KURL::List &urls, CvsOperation op )
{
    kdDebug(9000) << "CvsServicePartImpl::validateURLs() here!" << endl;

    // If files are to be added, we can avoid to check them to see if they are registered in the
    // repository ;)
    if (op == opAdd)
    {
        kdDebug(9000) << "This is a Cvs Add operation and will not be checked against repository ;-)" << endl;
        return;
    }
    QValueList<KURL>::iterator it = urls.begin();
    while (it != urls.end())
    {
        if (!CvsServicePartImpl::isRegisteredInRepository( projectDirectory, (*it) ))
        {
            kdDebug(9000) << "Warning: file " << (*it).path() << " does NOT belong to repository and will not be used" << endl;

            it = urls.erase( it );
        }
        else
        {
            kdDebug(9000) << "Warning: file " << (*it).path() << " is in repository and will be accepted" << endl;

            ++it;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::addToIgnoreList( const QString &projectDirectory, const KURL &url )
{
    kdDebug(9000) << "===> CvsServicePartImpl::addToIgnoreList() here! " << endl;

    if ( url.path() == projectDirectory )
    {
        kdDebug(9000) << "Can't add to ignore list current project directory " << endl;
        return;
    }

    CVSDir cvsdir( url.directory() );
    cvsdir.ignoreFile( url.fileName() );
}

void CvsServicePartImpl::addToIgnoreList( const QString &projectDirectory, const KURL::List &urls )
{
    for (size_t i=0; i<urls.count(); ++i)
    {
        addToIgnoreList( projectDirectory, urls[i] );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CvsServicePartImpl::removeFromIgnoreList( const QString &/*projectDirectory*/, const KURL &url )
{
    kdDebug(9000) << "===> CvsServicePartImpl::removeFromIgnoreList() here! " << endl;

    QStringList ignoreLines;

    CVSDir cvsdir( url.directory() );
    cvsdir.doNotIgnoreFile( url.fileName() );
}

void CvsServicePartImpl::removeFromIgnoreList( const QString &projectDirectory, const KURL::List &urls )
{
    for (size_t i=0; i<urls.count(); ++i)
    {
        removeFromIgnoreList( projectDirectory, urls[i] );
    }
}

///////////////////////////////////////////////////////////////////////////////

bool CvsServicePartImpl::isValidDirectory( const QDir &dir ) const
{
    CVSDir cvsdir( dir );

    return cvsdir.isValid();
}

///////////////////////////////////////////////////////////////////////////////

CvsProcessWidget *CvsServicePartImpl::processWidget() const
{
    return m_widget;
}

///////////////////////////////////////////////////////////////////////////////

KDevMainWindow *CvsServicePartImpl::mainWindow() const
{
    return m_part->mainWindow();
}

///////////////////////////////////////////////////////////////////////////////

QString CvsServicePartImpl::projectDirectory() const
{
    return m_part->project()->projectDirectory();
}

///////////////////////////////////////////////////////////////////////////////

KDevCore *CvsServicePartImpl::core() const
{
    return m_part->core();
}

///////////////////////////////////////////////////////////////////////////////

KDevDiffFrontend *CvsServicePartImpl::diffFrontend() const
{
    return m_part->diffFrontend();
}


#include "cvspartimpl.moc"
