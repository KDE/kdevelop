/***************************************************************************
 *   Copyright (C) 2000-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <kdebug.h>
#include <kinstance.h>
#include <kstandarddirs.h>

#include "appwizardfactory.h"
#include "appwizardpart.h"


extern "C" {

    void *init_libkdevappwizard()
    {
        return new AppWizardFactory;
    }
    
};


AppWizardFactory::AppWizardFactory(QObject *parent, const char *name)
    : KDevFactory(parent, name)
{
}


AppWizardFactory::~AppWizardFactory()
{
    delete s_instance;
    s_instance = 0;
}


KDevPart *AppWizardFactory::createPartObject(KDevApi *api, QObject *parent,
                                             const QStringList &/*args*/)
{
    kdDebug(9010) << "Building AppWizardPart" << endl;
    return new AppWizardPart(api, parent, "app wizard part");
}


KInstance *AppWizardFactory::s_instance = 0;
KInstance *AppWizardFactory::instance()
{
    if (!s_instance) {
        s_instance = new KInstance("kdevappwizard");
        KStandardDirs *dirs = s_instance->dirs();
        dirs->addResourceType("apptemplates", KStandardDirs::kde_default("data") + "kdevappwizard/templates/");
        dirs->addResourceType("appimports", KStandardDirs::kde_default("data") + "kdevappwizard/imports/");
        dirs->addResourceType("appimportfiles", KStandardDirs::kde_default("data") + "kdevappwizard/importfiles/");
    }

    return s_instance;
}

#include "appwizardfactory.moc"
