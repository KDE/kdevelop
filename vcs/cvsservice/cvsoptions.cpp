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
#include <qtextstream.h>

#include <kdebug.h>
#include <kconfig.h>
#include <klocale.h>

#include "domutil.h"
#include "kdevproject.h"
#include "cvsoptions.h"

///////////////////////////////////////////////////////////////////////////////
// Macros
///////////////////////////////////////////////////////////////////////////////

#define default_revert          QString::fromLatin1("-C")
#define default_diff            QString::fromLatin1("-p")
#define default_rsh             QString::fromLatin1("")
#define default_contextLines    3
#define default_compression     0

///////////////////////////////////////////////////////////////////////////////
// static members
///////////////////////////////////////////////////////////////////////////////

CvsOptions *CvsOptions::m_instance = 0;
QString CvsOptions::invalidLocation( "ERROR-LOCATION-IS-NOT-SET-IN-PROJECT" );

///////////////////////////////////////////////////////////////////////////////
// class CvsOptions
///////////////////////////////////////////////////////////////////////////////

CvsOptions::CvsOptions()
    : m_recursiveWhenCommitRemove( true ),
    m_pruneEmptyDirsWhenUpdate( true ),
    m_recursiveWhenUpdate( true ),
    m_createDirsWhenUpdate( true ),
    m_revertOptions( default_revert ),
    m_diffOptions( default_diff ),
    m_cvsRshEnvVar( default_rsh ),
    m_compressionLevel( default_compression ),
    m_contextLines( default_contextLines )
{
    kdDebug( 9006 ) << " **** CvsOptions instance CREATED!" << endl;
    // We share some configuration data with cvsservice
    m_serviceConfig = new KConfig( "cvsservicerc" );
}

///////////////////////////////////////////////////////////////////////////////

CvsOptions::~CvsOptions()
{
    kdDebug( 9006 ) << " **** CvsOptions instance DESTROYED!" << endl;
    delete m_serviceConfig;

    m_instance = 0;
}

///////////////////////////////////////////////////////////////////////////////

