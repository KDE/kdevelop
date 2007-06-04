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

namespace KDevelop
{

struct _Catalog_Private
{
  QString dbName;
  CatalogBackend* backend;
  bool enabled;

  _Catalog_Private( CatalogBackend* bkend)
    : backend( bkend )
  {
  }

};


/*!
  \fn  Catalog::Catalog
*/
Catalog::Catalog( CatalogBackend* bkend)
  : d( new _Catalog_Private( bkend ) )
{
}

/*!
  \fn  Catalog::~Catalog
*/
Catalog::~Catalog()
{
  close();
  delete( d );
  d = 0;
}

/*!
  \fn  Catalog::indexList() const
*/
QList<QByteArray>  Catalog::indexList() const
{
  return d->backend->indexList();
}

bool  Catalog::enabled() const
{
  return d->enabled;
}

void  Catalog::setEnabled( bool isEnabled )
{
  d->enabled = isEnabled;
}

/*!
  \fn  Catalog::addIndex( const QString& name )
  @todo document these functions
*/
void  Catalog::addIndex( const QByteArray& name )
{
  d->backend->addIndex( name );
}

/*!
  \fn  Catalog::close()
*/
 
void  Catalog::close()
{
  d->backend->close();
  d->dbName = QString();
}

/*!
  \fn  Catalog::open( const QString& dbName )
*/
 
void  Catalog::open( const QString& dbName )
{
  if ( d->backend->open( dbName )
    d->dbName = dbName;
}

/*!
  \fn  Catalog::dbName() const
*/
 
QString  Catalog::dbName() const
{
  return d->dbName;
}

/*!
  \fn  Catalog::isValid() const
*/
 
bool  Catalog::isValid() const
{
  return d->backend->isOpen();
}

/*!
  \fn  Catalog::addItem( Tag& tag )
*/
 
void  Catalog::addItem( Tag& tag )
{
  if( tag.name().isEmpty() )
    return;

  QByteArray id = generateId();

  tag.setId( id );
  d->backend->addItem(id, tag);
}

/*!
  \fn  Catalog::getItemById( const QString& id )
*/
 
Tag  Catalog::getItemById( const QByteArray& id )
{

  return d->backend->getItemById( id );
  
}

/*!
  \fn  Catalog::sync()
*/
 
void  Catalog::sync()
{
  d->backend->sync();
}

/*!
  \fn  Catalog::query( const QValueList<QueryArgument>& args )
*/
 
QList<Tag>  Catalog::query( const QList<QueryArgument>& args )
{
  return d->backend->query( args );
  
}

QByteArray Catalog::generateId()
{
  static int n = 1;
  static char buffer[1024];
  qsnprintf(buffer, 1024, "%05d", n++ );
  return buffer;
}

}

//kate: space-indent on; indent-width 2; replace-tabs on;

