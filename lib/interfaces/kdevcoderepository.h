//
//
// C++ Interface: $MODULE$
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
