/***************************************************************************
 *   Copyright 2000-2001 Bernd Gehrmann <bernd@kdevelop.org>        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "appwizardfactory.h"

#include <kstandarddirs.h>

K_EXPORT_COMPONENT_FACTORY(kdevappwizard, AppWizardFactory("kdevappwizard"))

AppWizardFactory::AppWizardFactory(const char *instanceName)
    : KGenericFactory<AppWizardPart>(instanceName)
{
}

KComponentData *AppWizardFactory::createComponentData()
{
    KComponentData *componentData = KGenericFactory<AppWizardPart>::createComponentData();
    KStandardDirs *dirs = componentData->dirs();
    dirs->addResourceType("apptemplates", "data", "kdevappwizard/templates/");
    dirs->addResourceType("apptemplate_descriptions","data", "kdevappwizard/template_descriptions/");
    dirs->addResourceType("appimports", "data", "kdevappwizard/imports/");
    dirs->addResourceType("appimportfiles", "data", "kdevappwizard/importfiles/");

    return componentData;
}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
