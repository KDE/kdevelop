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

#include <kinstance.h>
#include <kstandarddirs.h>
#include <kdevplugininfo.h>
#include "javasupportfactory.h"

static const KDevPluginInfo data("kdevjavasupport");
K_EXPORT_COMPONENT_FACTORY( libkdevjavasupport, JavaSupportFactory )

JavaSupportFactory::JavaSupportFactory()
    : KDevGenericFactory<JavaSupportPart>( data )
{
}

KInstance *JavaSupportFactory::createInstance()
{
    KInstance *instance = KDevGenericFactory<JavaSupportPart>::createInstance();
    KStandardDirs *dirs = instance->dirs();
    dirs->addResourceType( "newclasstemplates", KStandardDirs::kde_default("data") + "kdevjavasupport/newclass/" );
    dirs->addResourceType( "pcs", KStandardDirs::kde_default( "data" ) + "kdevjavasupport/pcs/" );

    return instance;
}

const KDevPluginInfo *JavaSupportFactory::info()
{
    return &data;
}

