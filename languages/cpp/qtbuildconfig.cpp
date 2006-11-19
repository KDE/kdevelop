/*
	Copyright (C) 2005 by Tobias Erbsland <te@profzone.ch>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	version 2, License as published by the Free Software Foundation.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public License
	along with this library; see the file COPYING.LIB.  If not, write to
	the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	Boston, MA 02111-1307, USA.
*/
#include "qtbuildconfig.h"
#include "cppsupportpart.h"

#include <domutil.h>

#include <kdebug.h>
#include <qdom.h>

#include <stdlib.h>

const QString QtBuildConfig::m_configRoot = QString( "/kdevcppsupport/qt" );

QtBuildConfig::QtBuildConfig( CppSupportPart * part, QDomDocument* dom )
	: QObject( part ), m_part( part ), m_dom( dom )
{
	init();
}

QtBuildConfig::~QtBuildConfig()
{
}

void QtBuildConfig::init( )
{
	m_used = DomUtil::readBoolEntry( *m_dom, m_configRoot + "/used", false );
	m_version = DomUtil::readIntEntry( *m_dom, m_configRoot + "/version", 3 );
	if( m_version < 3 || m_version > 4 )
	{
		m_version = 3;
	}
	m_includeStyle = DomUtil::readIntEntry( *m_dom, m_configRoot + "/includestyle", 3 );
	if( m_includeStyle < 3 || m_includeStyle > 4 )
	{
		m_includeStyle = m_version;
	}
	m_root = DomUtil::readEntry( *m_dom, m_configRoot + "/root" );
	m_designerIntegration = DomUtil::readEntry( *m_dom, m_configRoot + "/designerintegration" );
	if( m_designerIntegration.isEmpty() )
	{
		if ( m_version == 3 )
			m_designerIntegration = "EmbeddedKDevDesigner";
		else
			m_designerIntegration = "ExternalDesigner";
	}
}

void QtBuildConfig::store( )
{
	DomUtil::writeBoolEntry( *m_dom, m_configRoot + "/used", m_used );
	DomUtil::writeIntEntry( *m_dom, m_configRoot + "/version", m_version );
	DomUtil::writeIntEntry( *m_dom, m_configRoot + "/includestyle", m_includeStyle );
	DomUtil::writeEntry( *m_dom, m_configRoot + "/root", m_root );
	DomUtil::writeEntry( *m_dom, m_configRoot + "/designerintegration", m_designerIntegration );

	emit stored();
}

void QtBuildConfig::setUsed( bool used )
{
	m_used = used;
}

void QtBuildConfig::setVersion( int version )
{
	m_version = version;
}

void QtBuildConfig::setIncludeStyle( int style )
{
	m_includeStyle = style;
}

void QtBuildConfig::setRoot( const QString& root )
{
	m_root = root;
}

void QtBuildConfig::setDesignerIntegration( const QString& designerIntegration )
{
	m_designerIntegration = designerIntegration;
}
#include "qtbuildconfig.moc"

// kate: indent-mode csands; tab-width 4; space-indent off;
