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

#include <qfile.h>
#include <qobjectlist.h>
#include <qwidget.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

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


static KLibFactory *doctreeviewFactory = 0;
static KLibFactory *classviewFactory = 0;
static KLibFactory *grepviewFactory = 0;
static KLibFactory *appwizardFactory = 0;
static KLibFactory *outputviewsFactory = 0;


static KLibFactory *factoryForService(KService *service)
{

    // Currently some factories are directly linked and hard-coded
    // for efficiency reasons
    // ### FIXME: when these hardcoded entries are removed, then install those
    // parts into kde_moduledir (using kde_module_LTLIBRARIES) ! That's where
    // they belong, but we can't put them there currently, as gideon links
    // against them right now and we'll get unresolved symbols on startup as the
    // dynamic linker does not look in $kde_moduledir (for a good reason :)
    // (Simon)

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

    return KLibLoader::self()->factory(QFile::encodeName(service->library()));
}



void PartLoader::cleanup()
{
    delete doctreeviewFactory;
    delete classviewFactory;
    delete grepviewFactory;
    delete appwizardFactory;
    delete outputviewsFactory;
}


KDevPart *PartLoader::loadService(KService *service, const char *className, KDevApi *api, QObject *parent)
{
    QStringList args;
    QVariant prop = service->property("X-KDevelop-Args");
    if (prop.isValid())
        args = QStringList::split(" ", prop.toString());

    kdDebug(9000) << "Loading service " << service->name() << endl;
    KLibFactory *factory = factoryForService(service);
    if (!factory || !factory->inherits("KDevFactory")) {
        if (!factory) {
            QString errorMessage = KLibLoader::self()->lastErrorMessage();
            KMessageBox::error(0, i18n("There was an error loading the module %1.\n"
                                       "The diagnostics is:\n%2").arg(service->name()).arg(errorMessage));
            return 0;
        }
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
