/* This file is part of KDevelop
   Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>

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
#include "kdevcatalog.h"

#include <cstring>
#include <cstdlib>

#include <QDir>
#include <QFile>
#include <QList>
#include <QFileInfo>
#include <QByteArray>
#include <QDataStream>

#include <krandomsequence.h>
#include <kdebug.h>

#include <db.h>

#include <config.h>

struct _KDevCatalog_Private
{
  QString dbName;
  KDevCatalogBackend* backend;
  bool enabled;

  _KDevCatalog_Private( KDevCatalogBackend* bkend)
    : backend( bkend )
  {
  }

};


/*!
  \fn  KDevCatalog::KDevCatalog
*/
KDevCatalog::KDevCatalog( KDevCatalogBackend* bkend)
  : d( new _KDevCatalog_Private( bkend ) )
{
}

/*!
  \fn  KDevCatalog::~KDevCatalog
*/
KDevCatalog::~KDevCatalog()
{
  close();
  delete( d );
  d = 0;
}

/*!
  \fn  KDevCatalog::indexList() const
*/
QList<QByteArray>  KDevCatalog::indexList() const
{
  return d->backend->indexList();
}

bool  KDevCatalog::enabled() const
{
  return d->enabled;
}

void  KDevCatalog::setEnabled( bool isEnabled )
{
  d->enabled = isEnabled;
}

/*!
  \fn  KDevCatalog::addIndex( const QString& name )
  @todo document these functions
*/
void  KDevCatalog::addIndex( const QByteArray& name )
{
  d->backend->addIndex( name );
}

/*!
  \fn  KDevCatalog::close()
*/
 
void  KDevCatalog::close()
{
  d->backend->close();
  d->dbName = QString();
}

/*!
  \fn  KDevCatalog::open( const QString& dbName )
*/
 
void  KDevCatalog::open( const QString& dbName )
{
  if ( d->backend->open( dbName )
    d->dbName = dbName;
}

/*!
  \fn  KDevCatalog::dbName() const
*/
 
QString  KDevCatalog::dbName() const
{
  return d->dbName;
}

/*!
  \fn  KDevCatalog::isValid() const
*/
 
bool  KDevCatalog::isValid() const
{
  return d->backend->isOpen();
}

/*!
  \fn  KDevCatalog::addItem( Tag& tag )
*/
 
void  KDevCatalog::addItem( Tag& tag )
{
  if( tag.name().isEmpty() )
    return;

  QByteArray id = generateId();

  tag.setId( id );
  d->backend->addItem(id, tag);
}

/*!
  \fn  KDevCatalog::getItemById( const QString& id )
*/
 
Tag  KDevCatalog::getItemById( const QByteArray& id )
{

  return d->backend->getItemById( id );
  
}

/*!
  \fn  KDevCatalog::sync()
*/
 
void  KDevCatalog::sync()
{
  d->backend->sync();
}

/*!
  \fn  KDevCatalog::query( const QValueList<QueryArgument>& args )
*/
 
QList<Tag>  KDevCatalog::query( const QList<QueryArgument>& args )
{
  return d->backend->query( args );
  
}

QByteArray KDevCatalog::generateId()
{
  static int n = 1;
  static char buffer[1024];
  qsnprintf(buffer, 1024, "%05d", n++ );
  return buffer;
}

//kate: space-indent on; indent-width 2; replace-tabs on;

