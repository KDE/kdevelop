/***************************************************************************
*   Copyright (C) 2003 by Roberto Raggi                                   *
*   roberto@kdevelop.org                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "kdevkdelibsimporter.h"
#include "kdevkdelibsimporter.moc"
#include "settingsdialog.h"

#include <kdebug.h>
#include <kgenericfactory.h>

#include <qvaluestack.h>
#include <qlabel.h>
#include <qdir.h>
#include <qcombobox.h>

K_EXPORT_COMPONENT_FACTORY( libkdevkdelibsimporter, KGenericFactory<KDevKDELibsImporter>( "kdevkdelibsimporter" ) )

KDevKDELibsImporter::KDevKDELibsImporter( QObject * parent, const char * name, const QStringList& )
		: KDevPCSImporter( parent, name )
{}

KDevKDELibsImporter::~KDevKDELibsImporter()
{}

QStringList KDevKDELibsImporter::fileList( const QString& path )
{
	QDir dir( path );
	QStringList lst = dir.entryList( "*.h" );
	QStringList fileList;
	for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it )
	{
		fileList.push_back( dir.absPath() + "/" + ( *it ) );
	}
	return fileList;
}


QStringList KDevKDELibsImporter::fileList()
{
	if ( !m_settings )
		return QStringList();

	QStringList files;
	int scope = m_settings->cbParsingScope->currentItem();
	if ( scope == 0 )
	{
		files += fileList( m_settings->kdeDir() );
		files += fileList( m_settings->kdeDir() + "/arts" );
		files += fileList( m_settings->kdeDir() + "/artsc" );
		files += fileList( m_settings->kdeDir() + "/dcopc" );
		files += fileList( m_settings->kdeDir() + "/dom" );
		files += fileList( m_settings->kdeDir() + "/kabc" );
		files += fileList( m_settings->kdeDir() + "/kdeprint" );
		files += fileList( m_settings->kdeDir() + "/kdesu" );
		files += fileList( m_settings->kdeDir() + "/kio" );
		files += fileList( m_settings->kdeDir() + "/kjs" );
		files += fileList( m_settings->kdeDir() + "/kparts" );
		files += fileList( m_settings->kdeDir() + "/ktexteditor" );
	}
	else if ( scope == 1 )
	{
		QValueStack<QString> s;
		s.push( m_settings->kdeDir() );
		files += fileList( m_settings->kdeDir() );

		QDir dir;
		do
		{
			dir.setPath( s.pop() );
			kdDebug( 9015 ) << "Examining: " << dir.path() << endl;
			const QFileInfoList *dirEntries = dir.entryInfoList();
			QPtrListIterator<QFileInfo> it( *dirEntries );
			for ( ; it.current(); ++it )
			{
				QString fileName = it.current() ->fileName();
				if ( fileName == "." || fileName == ".." )
					continue;
				QString path = it.current() ->absFilePath();
				if ( it.current() ->isDir() )
				{
					kdDebug( 9015 ) << "Pushing: " << path << endl;
					s.push( path );
					files += fileList( path );
				}
			}
		}
		while ( !s.isEmpty() );
	}

	return files;
}

QStringList KDevKDELibsImporter::includePaths()
{
	if ( !m_settings )
		return QStringList();

	QStringList includePaths;
	includePaths.push_back( m_settings->kdeDir() );
	return includePaths;
}

QWidget * KDevKDELibsImporter::createSettingsPage( QWidget * parent, const char * name )
{
	m_settings = new SettingsDialog( parent, name );
	return m_settings;
}
//kate: indent-mode csands; tab-width 4; space-indent off;


