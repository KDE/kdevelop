/***************************************************************************
  Copyright 2006 David Nolden <david.nolden.kdevelop@art-master.de>
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
#include "kdevteamwork_part.h"
#include <kio/netaccess.h>
#include <krandom.h>
#include <QStringList>
#include <QFileInfo>
#include <QFile>
#include <QDateTime>
#include <kdebug.h>
#include <kurl.h>

/* Exclude this file from doublequote_chars check as krazy doesn't understand
std::string*/
//krazy:excludeall=doubleqote_chars

TeamworkFolderManager* TeamworkFolderManager::m_self = 0;

KUrl TeamworkFolderManager::workspaceDirectory() {
  return m_self->m_workspaceDir;
}

TeamworkFolderManager::TeamworkFolderManager( const KUrl& directory ) {
  m_self = this;

  KUrl u = directory;
  u.cleanPath();

  m_workspaceDir = u;
  u.addPath( ".teamwork" );
  u.cleanPath();
  m_teamworkDir = u;
}

TeamworkFolderManager* TeamworkFolderManager::self() {
  return m_self;
}

KUrl TeamworkFolderManager::teamworkAbsolute( const QString& subDirectory, const QString& subFolder ) {
  KUrl ret( self()->m_teamworkDir );
  ret.addPath( subFolder );
  if( subDirectory.startsWith( ret.toLocalFile() ) ) return subDirectory;
  ret.addPath( subDirectory );
  ret.cleanPath();
  return ret;
}

KUrl TeamworkFolderManager::workspaceAbsolute( const QString& subDirectory, const QString& subFolder ) {
  KUrl ret( self()->m_workspaceDir );
  ret.addPath( subFolder );
  if( subDirectory.startsWith( ret.toLocalFile() ) ) return subDirectory;
  ret.addPath( subDirectory );
  ret.cleanPath();
  return ret;
}

void TeamworkFolderManager::createTeamworkFolder() throw(QString) {
  KUrl ul = self()->m_workspaceDir;

  ul.addPath( ".teamwork" );
  if ( !KIO::NetAccess::exists( ul, true, 0 ) )
    if( !KIO::NetAccess::mkdir( ul, 0 ) )
      throw QString( "could not create \"%1\"" ).arg( ul.prettyUrl() );
}


KUrl TeamworkFolderManager::createUniqueDirectory( const QString& subFolder, const QString& name, const QString& namePrefix, const QString& nameSuffix ) throw(QString) {
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
      throw QString( "could not create directory %1 directory" ).arg( ul.prettyUrl() );

    sub.pop_front();
  }


  KUrl nu = ul;
  nu.addPath( namePrefix + name + nameSuffix );
  nu.cleanPath();

  if( !KIO::NetAccess::exists( nu, true, 0 ) ){
    KIO::NetAccess::mkdir( nu, 0 );

    if ( KIO::NetAccess::exists( nu, true, 0 ) )
      return nu;
  }

    ///If the file exists try it with additional date/time
  nu = ul;
  nu.addPath( namePrefix + name + QDateTime::currentDateTime().toString( Qt::ISODate ) + nameSuffix );

  nu.cleanPath();
  if( !KIO::NetAccess::exists( nu, true, 0 ) ) {
    KIO::NetAccess::mkdir( nu, 0 );

    if ( KIO::NetAccess::exists( nu, true, 0 ) )
      return nu;
  }

    ///If even this file exists, add a suffix behind the date
  for( int a = 0; a < 100; a++ ) {
    nu = ul;
    nu.addPath( namePrefix + QString("_%1_").arg( a ) + name + QDateTime::currentDateTime().toString( Qt::ISODate ) + nameSuffix );
    nu.cleanPath();
    if( !KIO::NetAccess::exists( nu, true, 0 ) ){
      KIO::NetAccess::mkdir( nu, 0 );

      if ( KIO::NetAccess::exists( nu, true, 0 ) )
        return nu;
    }
  }

  throw QString( "failed to allocate filename for %1" ).arg( name + "." + nameSuffix );
}

