//
//
// C++ Interface: cvsoptions
//
// Description:
// Defines default command line options for CVS commands.
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "domutil.h"

#include "cvsoptions.h"

CvsOptions *CvsOptions::m_instance = 0;

CvsOptions::CvsOptions()
	: m_cvs(QString::fromLatin1("-f")), m_commit(QString::fromLatin1("")),
	m_update(QString::fromLatin1("-dP")), m_add(QString::fromLatin1("")),
	m_remove(QString::fromLatin1("-f")), m_replace(QString::fromLatin1("-C -d -P")),
	m_diff(QString::fromLatin1("-u3 -p")), m_log(QString::fromLatin1("")),
	m_rsh(QString::fromLatin1(""))
{
}

CvsOptions::~CvsOptions()
{
	m_instance = 0;
}

void CvsOptions::save( QDomDocument &dom )
{
	DomUtil::writeEntry( dom, "/kdevcvs/cvsoptions", m_cvs );
	DomUtil::writeEntry( dom, "/kdevcvs/commitoptions", m_commit );
	DomUtil::writeEntry( dom, "/kdevcvs/addoptions", m_add );
	DomUtil::writeEntry( dom, "/kdevcvs/logoptions", m_log );
	DomUtil::writeEntry( dom, "/kdevcvs/updateoptions", m_update );
	DomUtil::writeEntry( dom, "/kdevcvs/removeoptions", m_remove );
	DomUtil::writeEntry( dom, "/kdevcvs/replaceoptions", m_replace );
	DomUtil::writeEntry( dom, "/kdevcvs/diffoptions", m_diff );
	DomUtil::writeEntry( dom, "/kdevcvs/rshoptions", m_rsh );
}

void CvsOptions::load( const QDomDocument &dom )
{
	m_cvs     = DomUtil::readEntry( dom, "/kdevcvs/cvsoptions", default_cvs );
	m_commit  = DomUtil::readEntry( dom, "/kdevcvs/commitoptions", default_commit );
	m_add     = DomUtil::readEntry( dom, "/kdevcvs/addoptions", default_add );
	m_log     = DomUtil::readEntry( dom, "/kdevcvs/logoptions", default_log );
	m_update  = DomUtil::readEntry( dom, "/kdevcvs/updateoptions", default_update );
	m_remove  = DomUtil::readEntry( dom, "/kdevcvs/removeoptions", default_remove );
	m_replace = DomUtil::readEntry( dom, "/kdevcvs/replaceoptions", default_replace );
	m_diff    = DomUtil::readEntry( dom, "/kdevcvs/diffoptions", default_diff );
	m_rsh     = DomUtil::readEntry( dom, "/kdevcvs/rshoptions", default_rsh );
}

void CvsOptions::setCvs( const QString &p )
{
	m_cvs = p;
}

QString CvsOptions::cvs()
{
	return m_cvs;
}

void CvsOptions::setCommit( const QString &p )
{
	m_commit = p;
}

QString CvsOptions::commit()
{
	return m_commit;
}

void CvsOptions::setUpdate( const QString &p )
{
	m_update = p;
}

QString CvsOptions::update()
{
	return m_update;
}

void CvsOptions::setAdd( const QString &p )
{
	m_add = p;
}

QString CvsOptions::add()
{
	return m_add;
}

void CvsOptions::setRemove( const QString &p )
{
	m_remove = p;
}

QString CvsOptions::remove()
{
	return m_remove;
}

void CvsOptions::setReplace( const QString &p )
{
	m_replace = p;
}

QString CvsOptions::replace()
{
	return m_replace;
}

void CvsOptions::setDiff( const QString &p )
{
	m_diff = p;
}

QString CvsOptions::diff()
{
	return m_diff;
}

void CvsOptions::setLog( const QString &p )
{
	m_log = p;
}

QString CvsOptions::log()
{
	return m_log;
}

void CvsOptions::setRsh( const QString &p )
{
	m_rsh = p;
}

QString CvsOptions::rsh()
{
	return m_rsh;
}
