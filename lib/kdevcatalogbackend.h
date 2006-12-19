/* This file is part of KDevelop
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

#ifndef KDEVCATALOGBACKEND_H
#define KDEVCATALOGBACKEND_H

#include <QList>
#include <QPair>
#include <QVariant>
#include <QByteArray>

#include "tag.h"
#include "kdevexport.h"

/**
 * Abstract class to define the interface to be used when accessing the PCS
 * catalog via a database
 */
class KDEVPLATFORM_EXPORT KDevCatalogBackend
{
public:
    typedef QPair<QByteArray, QVariant> QueryArgument;

public:
    KDevCatalogBackend() {};
    virtual ~KDevCatalogBackend() {};

    //! Open the catalog backend
    virtual void open( const QString& dbName ) = 0;

    //! Close the catalog backend
    virtual void close() = 0;

    //! Commit any unsaved changes to the backend
    virtual void sync() = 0;

    //! Check if the backend is open
    virtual bool isOpen() = 0;

    //! Add an item to the backend
    virtual void addItem( Tag& ) = 0;

    //! Get an item from the backend using the id for the id
    virtual Tag getItemById( int id ) = 0;

    //! Query the catalog for a list of items
    virtual QList<Tag> query( const QList<QueryArgument>& args ) = 0;

    //! Get a list of the indexes
    virtual QList<QByteArray> indexList() const = 0;

    //! Add an index to the backend
    virtual void addIndex( const QByteArray& name ) = 0;

};

#endif

//kate: indent-spaces on; indent-width 4; replace-tabs on;
