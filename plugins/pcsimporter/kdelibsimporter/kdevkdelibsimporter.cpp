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

#include <kgenericfactory.h>

#include <qlabel.h>
#include <qdir.h>

K_EXPORT_COMPONENT_FACTORY( libkdevkdelibsimporter, KGenericFactory<KDevKDELibsImporter>( "kdevkdelibsimporter" ) );

KDevKDELibsImporter::KDevKDELibsImporter( QObject * parent, const char * name, const QStringList& )
    : KDevPCSImporter( parent, name )
{
}

KDevKDELibsImporter::~KDevKDELibsImporter()
{
}

QStringList KDevKDELibsImporter::fileList( const QString& path )
{
    QDir dir( path );
    QStringList lst = dir.entryList( "*.h" );
    QStringList fileList;
    for( QStringList::Iterator it=lst.begin(); it!=lst.end(); ++it )
    {
        fileList.push_back( dir.absPath() + "/" + (*it) );
    }
    return fileList;
}


QStringList KDevKDELibsImporter::fileList()
{
    if( !m_settings )
	return QStringList();

    QStringList files;
    files += fileList( m_settings->kdeDir() + "/include" );
    files += fileList( m_settings->kdeDir() + "/include/kio" );
    files += fileList( m_settings->kdeDir() + "/include/kparts" );
    files += fileList( m_settings->kdeDir() + "/include/ktexteditor" );

    return files;
}

QStringList KDevKDELibsImporter::includePaths()
{
    if( !m_settings )
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

