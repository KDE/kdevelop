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


#include <kstandarddirs.h>
#include <kdevplugininfo.h>

#include "appwizardfactory.h"

static const KDevPluginInfo data("kdevappwizard");
K_EXPORT_COMPONENT_FACTORY( libkdevappwizard, AppWizardFactory )

AppWizardFactory::AppWizardFactory()
    : KDevGenericFactory<AppWizardPart>( data )
{
}

KInstance *AppWizardFactory::createInstance()
{
    KInstance *instance = KDevGenericFactory<AppWizardPart>::createInstance();
    KStandardDirs *dirs = instance->dirs();
    dirs->addResourceType("apptemplates", KStandardDirs::kde_default("data") + "kdevappwizard/templates/");
    dirs->addResourceType("appimports", KStandardDirs::kde_default("data") + "kdevappwizard/imports/");
    dirs->addResourceType("appimportfiles", KStandardDirs::kde_default("data") + "kdevappwizard/importfiles/");

    return instance;
}

const KDevPluginInfo *AppWizardFactory::info()
{
    return &data;
}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
