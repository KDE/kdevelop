/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qobjectlist.h>
#include <qwidget.h>
#include <kdebug.h>


#include <kconfig.h>
#include <kapp.h>


#include "kdevfactory.h"
#include "kdevpart.h"
#include "partloader.h"


extern "C" {
    void *init_libkdevdoctreeview();
    void *init_libkdevclassview();
    void *init_libkdevgrepview();
    void *init_libkdevappwizard();
    void *init_libkdevoutputviews();
};


static KLibFactory *factoryForService(KService *service)
{
    static KLibFactory *doctreeviewFactory = 0;
    static KLibFactory *classviewFactory = 0;
    static KLibFactory *grepviewFactory = 0;
    static KLibFactory *appwizardFactory = 0;
    static KLibFactory *outputviewsFactory = 0;
    
    // Currently some factories are directly linked and hard-coded
    // for efficiency reasons
    
    if (service->name() == "KDevDocTreeView") {
        if (!doctreeviewFactory)
            doctreeviewFactory = static_cast<KLibFactory*>(init_libkdevdoctreeview());
        return doctreeviewFactory;
    }
    else if (service->name() == "KDevClassView") {
        if (!classviewFactory)
            classviewFactory = static_cast<KLibFactory*>(init_libkdevclassview());
        return classviewFactory;
    }
    else if (service->name() == "KDevGrepView") {
        if (!grepviewFactory)
            grepviewFactory = static_cast<KLibFactory*>(init_libkdevgrepview());
        return grepviewFactory;
    }
    else if (service->name() == "KDevAppWizard") {
        if (!appwizardFactory)
            appwizardFactory = static_cast<KLibFactory*>(init_libkdevappwizard());
        return appwizardFactory;
    }
    else if (service->name() == "KDevMakeView" || service->name() == "KDevAppOutputView") {
        if (!outputviewsFactory)
            outputviewsFactory = static_cast<KLibFactory*>(init_libkdevoutputviews());
        return outputviewsFactory;
    }
    
    return KLibLoader::self()->factory(service->library());
}



KDevPart *PartLoader::loadByName(const QString &name, const char *className,
                                 KDevApi *api, QObject *parent)
{
    KService::Ptr service = KService::serviceByName(name);
    if (!service)
        return 0;
    
    QStringList args;
    QVariant prop = service->property("X-KDevelop-Args");
    if (prop.isValid())
        args = QStringList::split(" ", prop.toString());

    kdDebug(9000) << "Loading service " << service->name() << endl;
    KLibFactory *factory = factoryForService(service);
    if (!factory->inherits("KDevFactory")) {
        kdDebug(9000) << "Does not have a KDevFactory" << endl;
        return 0;
    }  
    
    KDevPart *part = static_cast<KDevFactory*>(factory)->createPart(api, parent, args);
    
    if (!part->inherits(className)) {
        kdDebug(9000) << "Part does not inherit " << className << endl;
        return 0;
    }
    
    return part;
}


KDevPart *PartLoader::loadByQuery(const QString &serviceType, const QString &constraint, const char *className,
                                  KDevApi *api, QObject *parent)
{
    KTrader::OfferList offers = KTrader::self()->query(serviceType, constraint);
    if (offers.isEmpty())
        return 0;

    KService *service = *offers.begin();

    QStringList args;
    QVariant prop = service->property("X-KDevelop-Args");
    if (prop.isValid())
        args = QStringList::split(" ", prop.toString());

    kdDebug(9000) << "Loading service " << service->name() << endl;
    KLibFactory *factory = factoryForService(service);
    if (!factory->inherits("KDevFactory")) {
        kdDebug(9000) << "Does not have a KDevFactory" << endl;
        return 0;
    }  
    
    KDevPart *part = static_cast<KDevFactory*>(factory)->createPart(api, parent, args);

    if (!part->inherits(className)) {
        kdDebug(9000) << "Part does not inherit " << className << endl;
        return 0;
    }

    return part;
}


QList<KDevPart> PartLoader::loadAllByQuery(const QString &serviceType, const QString &constraint, const char *className,
                                           KDevApi *api, QObject *parent, bool filter)
{
    KConfig *config = kapp->config();
    config->setGroup("Plugins");
  
    QList<KDevPart> list;
    
    KTrader::OfferList offers = KTrader::self()->query(serviceType, constraint);
    if (offers.isEmpty())
        return QList<KDevPart>();

    for (KTrader::OfferList::ConstIterator it = offers.begin(); it != offers.end(); ++it) {
        KService *service = *it;

        if (filter && !config->readBoolEntry(service->name(), true))
          continue;

        QStringList args;
        QVariant prop = service->property("X-KDevelop-Args");
        if (prop.isValid())
            args = QStringList::split(" ", prop.toString());

        kdDebug(9000) << "Loading service " << service->name() << endl;
        KLibFactory *factory = factoryForService(service);
        if (!factory->inherits("KDevFactory")) {
            kdDebug(9000) << "Does not have a KDevFactory" << endl;
            continue;
        }  
        
        KDevPart *part = static_cast<KDevFactory*>(factory)->createPart(api, parent, args);
        
        if (!part->inherits(className)) {
            kdDebug(9000) << "Component does not inherit " << className << endl;
            continue;
        }
        
        list.append(part);
    }
    
    return list;
}

