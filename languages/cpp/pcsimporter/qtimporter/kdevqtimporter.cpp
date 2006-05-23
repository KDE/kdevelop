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

#include "kdevqtimporter.h"
#include "kdevqtimporter.moc"
#include "settingsdialog.h"

#include <kgenericfactory.h>

#include <qlabel.h>
#include <qdir.h>

K_EXPORT_COMPONENT_FACTORY( libkdevqtimporter, KGenericFactory<KDevQtImporter>( "kdevqtimporter" ) )

KDevQtImporter::KDevQtImporter( QObject * parent, const char * name, const QStringList& )
		: KDevPCSImporter( parent, name )
{}

KDevQtImporter::~KDevQtImporter()
{}

QStringList KDevQtImporter::fileList()
{
	if ( !m_settings )
		return QStringList();

	QDir dir( m_settings->qtDir() );
	QStringList lst = dir.entryList( "*.h" );
	QStringList fileList;
	for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it )
	{
		if ( ( *it ).startsWith( "qconfig-" ) )
		{
			if ( ( *it ).endsWith( m_settings->configuration() + ".h" ) )
				fileList.prepend( dir.absPath() + "/" + ( *it ) );
		}
		else
		{
			fileList.push_back( dir.absPath() + "/" + ( *it ) );
		}
	}
	return fileList;
}

QStringList KDevQtImporter::includePaths()
{
	if ( !m_settings )
		return QStringList();

	QStringList includePaths;
	includePaths.push_back( m_settings->qtDir() );
	includePaths.push_back( m_settings->qtDir() + "/private" );
	includePaths.push_back( m_settings->qtDir() + "/default" );

	/// @todo add mkspec
	return includePaths;
}

QWidget * KDevQtImporter::createSettingsPage( QWidget * parent, const char * name )
{
	m_settings = new SettingsDialog( parent, name );
	return m_settings;
}

//kate: indent-mode csands; tab-width 4; space-indent off;