CvsOptions* CvsOptions::instance()
{
    if (!m_instance)
    {
        m_instance = new CvsOptions();
    }
    return m_instance;
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptions::save( KDevProject *project )
{
    kdDebug( 9006 ) << " **** CvsOptions::save( KDevProject* ) here" << endl;
    Q_ASSERT( project );

    QDomDocument &dom = *project->projectDom();

    DomUtil::writeBoolEntry( dom, "/kdevcvsservice/recursivewhenupdate", recursiveWhenUpdate() );
    DomUtil::writeBoolEntry( dom, "/kdevcvsservice/prunedirswhenupdate", pruneEmptyDirsWhenUpdate() );
    DomUtil::writeBoolEntry( dom, "/kdevcvsservice/createdirswhenupdate", createDirsWhenUpdate() );
    DomUtil::writeBoolEntry( dom, "/kdevcvsservice/recursivewhencommitremove", recursiveWhenCommitRemove() );
    DomUtil::writeEntry( dom, "/kdevcvsservice/revertoptions", revertOptions() );
//    DomUtil::writeEntry( dom, "/kdevcvsservice/location", location() );

    // [Repository-:ext:anonymous@cvs.ogre.sourceforge.net:/cvsroot/ogrenew]
    QString groupName = "Repository-" + guessLocation( project->projectDirectory() );
    m_serviceConfig->setGroup( groupName );

    m_serviceConfig->writeEntry( "ContextLines", contextLines() );
    m_serviceConfig->writeEntry( "DiffOptions", diffOptions() );
    m_serviceConfig->writeEntry( "rsh", cvsRshEnvVar() );
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptions::load( KDevProject *project )
{
    kdDebug( 9006 ) << " **** CvsOptions::load( KDevProject* ) here" << endl;
    Q_ASSERT( project );
    QDomDocument &dom = *project->projectDom();

    m_recursiveWhenUpdate = DomUtil::readBoolEntry( dom, "/kdevcvsservice/recursivewhenupdate", true );
    m_pruneEmptyDirsWhenUpdate = DomUtil::readBoolEntry( dom, "/kdevcvsservice/prunedirswhenupdate", true );
    m_createDirsWhenUpdate = DomUtil::readBoolEntry( dom, "/kdevcvsservice/createdirswhenupdate", true );
    m_recursiveWhenCommitRemove = DomUtil::readBoolEntry( dom, "/kdevcvsservice/recursivewhencommitremove", true );
    m_revertOptions = DomUtil::readEntry( dom, "/kdevcvsservice/revertoptions", default_revert );
//    m_location = DomUtil::readEntry( dom, "/kdevcvsservice/location", guessLocation( project->projectDirectory() ) );

    QString groupName = "Repository-" + guessLocation( project->projectDirectory() );
    m_serviceConfig->setGroup( groupName );

    m_contextLines = m_serviceConfig->readUnsignedNumEntry( "ContextLines", default_contextLines );
    m_diffOptions  = m_serviceConfig->readEntry( "DiffOptions", default_diff );
    m_cvsRshEnvVar = m_serviceConfig->readEntry( "rsh", default_rsh );
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptions::setRecursiveWhenCommitRemove( bool b )
{
    this->m_recursiveWhenCommitRemove = b;
}

///////////////////////////////////////////////////////////////////////////////

bool CvsOptions::recursiveWhenCommitRemove() const
{
    return this->m_recursiveWhenCommitRemove;
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptions::setPruneEmptyDirsWhenUpdate( bool b )
{
    this->m_pruneEmptyDirsWhenUpdate = b;
}

///////////////////////////////////////////////////////////////////////////////

bool CvsOptions::pruneEmptyDirsWhenUpdate() const
{
    return this->m_pruneEmptyDirsWhenUpdate;
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptions::setRecursiveWhenUpdate( bool b )
{
    this->m_recursiveWhenUpdate = b;
}

///////////////////////////////////////////////////////////////////////////////

bool CvsOptions::recursiveWhenUpdate() const
{
    return this->m_recursiveWhenUpdate;
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptions::setCreateDirsWhenUpdate( bool b )
{
    this->m_createDirsWhenUpdate = b;
}

///////////////////////////////////////////////////////////////////////////////

bool CvsOptions::createDirsWhenUpdate() const
{
    return this->m_createDirsWhenUpdate;
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptions::setRevertOptions( const QString &p )
{
    this->m_revertOptions = p;
}

///////////////////////////////////////////////////////////////////////////////

QString CvsOptions::revertOptions()
{
    return this->m_revertOptions;
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptions::setDiffOptions( const QString &p )
{
    this->m_diffOptions = p;
}

///////////////////////////////////////////////////////////////////////////////

QString CvsOptions::diffOptions()
{
    return this->m_diffOptions;
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptions::setCvsRshEnvVar( const QString &p )
{
    this->m_cvsRshEnvVar = p;
}

///////////////////////////////////////////////////////////////////////////////

QString CvsOptions::cvsRshEnvVar()
{
    return this->m_cvsRshEnvVar;
}

///////////////////////////////////////////////////////////////////////////////

QString CvsOptions::location()
{
    return m_location;
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptions::setLocation( const QString &p )
{
    m_location = p;
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptions::setContextLines( unsigned int contextLines )
{
    m_contextLines = contextLines;
}

///////////////////////////////////////////////////////////////////////////////

unsigned int CvsOptions::contextLines() const
{
    return m_contextLines;
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptions::setCompressionLevel( unsigned int compressionLevel )
{
    m_compressionLevel = compressionLevel;
}

///////////////////////////////////////////////////////////////////////////////

unsigned int CvsOptions::compressionLevel() const
{
    return m_compressionLevel;
}

///////////////////////////////////////////////////////////////////////////////

QString CvsOptions::guessLocation( const QString &projectDir ) const
{
    QString rootFileName( projectDir + "/CVS/Root" );

    QFile f( rootFileName );
    if (f.open( QIODevice::ReadOnly ))
    {
        QTextStream t( &f );
        QString serverLocation = t.readLine();
        kdDebug(9000) << "===> Server location guessed: " << serverLocation << endl;
        return serverLocation;
    }
    else
    {
        kdDebug(9000) << "===> Error: could not open CVS/Entries!! " << endl;
        return i18n( "Error while guessing repository location." );
    }
}
