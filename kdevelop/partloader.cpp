/***************************************************************************
 *   Copyright (C) 2000 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "partloader.h"
#include <qobjectlist.h>
#include <qwidget.h>
#include <kdebug.h>


QObject *PartLoader::loadByName(QWidget *parent, const QString &name,
                                const char *className)
{
    KService::Ptr service = KService::serviceByName(name);
    if (!service)
        return 0;
    
    kdDebug(9000) << "Loading component " << service->name() << endl;
    KLibFactory *factory = KLibLoader::self()->factory(service->library());
    
    QStringList args;
    QVariant prop = service->property("X-KDevelop-Args");
    if (prop.isValid())
        args = QStringList::split(" ", prop.toString());
    
    QObject *obj = factory->create(parent, service->name().latin1(),
                                   className, args);
    
    if (!obj->inherits(className)) {
        kdDebug(9000) << "Component does not inherit " << className << endl;
        return 0;
    }
    
    return obj;
}


QObject *PartLoader::loadByQuery(QWidget *parent, const QString &serviceType, const QString &constraint,
                                 const char *className)
{
    KTrader::OfferList offers = KTrader::self()->query(serviceType, constraint);
    if (offers.isEmpty())
        return 0;

    KService *service = *offers.begin();
    kdDebug(9000) << "Loading component " << service->name() << endl;

    KLibFactory *factory = KLibLoader::self()->factory(service->library());

    QStringList args;
    QVariant prop = service->property("X-KDevelop-Args");
    if (prop.isValid())
        args = QStringList::split(" ", prop.toString());

    QObject *obj = factory->create(parent, service->name().latin1(),
                                   className, args);

    if (!obj->inherits(className)) {
        kdDebug(9000) << "Component does not inherit " << className << endl;
        return 0;
    }

    return obj;
}


QObjectList PartLoader::loadAllByQuery(QWidget *parent, const QString &serviceType, const QString &constraint,
                                       const char *className)
{
    QObjectList list;
    
    KTrader::OfferList offers = KTrader::self()->query(serviceType, constraint);
    if (offers.isEmpty())
        return QObjectList();

    for (KTrader::OfferList::ConstIterator it = offers.begin(); it != offers.end(); ++it) {
        KService *service = *it;
        kdDebug(9000) << "Loading component " << service->name() << endl;

        KLibFactory *factory = KLibLoader::self()->factory(service->library());
        
        QStringList args;
        QVariant prop = service->property("X-KDevelop-Args");
        if (prop.isValid())
            args = QStringList::split(" ", prop.toString());

        QObject *obj = factory->create(parent, service->name().latin1(),
                                       className, args);

        if (!obj->inherits(className)) {
            kdDebug(9000) << "Component does not inherit " << className << endl;
            return QObjectList();
        }

        list.append(obj);
    }

    return list;
}

