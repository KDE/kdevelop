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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "kdevcoderepository.h"
#include <QList>

namespace KDevelop
{
    
struct CodeRepositoryData
{
    Catalog* mainCatalog;
    QList<Catalog*> catalogs;
    
    CodeRepositoryData(): mainCatalog( 0 ) {}
};

CodeRepository::CodeRepository( )
    : d( new CodeRepositoryData )
{
}

CodeRepository::~CodeRepository( )
{
    delete( d );
}

Catalog* CodeRepository::mainCatalog()
{
    return d->mainCatalog;
}

void CodeRepository::setMainCatalog( Catalog * mainCatalog )
{
    d->mainCatalog = mainCatalog;
}

void CodeRepository::registerCatalog( Catalog * catalog )
{
    d->catalogs.append( catalog );
    emit catalogRegistered( catalog );
}

void CodeRepository::unregisterCatalog( Catalog * catalog )
{
    d->catalogs.removeAll( catalog );
    emit catalogUnregistered( catalog );
}

void CodeRepository::touchCatalog( Catalog * catalog )
{
    emit catalogChanged( catalog );
}

QList< Catalog * > CodeRepository::registeredCatalogs( )
{
    return d->catalogs;
}

}
#include "kdevcoderepository.moc"
