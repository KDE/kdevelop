//
//
// C++ Interface: svnoptions
//
// Description:
// Defines default command line options for SVN commands.
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <kdebug.h>

#include "domutil.h"

#include "svnoptions.h"

#define default_svn 	QString::fromLatin1("")
#define default_commit	QString::fromLatin1("")
#define default_update	QString::fromLatin1("")
#define default_add	QString::fromLatin1("")
#define default_remove	QString::fromLatin1("")
#define default_revert	QString::fromLatin1("")
#define default_diff	QString::fromLatin1("")
#define default_log	QString::fromLatin1("")
#define default_rsh	QString::fromLatin1("")

SvnOptions *SvnOptions::m_instance = 0;

SvnOptions::SvnOptions()
	: m_svn(default_svn), m_commit(default_commit),
	m_update(default_update), m_add(default_add),
	m_remove(default_remove), m_revert(default_revert),
	m_diff(default_diff), m_log(default_log),
	m_rsh(default_rsh)
{
}

SvnOptions::~SvnOptions()
{
	m_instance = 0;
}

SvnOptions* SvnOptions::instance()
{
	if (!m_instance)
	{
		m_instance = new SvnOptions();
	}
	return m_instance;
}

void SvnOptions::save( QDomDocument &dom )
{
	kdDebug( 9999 ) << " **** SvnOptions::save( QDomDocument &) here" << endl;

	DomUtil::writeEntry( dom, "/kdevsvn/svnoptions", m_svn );
	DomUtil::writeEntry( dom, "/kdevsvn/commitoptions", m_commit );
	DomUtil::writeEntry( dom, "/kdevsvn/addoptions", m_add );
	DomUtil::writeEntry( dom, "/kdevsvn/logoptions", m_log );
	DomUtil::writeEntry( dom, "/kdevsvn/updateoptions", m_update );
	DomUtil::writeEntry( dom, "/kdevsvn/removeoptions", m_remove );
	DomUtil::writeEntry( dom, "/kdevsvn/revertoptions", m_revert );
	DomUtil::writeEntry( dom, "/kdevsvn/diffoptions", m_diff );
	DomUtil::writeEntry( dom, "/kdevsvn/rshoptions", m_rsh );
}

void SvnOptions::load( const QDomDocument &dom )
{
	kdDebug( 9999 ) << " **** SvnOptions::load( const QDomDocument &) here" << endl;

	m_svn     = DomUtil::readEntry( dom, "/kdevsvn/svnoptions", default_svn );
	m_commit  = DomUtil::readEntry( dom, "/kdevsvn/commitoptions", default_commit );
	m_add     = DomUtil::readEntry( dom, "/kdevsvn/addoptions", default_add );
	m_log     = DomUtil::readEntry( dom, "/kdevsvn/logoptions", default_log );
	m_update  = DomUtil::readEntry( dom, "/kdevsvn/updateoptions", default_update );
	m_remove  = DomUtil::readEntry( dom, "/kdevsvn/removeoptions", default_remove );
	m_revert  = DomUtil::readEntry( dom, "/kdevsvn/revertoptions", default_revert );
	m_diff    = DomUtil::readEntry( dom, "/kdevsvn/diffoptions", default_diff );
	m_rsh     = DomUtil::readEntry( dom, "/kdevsvn/rshoptions", default_rsh );
}

void SvnOptions::setSvn( const QString &p )
{
	m_svn = p;
}

QString SvnOptions::svn()
{
	return m_svn;
}

void SvnOptions::setCommit( const QString &p )
{
	m_commit = p;
}

QString SvnOptions::commit()
{
	return m_commit;
}

void SvnOptions::setUpdate( const QString &p )
{
	m_update = p;
}

QString SvnOptions::update()
{
	return m_update;
}

void SvnOptions::setAdd( const QString &p )
{
	m_add = p;
}

QString SvnOptions::add()
{
	return m_add;
}

void SvnOptions::setRemove( const QString &p )
{
	m_remove = p;
}

QString SvnOptions::remove()
{
	return m_remove;
}

void SvnOptions::setRevert( const QString &p )
{
	m_revert = p;
}

QString SvnOptions::revert()
{
	return m_revert;
}

void SvnOptions::setDiff( const QString &p )
{
	m_diff = p;
}

QString SvnOptions::diff()
{
	return m_diff;
}

void SvnOptions::setLog( const QString &p )
{
	m_log = p;
}

QString SvnOptions::log()
{
	return m_log;
}

void SvnOptions::setRsh( const QString &p )
{
	m_rsh = p;
}

QString SvnOptions::rsh()
{
	return m_rsh;
}
