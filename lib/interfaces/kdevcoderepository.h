/* This file is part of the KDE project
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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef KDEVCODEREPOSITORY_H
#define KDEVCODEREPOSITORY_H

#include <qobject.h>
#include <q3valuelist.h>

/**
@file kdevcoderepository.h
Code repository - the persistant symbol store accessor.
*/

class KDevCodeRepositoryData;
class Catalog;

/**
Code repository - the persistant symbol store accessor.
Symbols from parsed files can be saved to the persistant symbol store.
Persistance in this case means that symbol database is never loaded into memory
and works like a usual database which executes queries.

Code repository consists from @ref Catalog objects that represent separate symbol 
databases. Catalogs can be created/loaded/unloaded dynamically.
To find a symbol in the repository each catalog should be queried.

Persistant symbol store is useful to keep information about code that
never or rarely changes. System libraries are perfect examples of such code.
Symbols from code contained in project files are better stored in memory
symbol store like @ref CodeModel.
*/
class KDevCodeRepository : public QObject
{
    Q_OBJECT
public:
    /**Constructor.*/
    KDevCodeRepository();
    /**Destructor.*/
    virtual ~KDevCodeRepository();
    
    /**@return The main catalog. Each catalog can be marked is main 
    to provide easy access to it.*/
    Catalog* mainCatalog();
    /**Sets the main catalog.
    @param mainCatalog The catalog to be marked as main.*/
    void setMainCatalog( Catalog* mainCatalog );

    /**@return The list of registered catalogs.*/
    Q3ValueList<Catalog*> registeredCatalogs();

    /**Registers catalog in the repository.
    @param catalog The catalog to register.*/
    void registerCatalog( Catalog* catalog );
    /**Unregisters catalog from the repository.
    @param catalog The catalog to unregister.*/
    void unregisterCatalog( Catalog* catalog );
    /**Marks catalog as changed and emits @ref catalogChanged signal.
    @param catalog The catalog to touch.*/
    void touchCatalog( Catalog* catalog );

signals:
    /**Emitted when a new catalog is registered.
    @param catalog The new catalog.*/
    void catalogRegistered( Catalog* catalog );

    /**Emitted when a catalog in removed
    @param catalog The catalog that was removed.*/
    void catalogUnregistered( Catalog* catalog );

    /**Emitted when the contens of catalog is changed.
    @param catalog Changed catalog.*/
    void catalogChanged( Catalog* catalog );

private:
    KDevCodeRepositoryData* d;
};

#endif
