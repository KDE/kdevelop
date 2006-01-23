

/* This file is part of the KDE project
   Copyright (C) 2005 Ian Reinhart Geiser <geiseri@kde.org>
   
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

#include "kdevprojectiface.h"
#include "kdevproject.h"
#include "domutil.h"

KDevProjectIface::KDevProjectIface(KDevProject *prj)
  : QObject(prj), DCOPObject("KDevProject"), m_prj(prj)
{
}


KDevProjectIface::~KDevProjectIface()
{
}

void KDevProjectIface::openProject( const QString & dirName, const QString & projectName )
{
  m_prj->openProject(dirName,projectName);
}

void KDevProjectIface::closeProject( )
{
  m_prj->closeProject();
}

uint KDevProjectIface::options( ) const
{
  return (uint)m_prj->options();
}

QString KDevProjectIface::projectDirectory( ) const
{
  return m_prj->projectDirectory();
}

QString KDevProjectIface::projectName( ) const
{
  return m_prj->projectName();
}

QString KDevProjectIface::mainProgram( bool relative ) const
{
  return m_prj->mainProgram(relative);
}

QString KDevProjectIface::runDirectory( ) const
{
  return m_prj->runDirectory();
}

QString KDevProjectIface::activeDirectory( ) const
{
  return m_prj->activeDirectory();
}

QString KDevProjectIface::buildDirectory( ) const
{
  return m_prj->buildDirectory();
}

QStringList KDevProjectIface::allFiles( ) const
{
  return m_prj->allFiles();
}

QStringList KDevProjectIface::distFiles( ) const
{
  return m_prj->distFiles();
}

void KDevProjectIface::addFiles( const QStringList & fileList )
{
  m_prj->addFiles(fileList);
}

void KDevProjectIface::addFile( const QString & fileName )
{
  m_prj->addFile(fileName);
}

void KDevProjectIface::removeFiles( const QStringList & fileList )
{
  m_prj->removeFiles(fileList);
}

void KDevProjectIface::removeFile( const QString & fileName )
{
  m_prj->removeFile(fileName);
}

void KDevProjectIface::changedFiles( const QStringList & fileList )
{
  m_prj->changedFiles(fileList);
}

void KDevProjectIface::changedFile( const QString & fileName )
{
  m_prj->changedFile(fileName);
}

bool KDevProjectIface::isProjectFile( const QString & absFileName )
{
  return m_prj->isProjectFile(absFileName);
}

QString KDevProjectIface::relativeProjectFile( const QString & absFileName )
{
  return m_prj->relativeProjectFile(absFileName);
}

QStringList KDevProjectIface::symlinkProjectFiles( )
{
  return m_prj->symlinkProjectFiles();
}

QString KDevProjectIface::readEntry( const QString & path, const QString & defaultEntry )
{
  return DomUtil::readEntry( *m_prj->projectDom(), path, defaultEntry);
}

int KDevProjectIface::readIntEntry( const QString & path, int defaultEntry )
{
  return DomUtil::readIntEntry(*m_prj->projectDom(), path,defaultEntry);
}

bool KDevProjectIface::readBoolEntry( const QString & path, bool defaultEntry )
{
  return DomUtil::readBoolEntry(*m_prj->projectDom(), path, defaultEntry);
}

QStringList KDevProjectIface::readListEntry( const QString & path, const QString & tag )
{
  return DomUtil::readListEntry(*m_prj->projectDom(), path, tag);
}

QMap< QString, QString > KDevProjectIface::readMapEntry( const QString & path )
{
  return DomUtil::readMapEntry(*m_prj->projectDom(), path);
}

void KDevProjectIface::writeEntry( const QString & path, const QString & value )
{
  DomUtil::writeEntry(*m_prj->projectDom(), path, value);
}

void KDevProjectIface::writeIntEntry( const QString & path, int value )
{
  DomUtil::writeIntEntry(*m_prj->projectDom(), path, value);
}

void KDevProjectIface::writeBoolEntry( const QString & path, bool value )
{
  DomUtil::writeBoolEntry(*m_prj->projectDom(), path, value);
}

void KDevProjectIface::writeListEntry( const QString & path, const QString & tag, const QStringList & value )
{
  DomUtil::writeListEntry(*m_prj->projectDom(), path, tag, value);
}

void KDevProjectIface::writeMapEntry( const QString & path, const QMap< QString, QString > & map )
{
  DomUtil::writeMapEntry(*m_prj->projectDom(), path, map);
}

#include "kdevprojectiface.moc"
