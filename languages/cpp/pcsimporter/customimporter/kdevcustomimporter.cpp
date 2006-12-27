/***************************************************************************
*   Copyright (C) 2003 by Alexander Dymo                                  *
*   cloudtemple@mksat.net                                                 *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/
#include "kdevcustomimporter.h"

#include "settingsdialog.h"

#include <qvaluestack.h>
#include <qdir.h>

#include <kdebug.h>
#include <kgenericfactory.h>

K_EXPORT_COMPONENT_FACTORY( libkdevcustompcsimporter, KGenericFactory<KDevCustomImporter>( "kdevcustompcsimporter" ) )

KDevCustomImporter::KDevCustomImporter( QObject* parent, const char* name, const QStringList & // args
                                      )
		: KDevPCSImporter( parent, name )
{}


KDevCustomImporter::~KDevCustomImporter()
{}


QString KDevCustomImporter::dbName() const
{
	return m_settings->dbName();
}

QStringList KDevCustomImporter::fileList( const QString& path )
{
	QDir dir( path );
	if ( !dir.exists() )
		return QStringList();
// 	QStringList lst = dir.entryList( "*.h;*.H;*.hh;*.hxx;*.hpp;*.tlh" );
	QStringList lst = dir.entryList( m_settings->filePattern() );
	QStringList fileList;
	for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it )
	{
		fileList.push_back( dir.absPath() + "/" + ( *it ) );
	}
	return fileList;
}

QStringList KDevCustomImporter::fileList()
{
	if ( !m_settings )
		return QStringList();

	QStringList lst = m_settings->dirs();
	QStringList files;
	for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it )
	{
		if ( !m_settings->recursive() )
			files += fileList( *it );
		else
			processDir( *it, files );
	}

	return files;
}

QStringList KDevCustomImporter::includePaths()
{
	if ( !m_settings )
		return QStringList();

	return m_settings->dirs();
}

QWidget* KDevCustomImporter::createSettingsPage( QWidget* parent, const char* name )
{
	m_settings = new SettingsDialog( parent, name );
	return m_settings;
}

void KDevCustomImporter::processDir( const QString path, QStringList & files )
{
	QValueStack<QString> s;
	s.push( path );
	files += fileList( path );

	QDir dir;
	do
	{
		dir.setPath( s.pop() );
		if (!dir.exists())
		    continue;
		kdDebug( 9015 ) << "Examining: " << dir.path() << endl;
		const QFileInfoList *dirEntries = dir.entryInfoList();
		if ( !dirEntries ) continue;
		QPtrListIterator<QFileInfo> it( *dirEntries );
		for ( ; dirEntries && it.current(); ++it )
		{
			QString fileName = it.current() ->fileName();
			if ( fileName == "." || fileName == ".." )
				continue;
			if ( it.current() ->isDir() )
			{
                                QString tmpPath = it.current() ->absFilePath();
				kdDebug( 9015 ) << "Pushing: " << tmpPath << endl;
				s.push( tmpPath );
				files += fileList( tmpPath );
			}
		}
	}
	while ( !s.isEmpty() );
}

#include "kdevcustomimporter.moc" 
//kate: indent-mode csands; tab-width 4; space-indent off;
