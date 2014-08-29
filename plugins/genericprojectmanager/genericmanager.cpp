/* This file is part of KDevelop
    CopyRight 2010 Milian Wolff <mail@milianw.de>
    Copyright 2004 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "genericmanager.h"

#include <KPluginLoader>
#include <KPluginFactory>
#include <KAboutData>
#include <KConfigGroup>

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(GenericSupportFactory, "kdevgenericmanager.json", registerPlugin<GenericProjectManager>();)
// K_EXPORT_PLUGIN(GenericSupportFactory(
//     KAboutData("kdevgenericmanager", "kdevgenericprojectmanager", ki18n("Generic Project Manager"),
//                "0.2", ki18n("A plugin to support basic project management on a filesystem level"), KAboutData::License_GPL)))

GenericProjectManager::GenericProjectManager( QObject* parent, const QVariantList& args )
    : AbstractFileManagerPlugin( "kdevgenericmanager", parent, args )
{
}

#include "genericmanager.moc"
