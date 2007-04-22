/***************************************************************************
  copyright            : (C) 2006 by David Nolden
  email                : david.nolden.kdevelop@art-master.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "teamworkfoldermanager.h"
#include <icore.h>
#include <iprojectcontroller.h>
#include <iproject.h>
#include "kdevteamwork.h"
#include <kio/netaccess.h>
#include <krandom.h>
#include <QStringList>
#include <QFileInfo>
#include <QFile>
#include <QDateTime>
#include <kdebug.h>


TeamworkFolderManager* TeamworkFolderManager::m_self = 0;

QString TeamworkFolderManager::workspaceDirectory() {
  return m_self->m_workspaceDir;
}



TeamworkFolderManager::TeamworkFolderManager() {
  m_self = this;
  if( KDevTeamwork::core()->projectController()->projects().isEmpty() ) {
    kdDebug() << "ERROR: Teamwork-plugin started with no project open";
    return;
  }
  KUrl u = KDevTeamwork::core()->projectController()->projects().front()->folder();
  m_workspaceDir = u.path();
  u.addPath( ".teamwork" );
  u.cleanPath();
  m_teamworkDir = u.path();
}

TeamworkFolderManager* TeamworkFolderManager::self() {
  return m_self;
}

QString TeamworkFolderManager::absolute( const QString& subDirectory, const QString& subFolder ) {
  KUrl ret( self()->m_teamworkDir );
  ret.addPath( subFolder );
  if( subDirectory.startsWith( ret.path() ) ) return subDirectory;
  ret.addPath( subDirectory );
  return ret.path();
}

QString TeamworkFolderManager::workspaceAbsolute( const QString& subDirectory, const QString& subFolder ) {
  KUrl ret( self()->m_workspaceDir );
  ret.addPath( subFolder );
  if( subDirectory.startsWith( ret.path() ) ) return subDirectory;
  ret.addPath( subDirectory );
  return ret.path();
}

QString TeamworkFolderManager::createUniqueDirectory( QString subFolder, QString name, QString namePrefix, QString nameSuffix ) throw(QString) {
  if( self()->m_workspaceDir.isEmpty() )
    throw QString( "no teamwork-workspace-directory set" );
  KUrl ul = self()->m_workspaceDir;

  ul.addPath( ".teamwork" );
  if ( !KIO::NetAccess::exists( ul, true, 0 ) )
    KIO::NetAccess::mkdir( ul, 0 );

  if ( !KIO::NetAccess::exists( ul, true, 0 ) )
    throw QString( "could not create .teamwork-directory" );

    
  QStringList sub = subFolder.split( "/" );
  while( !sub.isEmpty() ) {
    ul.addPath( sub.front() );
    if ( !KIO::NetAccess::exists( ul, true, 0 ) )
      KIO::NetAccess::mkdir( ul, 0 );

    if ( !KIO::NetAccess::exists( ul, true, 0 ) )
      throw QString( "could not create directory %1 directory" ).arg( ul.path() );
      
    sub.pop_front();
  }

    
  KUrl nu = ul;
  nu.addPath( namePrefix + name + nameSuffix );
  nu.cleanPath();
  
  if( !KIO::NetAccess::exists( nu, true, 0 ) ){
    KIO::NetAccess::mkdir( nu, 0 );

    if ( KIO::NetAccess::exists( nu, true, 0 ) )
      return nu.path();
  }

    ///If the file exists try it with additional date/time
  nu = ul;
  nu.addPath( namePrefix + name + QDateTime::currentDateTime().toString( Qt::ISODate ) + nameSuffix );
    
  nu.cleanPath();
  if( !KIO::NetAccess::exists( nu, true, 0 ) ) {
    KIO::NetAccess::mkdir( nu, 0 );

    if ( KIO::NetAccess::exists( nu, true, 0 ) )
      return nu.path();
  }

    ///If even this file exists, add a suffix behind the date
  for( int a = 0; a < 100; a++ ) {
    nu = ul;
    nu.addPath( namePrefix + QString("_%1_").arg( a ) + name + QDateTime::currentDateTime().toString( Qt::ISODate ) + nameSuffix );
    nu.cleanPath();
    if( !KIO::NetAccess::exists( nu, true, 0 ) ){
      KIO::NetAccess::mkdir( nu, 0 );

      if ( KIO::NetAccess::exists( nu, true, 0 ) )
        return nu.path();
    }
  }

  throw QString( "failed to allocate filename for %1" ).arg( name + "." + nameSuffix );
}

QString TeamworkFolderManager::createUniqueFile( QString subFolder, QString extension, QString name, QString namePrefix, QString nameSuffix ) throw(QString) {
  if( self()->m_workspaceDir.isEmpty() )
    throw QString( "no teamwork-workspace-directory set" );
    KUrl ul = self()->m_workspaceDir;

    ul.addPath( ".teamwork" );
    if ( !KIO::NetAccess::exists( ul, true, 0 ) )
      KIO::NetAccess::mkdir( ul, 0 );

    if ( !KIO::NetAccess::exists( ul, true, 0 ) )
      throw QString( "could not create .teamwork-directory" );

    
    QStringList sub = subFolder.split( "/" );
    while( !sub.isEmpty() ) {
      ul.addPath( sub.front() );
      if ( !KIO::NetAccess::exists( ul, true, 0 ) )
        KIO::NetAccess::mkdir( ul, 0 );

      if ( !KIO::NetAccess::exists( ul, true, 0 ) )
        throw QString( "could not create directory %1 directory" ).arg( ul.path() );
      
      sub.pop_front();
    }

    
    KUrl nu = ul;
    nu.addPath( namePrefix + name + nameSuffix + "." + extension );

    nu.cleanPath();
    if( !KIO::NetAccess::exists( nu, true, 0 ) ){
      QFile f( nu.path() );
      if( f.open(QIODevice::WriteOnly) )
        return nu.path();
    }

    ///If the file exists try it with additional date/time
    nu = ul;
    nu.addPath( namePrefix + name + QDateTime::currentDateTime().toString( Qt::ISODate ) + nameSuffix + "." + extension );
    
    nu.cleanPath();
    if( !KIO::NetAccess::exists( nu, true, 0 ) ) {
      QFile f( nu.path() );
      if( f.open(QIODevice::WriteOnly) )
        return nu.path();
    }

    ///If even this file exists, add a suffix behind the date
    for( int a = 0; a < 100; a++ ) {
      nu = ul;
      nu.addPath( namePrefix + QString("_%1_").arg( a ) + name + QDateTime::currentDateTime().toString( Qt::ISODate ) + nameSuffix + "." + extension );
      nu.cleanPath();
      if( !KIO::NetAccess::exists( nu, true, 0 ) ){
        QFile f( nu.path() );
        if( f.open(QIODevice::WriteOnly) )
          return nu.path();
      }
    }

    throw QString( "failed to allocate filename for %1" ).arg( name + "." + nameSuffix );
}

QString TeamworkFolderManager::createUniqueFile( QString subFolder, QString fileName, QString namePrefix, QString nameSuffix ) throw(QString) {
  QFileInfo i( fileName );
  KUrl u( subFolder );
  u.addPath( i.path() );
  return createUniqueFile( u.path(), i.completeSuffix(), i.baseName(), namePrefix, nameSuffix );
}

void TeamworkFolderManager::registerTempItem( const QString& u ) {
  self()->m_tempItems[absolute(u)] = true;
}

QString TeamworkFolderManager::relative( const QString& url, const QString& subfolder ) {
  KUrl u = self()->m_teamworkDir;
  u.addPath( subfolder );
  u.adjustPath( KUrl::AddTrailingSlash );
  return KUrl::relativeUrl( u, KUrl(url) );
}

QString TeamworkFolderManager::workspaceRelative( const QString& url, const QString& subfolder ) {
  KUrl u = self()->m_workspaceDir;
  u.addPath( subfolder );
  u.adjustPath( KUrl::AddTrailingSlash );
  return KUrl::relativeUrl( u, KUrl(url) );
}

TeamworkFolderManager::~TeamworkFolderManager() {
  for( QMap< QString, bool >::iterator it = m_tempItems.begin();it != m_tempItems.end(); ++it ) {
    ///First, make sure that the file is really a subfolder of the .teamwork-directory
    KUrl f(  it.key() );
    f.cleanPath();
    if( (f.path()).startsWith( m_teamworkDir ) ) {
      if( ! KIO::NetAccess::del( f, 0 ) )
        kDebug() << "TeamworkFolderManager error: File " << f << " could not be deleted" << endl;
    } else {
      kDebug() << "TeamworkFolderManager error: File " << f << " was registered as temporary file, but is not in folder " << m_teamworkDir << endl;
    }
  }
  m_tempItems.clear();
}

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
