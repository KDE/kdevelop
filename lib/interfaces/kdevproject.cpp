/* This file is part of the KDE project
   Copyright (C) 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
   Copyright (C) 2003 Mario Scalas <mario.scalas@libero.it>
   Copyright (C) 2003 Alexander Dymo <cloudtemple@mksat.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <kdebug.h>

#include "kdevproject.h"
#include <urlutil.h>
#include <qfileinfo.h>

KDevProject::KDevProject( const QString& pluginName, const QString& icon, QObject *parent, const char *name)
    : KDevPlugin( pluginName, icon, parent, name)
{
//    connect( this, SIGNAL(addedFilesToProject(const QStringList& )), this, SLOT(slotBuildFileMap()) ); // too expensive
//    connect( this, SIGNAL(removedFilesFromProject(const QStringList& )), this, SLOT(slotBuildFileMap()) ); // too expensive

//    connect( this, SIGNAL(changedFilesInProject(const QStringList& )), this, SLOT(slotBuildFileMap()) ); // no reason for this one

    connect( this, SIGNAL(addedFilesToProject(const QStringList& )), this, SLOT(slotAddFilesToFileMap(const QStringList& )) ); 
    connect( this, SIGNAL(removedFilesFromProject(const QStringList& )), this, SLOT(slotRemoveFilesFromFileMap(const QStringList& )) ); 
}

KDevProject::~KDevProject()
{
}

void KDevProject::changedFile( const QString & fileName )
{
    QStringList fileList;
    fileList.append ( fileName );

    emit changedFilesInProject( fileList );

}

void KDevProject::changedFiles( const QStringList & fileList )
{
    emit changedFilesInProject( fileList );
}

KDevProject::Options KDevProject::options() const
{
    return (KDevProject::Options)0;
}

bool KDevProject::isProjectFile( const QString & absFileName )
{
    return m_absToRel.contains( absFileName );
}

QString KDevProject::relativeProjectFile( const QString & absFileName )
{
    if( isProjectFile(absFileName) )
	return m_absToRel[ absFileName ];
    return QString::null;
}

void KDevProject::slotBuildFileMap( )
{
	kdDebug(9000) << k_funcinfo << endl;

    m_absToRel.clear();
    m_symlinkList.clear();
    const QStringList fileList = allFiles();
    for( QStringList::ConstIterator it=fileList.begin(); it!=fileList.end(); ++it )
    {
	QFileInfo fileInfo( projectDirectory() + "/" + *it );
	m_absToRel[ URLUtil::canonicalPath(fileInfo.absFilePath()) ] = *it;
	
        if ( URLUtil::canonicalPath( fileInfo.absFilePath() ) != fileInfo.absFilePath() )
        {
            m_symlinkList << *it;
        }
    }
}

void KDevProject::openProject( const QString & /*dirName*/, const QString & /*projectName*/ )
{
    slotBuildFileMap();
}

QStringList KDevProject::symlinkProjectFiles( )
{
    return m_symlinkList;
}

void KDevProject::slotAddFilesToFileMap( const QStringList & fileList )
{
	QStringList::ConstIterator it = fileList.begin();
	while( it != fileList.end() )
	{
		QFileInfo fileInfo( projectDirectory() + "/" + *it );
		m_absToRel[ URLUtil::canonicalPath(fileInfo.absFilePath()) ] = *it;
		
		if ( URLUtil::canonicalPath( fileInfo.absFilePath() ) != fileInfo.absFilePath() )
		{
			m_symlinkList << *it;
		}

		++it;
	}
}

void KDevProject::slotRemoveFilesFromFileMap( const QStringList & fileList )
{
	QStringList::ConstIterator it = fileList.begin();
	while( it != fileList.end() )
	{
		QFileInfo fileInfo( projectDirectory() + "/" + *it );
		m_absToRel.remove( URLUtil::canonicalPath(fileInfo.absFilePath()) );
		
		m_symlinkList.remove( *it );

		++it;
	}
}

#include "kdevproject.moc"
