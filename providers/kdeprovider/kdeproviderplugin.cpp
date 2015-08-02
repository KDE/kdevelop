/*
 * This file is part of KDevelop
 * Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>
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

#include "kdeproviderplugin.h"
#include <QVariantList>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kaboutdata.h>
#include <KLocalizedString>
#include "kdeproviderwidget.h"

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(KDevKDEProviderFactory, "kdevkdeprovider.json", registerPlugin<KDEProviderPlugin>(); )

KDEProviderPlugin::KDEProviderPlugin ( QObject* parent, const QVariantList& ) 
    : IPlugin ( "kdevkdeprovider", parent )
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectProvider )
}

KDEProviderPlugin::~KDEProviderPlugin()
{}

QString KDEProviderPlugin::name() const
{
    return i18n("KDE");
}

KDevelop::IProjectProviderWidget* KDEProviderPlugin::providerWidget(QWidget* parent)
{
    return new KDEProviderWidget(parent);
}

#include "kdeproviderplugin.moc"
