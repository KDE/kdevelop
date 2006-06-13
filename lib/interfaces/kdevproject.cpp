/* This file is part of the KDE project
   Copyright (C) 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
   Copyright (C) 2003 Mario Scalas <mario.scalas@libero.it>
   Copyright (C) 2003-2004 Alexander Dymo <adymo@kdevelop.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <kdebug.h>

#include "kdevproject.h"
#include <qfileinfo.h>
#include <QTimer>
#include <dbus/qdbus.h>
#include "filetemplate.h"

struct KDevProject::Private {
    QMap<QString, QString> m_absToRel;
    QStringList m_symlinkList;
    QTimer *m_timer;
    QHash<QString, QString> m_templExpandMap;
    QHash<QString, QString> m_templExpandMapXML;
};

KDevProject::KDevProject(const KDevPluginInfo *info, QObject *parent)
    : KDevPlugin(info, parent), d(new KDevProject::Private())
{
    QDBus::sessionBus().registerObject("/org/kdevelop/Project",
                                       this, QDBusConnection::ExportSlots);

    connect( this, SIGNAL(addedFilesToProject(const QStringList& )), this, SLOT(buildFileMap()) );
    connect( this, SIGNAL(removedFilesFromProject(const QStringList& )), this, SLOT(buildFileMap()) );

    connect( this, SIGNAL(addedFilesToProject(const QStringList& )), this, SLOT(slotAddFilesToFileMap(const QStringList& )) ); 
    connect( this, SIGNAL(removedFilesFromProject(const QStringList& )), this, SLOT(slotRemoveFilesFromFileMap(const QStringList& )) ); 
    
    d->m_timer = new QTimer(this);
    connect(d->m_timer, SIGNAL(timeout()), this, SLOT(slotBuildFileMap()));
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
    return d->m_absToRel.contains( absFileName );
}

QString KDevProject::relativeProjectFile( const QString & absFileName )
{
    if( isProjectFile(absFileName) )
	return d->m_absToRel[ absFileName ];
    return QString();
}

void KDevProject::buildFileMap()
{
    d->m_timer->stop();
    d->m_timer->start(0, true);
}

void KDevProject::slotBuildFileMap()
{
    kDebug(9000) << k_funcinfo << endl;

    d->m_absToRel.clear();
    d->m_symlinkList.clear();
    const QStringList fileList = allFiles();
    for( QStringList::ConstIterator it=fileList.begin(); it!=fileList.end(); ++it )
    {
	QFileInfo fileInfo( projectDirectory() + "/" + *it );
	d->m_absToRel[ fileInfo.canonicalFilePath() ] = *it;
	
        if ( fileInfo.canonicalFilePath() != fileInfo.absoluteFilePath() )
        {
            d->m_symlinkList << *it;
        }
    }
}

void KDevProject::openProject( const QString & /*dirName*/, const QString & /*projectName*/ )
{
    buildFileMap();
    readSubstitutionMap();
}

QStringList KDevProject::symlinkProjectFiles( )
{
    return d->m_symlinkList;
}

void KDevProject::slotAddFilesToFileMap( const QStringList & fileList )
{
	QStringList::ConstIterator it = fileList.begin();
	while( it != fileList.end() )
	{
		QFileInfo fileInfo( projectDirectory() + "/" + *it );
		d->m_absToRel[ fileInfo.canonicalFilePath() ] = *it;
		
    	if ( fileInfo.canonicalFilePath() != fileInfo.absoluteFilePath() )
		{
			d->m_symlinkList << *it;
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
    	d->m_absToRel.remove( fileInfo.canonicalFilePath() );
		
		d->m_symlinkList.remove( *it );

		++it;
	}
}

void KDevProject::readSubstitutionMap()
{
    d->m_templExpandMap = DomUtil::readHashEntry(*projectDom(), "substmap");
    d->m_templExpandMapXML = FileTemplate::normalSubstMapToXML(d->m_templExpandMap);
}

const QHash<QString, QString>& KDevProject::substMap( SubstitutionMapTypes type )
{
    switch( type )
    {
    default:
    case NormalFile:
        return d->m_templExpandMap;
    case XMLFile:
        return d->m_templExpandMapXML;
    }
}

#include "kdevproject.moc"
