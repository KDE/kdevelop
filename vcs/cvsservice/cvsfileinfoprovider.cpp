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

#include <qregexp.h>
#include <kurl.h>
#include <kdebug.h>

#include <urlutil.h>
#include <kdevproject.h>

#include <dcopref.h>
#include <cvsjob_stub.h>
#include <cvsservice_stub.h>

#include "cvspart.h"
#include "cvsdir.h"
#include "cvsentry.h"
#include "cvsfileinfoprovider.h"


///////////////////////////////////////////////////////////////////////////////
// class CVSFileInfoProvider
///////////////////////////////////////////////////////////////////////////////

CVSFileInfoProvider::CVSFileInfoProvider( CvsServicePart *parent, CvsService_stub *cvsService )
    : KDevVCSFileInfoProvider( parent, "cvsfileinfoprovider" ),
    m_requestStatusJob( 0 ), m_cvsService( cvsService ), m_cachedDirEntries( 0 )
{
}

///////////////////////////////////////////////////////////////////////////////

CVSFileInfoProvider::~CVSFileInfoProvider()
{
   if (m_requestStatusJob && m_requestStatusJob->isRunning())
      m_requestStatusJob->cancel();
   delete m_requestStatusJob;
   delete m_cachedDirEntries;
}

///////////////////////////////////////////////////////////////////////////////

const VCSFileInfoMap *CVSFileInfoProvider::status( const QString &dirPath )
{
    // Same dir: we can do with cache ...
    if (dirPath != m_previousDirPath)
    {
        // ... different dir: flush old cache and cache new dir
        delete m_cachedDirEntries;
        CVSDir cvsdir( projectDirectory() + QDir::separator() + dirPath );
        m_previousDirPath = dirPath;
        m_cachedDirEntries = cvsdir.cacheableDirStatus();
    }
    return m_cachedDirEntries;
}

///////////////////////////////////////////////////////////////////////////////

bool CVSFileInfoProvider::requestStatus( const QString &dirPath, void *callerData )
{
    m_savedCallerData = callerData;
    if (m_requestStatusJob)
    {
        delete m_requestStatusJob;
        m_requestStatusJob = 0;
    }
    // Flush old cache
    if (m_cachedDirEntries)
    {
        delete m_cachedDirEntries;
        m_cachedDirEntries = 0;
        m_previousDirPath = dirPath;
    }

    // path, recursive, tagInfo: hmmm ... we may use tagInfo for collecting file tags ...
    DCOPRef job = m_cvsService->status( dirPath, true, false );
    m_requestStatusJob = new CvsJob_stub( job.app(), job.obj() );

    kdDebug(9006) << "Running command : " << m_requestStatusJob->cvsCommand() << endl;
    connectDCOPSignal( job.app(), job.obj(), "jobExited(bool, int)", "slotJobExited(bool, int)", true );
    connectDCOPSignal( job.app(), job.obj(), "receivedStdout(QString)", "slotReceivedOutput(QString)", true );
    return m_requestStatusJob->execute();
}

///////////////////////////////////////////////////////////////////////////////

void CVSFileInfoProvider::slotJobExited( bool normalExit, int /*exitStatus*/ )
{
    kdDebug(9006) << "CVSFileInfoProvider::slotJobExited(bool,int)" << endl;
    if (!normalExit)
        return;

//    m_cachedDirEntries = parse( m_requestStatusJob->output() );
    m_cachedDirEntries = parse( m_statusLines );
    // Remove me when not debugging
    printOutFileInfoMap( *m_cachedDirEntries );

    emit statusReady( *m_cachedDirEntries, m_savedCallerData );
}

///////////////////////////////////////////////////////////////////////////////

void CVSFileInfoProvider::slotReceivedOutput( QString someOutput )
{
    QStringList strings = m_bufferedReader.process( someOutput );
    if (strings.count() > 0)
    {
        m_statusLines += strings;
    }
}

///////////////////////////////////////////////////////////////////////////////

void CVSFileInfoProvider::slotReceivedErrors( QString /*someErrors*/ )
{
    /* Nothing to do */
}

///////////////////////////////////////////////////////////////////////////////

QString CVSFileInfoProvider::projectDirectory() const
{
    return owner()->project()->projectDirectory();
}

///////////////////////////////////////////////////////////////////////////////

