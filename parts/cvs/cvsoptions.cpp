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

#include <kdebug.h>

#include "domutil.h"

#include "cvsoptions.h"

#define default_cvs 	QString::fromLatin1("-f")
#define default_commit	QString::fromLatin1("")
#define default_update	QString::fromLatin1("-dP")
#define default_add		QString::fromLatin1("")
#define default_remove	QString::fromLatin1("-f")
#define default_revert	QString::fromLatin1("-C -d -P")
#define default_diff	QString::fromLatin1("-u3 -p")
#define default_log		QString::fromLatin1("")
#define default_rsh		QString::fromLatin1("")

CvsOptions *CvsOptions::m_instance = 0;

CvsOptions::CvsOptions()
	: m_cvs(default_cvs), m_commit(default_commit),
	m_update(default_update), m_add(default_add),
	m_remove(default_remove), m_revert(default_revert),
	m_diff(default_diff), m_log(default_log),
	m_rsh(default_rsh)
{
}

CvsOptions::~CvsOptions()
{
	m_instance = 0;
}

CvsOptions* CvsOptions::instance()
{
	if (!m_instance)
	{
		m_instance = new CvsOptions();
	}
	return m_instance;
}

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
	DomUtil::writeEntry( dom, "/kdevcvs/diffoptions", m_diff );
	DomUtil::writeEntry( dom, "/kdevcvs/rshoptions", m_rsh );
}

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

void CvsOptions::setRevert( const QString &p )
{
	m_revert = p;
}

QString CvsOptions::revert()
{
	return m_revert;
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
