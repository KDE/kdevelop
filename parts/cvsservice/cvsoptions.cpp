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

#include <kdebug.h>
#include <kconfig.h>


#include "domutil.h"

#include "cvsoptions.h"

#define default_cvs             QString::fromLatin1("-f")
#define default_commit          QString::fromLatin1("")
#define default_update          QString::fromLatin1("-dP")
#define default_add             QString::fromLatin1("")
#define default_remove          QString::fromLatin1("-f")
#define default_revert          QString::fromLatin1("-C -d -P")
#define default_diff            QString::fromLatin1("-u3 -p")
#define default_log             QString::fromLatin1("")
#define default_rsh             QString::fromLatin1("")
#define default_contextLines    3
#define default_compression     0

CvsOptions *CvsOptions::m_instance = 0;

///////////////////////////////////////////////////////////////////////////////
// class CvsOptions
///////////////////////////////////////////////////////////////////////////////

CvsOptions::CvsOptions()
    : m_cvs(default_cvs), m_commit(default_commit),
    m_update(default_update), m_add(default_add),
    m_remove(default_remove), m_revert(default_revert),
    m_diff(default_diff), m_log(default_log),
    m_rsh(default_rsh), m_compressionLevel( default_compression ),
    m_contextLines( default_contextLines )
{
    kdDebug( 9999 ) << " **** CvsOptions instance CREATED!" << endl;

    m_serviceConfig = new KConfig( "cvsservicerc" );
}

///////////////////////////////////////////////////////////////////////////////

CvsOptions::~CvsOptions()
{
    kdDebug( 9999 ) << " **** CvsOptions instance DESTROYED!" << endl;
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

void CvsOptions::save( QDomDocument &dom )
{
    kdDebug( 9999 ) << " **** CvsOptions::save( QDomDocument &) here" << endl;

    DomUtil::writeEntry( dom, "/kdevcvs/cvsoptions", m_cvs );
    DomUtil::writeEntry( dom, "/kdevcvs/commitoptions", m_commit );
    DomUtil::writeEntry( dom, "/kdevcvs/addoptions", m_add );
    DomUtil::writeEntry( dom, "/kdevcvs/logoptions", m_log );
    DomUtil::writeEntry( dom, "/kdevcvs/updateoptions", m_update );
    DomUtil::writeEntry( dom, "/kdevcvs/removeoptions", m_remove );
    DomUtil::writeEntry( dom, "/kdevcvs/revertoptions", m_revert );
//    DomUtil::writeEntry( dom, "/kdevcvs/rshoptions", m_rsh );
//    DomUtil::writeEntry( dom, "/kdevcvs/diffoptions", m_diff );
//    DomUtil::writeIntEntry( dom, "/kdevcvs/diffcontextLines", m_contextLines );

    m_serviceConfig->setGroup( "General" );
    m_serviceConfig->writeEntry( "ContextLines", m_contextLines );
    m_serviceConfig->writeEntry( "DiffOptions", m_diff );
    m_serviceConfig->writeEntry( "rsh", m_rsh );
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptions::load( const QDomDocument &dom )
{
    kdDebug( 9999 ) << " **** CvsOptions::load( const QDomDocument &) here" << endl;

    m_cvs     = DomUtil::readEntry( dom, "/kdevcvs/cvsoptions", default_cvs );
    m_commit  = DomUtil::readEntry( dom, "/kdevcvs/commitoptions", default_commit );
    m_add     = DomUtil::readEntry( dom, "/kdevcvs/addoptions", default_add );
    m_log     = DomUtil::readEntry( dom, "/kdevcvs/logoptions", default_log );
    m_update  = DomUtil::readEntry( dom, "/kdevcvs/updateoptions", default_update );
    m_remove  = DomUtil::readEntry( dom, "/kdevcvs/removeoptions", default_remove );
    m_revert  = DomUtil::readEntry( dom, "/kdevcvs/revertoptions", default_revert );
//    m_rsh     = DomUtil::readEntry( dom, "/kdevcvs/rshoptions", default_rsh );
//    m_diff    = DomUtil::readEntry( dom, "/kdevcvs/diffoptions", default_diff );
//    m_contextLines = DomUtil::readIntEntry( dom, "/kdevcvs/diffcontextLines", default_contextLines );

    m_serviceConfig->setGroup( "General" );
    m_contextLines = m_serviceConfig->readUnsignedNumEntry( "ContextLines", default_contextLines );
    m_diff         = m_serviceConfig->readEntry( "DiffOptions", default_diff );
    m_rsh          = m_serviceConfig->readEntry( "rsh", default_rsh );
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptions::setCvs( const QString &p )
{
    m_cvs = p;
}

///////////////////////////////////////////////////////////////////////////////

QString CvsOptions::cvs()
{
    return m_cvs;
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptions::setCommit( const QString &p )
{
    m_commit = p;
}

///////////////////////////////////////////////////////////////////////////////

QString CvsOptions::commit()
{
    return m_commit;
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptions::setUpdate( const QString &p )
{
    m_update = p;
}

///////////////////////////////////////////////////////////////////////////////

QString CvsOptions::update()
{
    return m_update;
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptions::setAdd( const QString &p )
{
    m_add = p;
}

///////////////////////////////////////////////////////////////////////////////

QString CvsOptions::add()
{
    return m_add;
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptions::setRemove( const QString &p )
{
    m_remove = p;
}

///////////////////////////////////////////////////////////////////////////////

QString CvsOptions::remove()
{
    return m_remove;
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptions::setRevert( const QString &p )
{
    m_revert = p;
}

///////////////////////////////////////////////////////////////////////////////

QString CvsOptions::revert()
{
    return m_revert;
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptions::setDiff( const QString &p )
{
    m_diff = p;
}

///////////////////////////////////////////////////////////////////////////////

QString CvsOptions::diff()
{
    return m_diff;
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptions::setLog( const QString &p )
{
    m_log = p;
}

///////////////////////////////////////////////////////////////////////////////

QString CvsOptions::log()
{
    return m_log;
}

///////////////////////////////////////////////////////////////////////////////

void CvsOptions::setRsh( const QString &p )
{
    m_rsh = p;
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

QString CvsOptions::rsh()
{
    return m_rsh;
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
