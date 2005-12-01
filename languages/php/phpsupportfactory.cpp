/*
 * KDevelop PHP Language Support
 *
 * Copyright (c) 2005 Escuder Nicolas <n.escuder@intra-links.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <kinstance.h>
#include <kstandarddirs.h>
#include <kdevplugininfo.h>

#include "phpsupportfactory.h"

K_EXPORT_COMPONENT_FACTORY(libkdevphpsupport, PHPSupportFactory)

static const KDevPluginInfo data("kdevphpsupport");

PHPSupportFactory::PHPSupportFactory()
	: KDevGenericFactory<PHPLanguageSupport>(data)
{
}

KInstance *PHPSupportFactory::createInstance()
{
	KInstance *instance = KDevGenericFactory<PHPLanguageSupport>::createInstance();
	KStandardDirs *dirs = instance->dirs();
	dirs->addResourceType("newclasstemplates", KStandardDirs::kde_default("data") + "kdevphpsupport/newclass/");
	dirs->addResourceType("pcs", KStandardDirs::kde_default("data") + "kdevphpsupport/pcs/");

	return instance;
}

const KDevPluginInfo * PHPSupportFactory::info()
{
	return &data;
}
//kate: indent-mode csands; tab-width 4; space-indent off;