VCSFileInfoMap *CVSFileInfoProvider::parse( QStringList stringStream )
{
    QRegExp rx_recordStart( "^=+$" );
    QRegExp rx_fileName( "^File: (\\.|\\-|\\w)+" );
    QRegExp rx_fileStatus( "Status: (\\.|-|\\s|\\w)+" );
    QRegExp rx_fileWorkRev( "\\bWorking revision:" );
    QRegExp rx_fileRepoRev( "\\bRepository revision:" );
    //QRegExp rx_stickyTag( "\\s+(Sticky Tag:\\W+(w+|\\(none\\)))" );
    //QRegExp rx_stickyDate( "" ); // @todo but are they useful?? :-/
    //QRegExp rx_stickyOptions( "" ); //@todo

    QString fileName,
        fileStatus,
        workingRevision,
        repositoryRevision,
        stickyTag,
        stickyDate,
        stickyOptions;

    VCSFileInfoMap *vcsStates = new VCSFileInfoMap;

    int state = 0;
    const int lastAcceptableState = 4;
    // This is where the dirty parsing is done: from a string stream representing the
    // 'cvs log' output we build a map with more useful strunctured data ;-)
    for (QStringList::const_iterator it=stringStream.begin(); it != stringStream.end(); ++it)
    {
        QString s = (*it).stripWhiteSpace();
        kdDebug(9006) << ">> Parsing: " << s << endl;

        if (rx_recordStart.exactMatch( s ))
            state = 1;
        else if (state == 1 && rx_fileName.search( s ) >= 0 && rx_fileStatus.search( s ) >= 0)    // FileName
        {
            fileName = rx_fileName.cap().replace( "File:", "" ).stripWhiteSpace();
            fileStatus = rx_fileStatus.cap().replace( "Status:", "" ).stripWhiteSpace();
            ++state; // Next state
            kdDebug(9006) << ">> " << fileName << ", " << fileStatus << endl;
        }
        else if (state == 2 && rx_fileWorkRev.search( s ) >= 0)
        {
            workingRevision = s.replace( "Working revision:", "" ).stripWhiteSpace();

            QRegExp rx_revision( "\\b(((\\d)+\\.?)*|New file!)" );
            if (rx_revision.search( workingRevision ) >= 0)
            {
                workingRevision = rx_revision.cap();
                kdDebug(9006) << ">> WorkRev: " << workingRevision << endl;
                ++state;
            }
        }
        else if (state == 3 && rx_fileRepoRev.search( s ) >= 0)
        {
            repositoryRevision = s.replace( "Repository revision:", "" ).stripWhiteSpace();

            QRegExp rx_revision( "\\b(((\\d)+\\.?)*|No revision control file)" );
            if (rx_revision.search( s ) >= 0)
            {
                repositoryRevision = rx_revision.cap();
                kdDebug(9006) << ">> RepoRev: " << repositoryRevision << endl;
                ++state;
            }
        }
/*
        else if (state == 4 && rx_stickyTag.search( s ) >= 0)
        {
            stickyTag = rx_stickyTag.cap();
            ++state;
        }
*/
        else if (state >= lastAcceptableState) // OK, parsed all useful info?
        {
            // Package stuff, put into map and get ready for a new record
            VCSFileInfo vcsInfo( fileName, workingRevision, repositoryRevision,
                String2EnumState( fileStatus ) );
            kdDebug(9006) << "== Inserting: " << vcsInfo.toString() << endl;
            vcsStates->insert( fileName, vcsInfo );
        }
    }
    return vcsStates;
}

///////////////////////////////////////////////////////////////////////////////

VCSFileInfo::FileState CVSFileInfoProvider::String2EnumState( QString stateAsString )
{
    // @todo add more status as "Conflict" and "Sticky" (but I dunno how CVS writes it so I'm going
    // to await until I have a conflict or somebody else fix it ;-)
    // @todo use QRegExp for better matching since it seems strings have changed between CVS releases :-(
    // @todo a new state for 'Needs patch'
    if (stateAsString == "Up-to-date")
        return VCSFileInfo::Uptodate;
    else if (stateAsString == "Locally Modified")
        return VCSFileInfo::Modified;
    else if (stateAsString == "Locally Added")
        return VCSFileInfo::Added;
    else if (stateAsString == "Unresolved Conflict")
        return VCSFileInfo::Conflict;
    else if (stateAsString == "Needs Patch")
        return VCSFileInfo::NeedsPatch;
    else if (stateAsString == "Needs Checkout")
        return VCSFileInfo::NeedsCheckout;
    else
        return VCSFileInfo::Unknown; /// \FIXME exhaust all the previous cases first ;-)
}

///////////////////////////////////////////////////////////////////////////////

void CVSFileInfoProvider::printOutFileInfoMap( const VCSFileInfoMap &map )
{
    kdDebug(9006) << "Files parsed:" << endl;
    for (VCSFileInfoMap::const_iterator it = map.begin(); it != map.end(); ++it)
    {
        const VCSFileInfo &vcsInfo = *it;
        kdDebug(9006) << vcsInfo.toString() << endl;
    }
}

#include "cvsfileinfoprovider.moc"
