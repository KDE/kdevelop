/* This file is part of KDevelop
   Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2006 Matt Rogers <mattr@kde.org>

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

#ifndef KDEVBDBCATALOGBACKEND_H
#define KDEVBDBCATALOGBACKEND_H

#include "kdevcatalogbackend.h"

#include <QMap>
#include <QList>
#include <QPair>
#include <QByteArray>

#include "tag.h"
#include "kdevexport.h"

#include <db.h>

/**
 * Catalog backend using the bdb
 */
class KDEVPLATFORM_EXPORT KDevBDBCatalogBackend : public KDevCatalogBackend
{
public:
    KDevBDBCatalogBackend();
    virtual ~KDevBDBCatalogBackend();

    //! Open the catalog backend
    virtual void open( const QString& dbName );

    //! Close the catalog backend
    virtual void close();

    //! Commit any unsaved changes to the backend
    virtual void sync();

    //! Check if the backend is open
    virtual bool isOpen();

    //! Add an item to the backend
    virtual void addItem( Tag& );

    //! Get an item from the backend using the id for the id
    virtual Tag getItemById( int id );

    //! Query the catalog for a list of items
    virtual QList<Tag> query( const QList<QueryArgument>& args );

    //! Get a list of the indexes
    virtual QList<QByteArray> indexList() const;

    //! Add an index to the backend
    virtual void addIndex( const QByteArray& name );

private:
    bool hasIndex( const QByteArray& name ) const;
    DB* index( const QByteArray& name );
    bool addItem( DB* dbp, const QByteArray& id, const Tag& tag );
    bool addItem( DB* dbp, const QVariant& id, const QByteArray& v );
    QByteArray generateId();

private:
    DB* m_db;
    QMap<QByteArray, DB*> m_indexList;
};

#endif