KUrl TeamworkFolderManager::createUniqueFile( const QString& subFolder, const QString& extension, const QString& name, const QString& namePrefix, const QString& nameSuffix ) throw(QString) {
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
        throw QString( "could not create directory %1 directory" ).arg( ul.prettyUrl() );

      sub.pop_front();
    }


    KUrl nu = ul;
    nu.addPath( namePrefix + name + nameSuffix + "." + extension );

    nu.cleanPath();
    if( !KIO::NetAccess::exists( nu, true, 0 ) ){
      if( createFile( nu ) )
        return nu;
    }

    ///If the file exists try it with additional date/time
    nu = ul;
    nu.addPath( namePrefix + name + QDateTime::currentDateTime().toString( Qt::ISODate ) + nameSuffix + "." + extension );

    nu.cleanPath();
    if( !KIO::NetAccess::exists( nu, true, 0 ) ) {
      if( createFile( nu ) )
        return nu;
    }

    ///If even this file exists, add a suffix behind the date
    for( int a = 0; a < 100; a++ ) {
      nu = ul;
      nu.addPath( namePrefix + QString("_%1_").arg( a ) + name + QDateTime::currentDateTime().toString( Qt::ISODate ) + nameSuffix + "." + extension );
      nu.cleanPath();
      if( !KIO::NetAccess::exists( nu, true, 0 ) ){
      if( createFile( nu ) )
        return nu;
      }
    }

    throw QString( "failed to allocate filename for %1" ).arg( name + "." + nameSuffix );
}

KUrl TeamworkFolderManager::createUniqueFile( const QString& subFolder, const QString& fileName, const QString& namePrefix, const QString& nameSuffix ) throw(QString) {
  QFileInfo i( fileName );
  KUrl u( subFolder );
  u.addPath( i.path() );
  return createUniqueFile( u.toLocalFile(), i.completeSuffix(), i.baseName(), namePrefix, nameSuffix );
}

void TeamworkFolderManager::registerTempItem( const KUrl& u ) {
  if( u.isRelative() )
    self()->m_tempItems[teamworkAbsolute(u.toLocalFile()).pathOrUrl()] = true;
  else
    self()->m_tempItems[u.pathOrUrl()] = true;
}

QString TeamworkFolderManager::teamworkRelative( const KUrl& url, const QString& subfolder ) {
  KUrl u = self()->m_teamworkDir;
  u.addPath( subfolder );
  u.adjustPath( KUrl::AddTrailingSlash );
  return KUrl::relativeUrl( u, url );
}

QString TeamworkFolderManager::workspaceRelative( const KUrl& url, const QString& subfolder ) {
  KUrl u = self()->m_workspaceDir;
  u.addPath( subfolder );
  u.adjustPath( KUrl::AddTrailingSlash );
  //kDebug() << "workspaceRelative( " << url << ", " << subfolder << " ) called. Worspace: " << self()->m_workspaceDir << " base: " << u << " result: " << KUrl::relativeUrl( u, url ) << endl;
  return KUrl::relativeUrl( u, url );
}

TeamworkFolderManager::~TeamworkFolderManager() {
  for( QMap< QString, bool >::iterator it = m_tempItems.begin();it != m_tempItems.end(); ++it ) {
    ///First, make sure that the file is really a subfolder of the .teamwork-directory
    KUrl f(  it.key() );
    f.cleanPath();
    if( (f.path()).startsWith( m_teamworkDir.path(KUrl::AddTrailingSlash) ) && (f.pathOrUrl()).startsWith( m_teamworkDir.pathOrUrl() ) ) {
      if( ! KIO::NetAccess::del( f, 0 ) )
        kDebug() << "TeamworkFolderManager error: File " << f.prettyUrl() << " could not be deleted" << endl;
    } else {
      kDebug() << "TeamworkFolderManager error: File " << f.prettyUrl() << " was registered as temporary file, but is not in folder " << m_teamworkDir << endl;
    }
  }
  m_tempItems.clear();
}

///@todo use netaccess!
bool TeamworkFolderManager::createFile( const KUrl& url ) {
  QFile f( url.path() );
  if( f.open(QIODevice::WriteOnly) )
    return true;
  else
    return false;
}


// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
