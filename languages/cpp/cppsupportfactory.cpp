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
#include "cppsupportfactory.h"

K_EXPORT_COMPONENT_FACTORY( libkdevcppsupport, CppSupportFactory )

static const KDevPluginInfo data("kdevcppsupport");

CppSupportFactory::CppSupportFactory()
: KDevGenericFactory<CppSupportPart>( data )
{
}

KInstance *CppSupportFactory::createInstance()
{
	KInstance *instance = KDevGenericFactory<CppSupportPart>::createInstance();
	KStandardDirs *dirs = instance->dirs();
	dirs->addResourceType( "newclasstemplates", KStandardDirs::kde_default("data") + "kdevcppsupport/newclass/" );
	dirs->addResourceType( "pcs", KStandardDirs::kde_default( "data" ) + "kdevcppsupport/pcs/" );
	
	return instance;
}

const KDevPluginInfo * CppSupportFactory::info()
{
	return &data;
}
//kate: indent-mode csands; tab-width 4; space-indent off;


