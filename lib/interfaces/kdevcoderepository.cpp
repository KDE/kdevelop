//
//
// C++ Implementation: $MODULE$
//
// Description:
//
//
// Author: Roberto Raggi <roberto@kdevelop.org>, (C) 2003
//         KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "kdevcoderepository.h"

struct KDevCodeRepositoryData
{
    QValueList<Catalog*> catalogs;
};

KDevCodeRepository::KDevCodeRepository( )
    : d( new KDevCodeRepositoryData )
{
}

KDevCodeRepository::~KDevCodeRepository( )
{
    delete( d );
}

void KDevCodeRepository::registerCatalog( Catalog * catalog )
{
    d->catalogs.append( catalog );
    emit catalogRegistered( catalog );
}

void KDevCodeRepository::unregisterCatalog( Catalog * catalog )
{
    d->catalogs.remove( catalog );
    emit catalogUnregistered( catalog );
}

void KDevCodeRepository::touchCatalog( Catalog * catalog )
{
    emit catalogChanged( catalog );
}

QValueList< Catalog * > KDevCodeRepository::registeredCatalogs( )
{
    return d->catalogs;
}

#include "kdevcoderepository.moc"
