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
#include <qvaluelist.h>

class KDevCodeRepositoryData;
class Catalog;

/**
@author KDevelop Authors
*/
class KDevCodeRepository : public QObject
{
    Q_OBJECT
public:
    KDevCodeRepository();
    virtual ~KDevCodeRepository();
    
    Catalog* mainCatalog();
    void setMainCatalog( Catalog* mainCatalog );

    QValueList<Catalog*> registeredCatalogs();

    void registerCatalog( Catalog* catalog );
    void unregisterCatalog( Catalog* catalog );
    void touchCatalog( Catalog* catalog );

signals:
    /**
     * Emitted when a new catalog is registered
     */
    void catalogRegistered( Catalog* catalog );

    /**
     * Emitted when a catalog in removed
     */
    void catalogUnregistered( Catalog* catalog );

    /**
     * Emitted when the contens of catalog is changed
     */
    void catalogChanged( Catalog* catalog );

private:
    KDevCodeRepositoryData* d;
};

#endif
