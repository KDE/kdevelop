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
	m_root = DomUtil::readEntry( *m_dom, m_configRoot + "/root", "" );
    m_qmakePath = DomUtil::readEntry(*m_dom, m_configRoot + "/qmake", "");
    m_designerPath = DomUtil::readEntry(*m_dom, m_configRoot + "/designer", "");

    if( m_root.isEmpty() || !isValidQtDir( m_root ) )
    {
        findQtDir();
    }
    if( m_qmakePath.isEmpty() || !isExecutable( m_qmakePath ) )
    {
        m_qmakePath = findExecutable( "qmake-qt"+ QString::number( m_version ) );
        if( m_qmakePath.isEmpty() || !isExecutable( m_qmakePath ) )
            m_qmakePath = findExecutable( "qmake" );
    }
    if( m_designerPath.isEmpty() || !isExecutable( m_designerPath ) )
    {
        m_designerPath = findExecutable( "designer-qt"+QString::number( m_version ) );
        if( m_designerPath.isEmpty() || !isExecutable( m_designerPath ) )
            m_designerPath = findExecutable( "designer" );
    }

	m_designerIntegration = DomUtil::readEntry( *m_dom, m_configRoot + "/designerintegration" );
	if( m_designerIntegration.isEmpty() )
	{
		if ( m_version == 3 )
			m_designerIntegration = "EmbeddedKDevDesigner";
		else
			m_designerIntegration = "ExternalDesigner";
	}
}

bool QtBuildConfig::isValidQtDir( const QString& path ) const
{
    QFileInfo inc( path + QString( QChar( QDir::separator() ) )+
                   "include"+QString( QChar( QDir::separator() ) )+
                   "qt.h" );
    return ( m_version == 4 || ( m_version != 4 && inc.exists() ) );
}

void QtBuildConfig::buildBinDirs( QStringList & dirs ) const
{
    if( m_version == 3 )
    {
        if( !m_root.isEmpty() )
            dirs << (m_root + QString( QChar( QDir::separator() ) ) + "bin");
        dirs << (::getenv("QTDIR") + QString( QChar( QDir::separator() ) ) + "bin");
    }
    QStringList paths = QStringList::split(":",::getenv("PATH"));
    dirs += paths;
    QString binpath = QDir::rootDirPath() + "bin";
    if( dirs.findIndex( binpath ) != -1 )
        dirs << binpath;

    binpath = QDir::rootDirPath() + "usr" + QString( QChar( QDir::separator() ) ) + "bin";
    if( dirs.findIndex( binpath ) != -1 )
        dirs << binpath;
    binpath = QDir::rootDirPath() + "usr" + QString( QChar( QDir::separator() ) ) + "local" + QString( QChar( QDir::separator() ) ) + "bin";
    if( dirs.findIndex( binpath ) != -1 )
        dirs << binpath;
}


QString QtBuildConfig::findExecutable( const QString& execname ) const
{
    QStringList dirs;
    buildBinDirs( dirs );

    for( QStringList::Iterator it=dirs.begin(); it!=dirs.end(); ++it )
    {
        QString designer = *it + QString( QChar( QDir::separator() ) ) + execname;
        if( !designer.isEmpty() && isExecutable( designer ) )
        {
            return designer;
        }
    }
    return "";
}

bool QtBuildConfig::isExecutable( const QString& path ) const
{
    QFileInfo fi(path);
    return( fi.exists() && fi.isExecutable() );
}

void QtBuildConfig::findQtDir()
{
    QStringList qtdirs;
    if( m_version == 3 )
        qtdirs.push_back( ::getenv("QTDIR") );
    qtdirs.push_back( QDir::rootDirPath()+"usr"+QString( QChar( QDir::separator() ) )+"lib"+QString( QChar( QDir::separator() ) )+"qt"+QString("%1").arg( m_version ) );
    qtdirs.push_back( QDir::rootDirPath()+"usr"+QString( QChar( QDir::separator() ) )+"lib"+QString( QChar( QDir::separator() ) )+"qt"+QString( QChar( QDir::separator() ) )+QString("%1").arg( m_version ) );
    qtdirs.push_back( QDir::rootDirPath()+"usr"+QString( QChar( QDir::separator() ) )+"share"+QString( QChar( QDir::separator() ) )+"qt"+QString("%1").arg( m_version ) );
    qtdirs.push_back( QDir::rootDirPath()+"usr" );
    qtdirs.push_back( QDir::rootDirPath()+"usr"+QString( QChar( QDir::separator() ) )+"lib"+QString( QChar( QDir::separator() ) )+"qt" );

    for( QStringList::Iterator it=qtdirs.begin(); it!=qtdirs.end(); ++it )
    {
        QString qtdir = *it;
        if( !qtdir.isEmpty() && isValidQtDir(qtdir) )
        {
            m_root = qtdir;
            return;
        }
    }
}

void QtBuildConfig::store( )
{
	DomUtil::writeBoolEntry( *m_dom, m_configRoot + "/used", m_used );
	DomUtil::writeIntEntry( *m_dom, m_configRoot + "/version", m_version );
	DomUtil::writeIntEntry( *m_dom, m_configRoot + "/includestyle", m_includeStyle );
	DomUtil::writeEntry( *m_dom, m_configRoot + "/root", m_root );
	DomUtil::writeEntry( *m_dom, m_configRoot + "/designerintegration", m_designerIntegration );
    DomUtil::writeEntry(*m_dom, m_configRoot + "/qmake", m_qmakePath );
    DomUtil::writeEntry(*m_dom, m_configRoot + "/designer", m_designerPath );

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

void QtBuildConfig::setQMakePath( const QString& path )
{
    m_qmakePath = path;
}

void QtBuildConfig::setDesignerPath( const QString& path )
{
    m_designerPath = path;
}

void QtBuildConfig::setDesignerIntegration( const QString& designerIntegration )
{
	m_designerIntegration = designerIntegration;
}
#include "qtbuildconfig.moc"

//kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
