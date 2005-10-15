/* KDevelop C++ Language Support
 *
 * Copyright (C)  2005  Matt Rogers <mattr@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <kinstance.h>
#include <kstandarddirs.h>

#include <kdevplugininfo.h>
#include "cppsupportfactory.h"

K_EXPORT_COMPONENT_FACTORY(libkdevcppsupport, CppLanguageSupportFactory)

static const KDevPluginInfo data("kdevcppsupport");

CppLanguageSupportFactory::CppLanguageSupportFactory()
    : KDevGenericFactory<CppLanguageSupport>(data)
{
}

KInstance* CppLanguageSupportFactory::createInstance()
{
    KInstance* instance = KDevGenericFactory<CppLanguageSupport>::createInstance();
    KStandardDirs *dirs = instance->dirs();
    dirs->addResourceType("newclasstemplates",
                          KStandardDirs::kde_default("data") +
                          "kdevelop/cpp/newclasses");
    dirs->addResourceType("pcs",
                          KStandardDirs::kde_default("data") +
                          "kdevelop/cpp/pcs");

    return instance;
}

const KDevPluginInfo* CppLanguageSupportFactory::info()
{
    return &data;
}

//kate: space-indent on; indent-width 4;
