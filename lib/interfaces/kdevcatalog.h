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

#ifndef KDEVCATALOG_H
#define KDEVCATALOG_H

#include <qlist.h>
#include <qpair.h>
#include <QVariant>
#include <QByteArray>

#include "tag.h"
#include "kdevexport.h"

/**
   @file catalog.h
   KDevCatalog database - the persistant symbol store database.
*/

/**
   KDevCatalog objects represent separate symbol databases.
   KDevCatalogs can be created/loaded/unloaded dynamically.
   To find a symbol in the repository each catalog should be queried.

   Persistant symbol store is useful to keep information about code that
   never or rarely changes. System libraries are perfect examples of such code.
*/
class KDEVINTERFACES_EXPORT KDevCatalog
{
 public:
  typedef QPair<QByteArray, QVariant> QueryArgument;

 public:
  KDevCatalog();
  virtual ~KDevCatalog();

  bool isValid() const;
  QString dbName() const;

  bool enabled() const;
  void setEnabled( bool en );

  virtual void open( const QString& dbName );
  virtual void close();
  virtual void sync();

  QList<QByteArray> indexList() const;
  void addIndex( const QByteArray& name );

  void addItem( Tag& tag );

  Tag getItemById( const QByteArray& id );
  QList<Tag> query( const QList<QueryArgument>& args );

  QByteArray generateId();

 private:
  class _KDevCatalog_Private* d;

 private:
  KDevCatalog( const KDevCatalog& source );
  void operator = ( const KDevCatalog& source );
};

#endif // KDEVCATALOG_H

