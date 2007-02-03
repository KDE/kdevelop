/* This file is part of the KDE project
 Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
 Copyright (C) 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 Copyright (C) 2002 Harald Fernengel <harry@kdevelop.org>
 Copyright (C) 2002 F@lk Brettschneider <falkbr@kdevelop.org>
 Copyright (C) 2003 Julian Rockey <linux@jrockey.com>
 Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>
 Copyright (C) 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
 Copyright (C) 2003 Mario Scalas <mario.scalas@libero.it>
 Copyright (C) 2003-2004,2007 Alexander Dymo <adymo@kdevelop.org>
 Copyright (C) 2006 Adam Treat <treat@kde.org>
 Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>

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
#include <kaction.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kcomponentdata.h>
#include <kiconloader.h>
#include <kdebug.h>
#include "iplugin.h"
#include "icore.h"
#include "iplugincontroller.h"
// #include "kdevmainwindow.h"
// #include "kdevIPlugincontroller.h"

namespace Koncrete
{

class IPlugin::Private
{
public:
    Private()
        : iconLoader(0)
    {}

    ~Private()
    {
        delete iconLoader;
    }

    ICore *core;
    KIconLoader* iconLoader;
};

IPlugin::IPlugin( const KComponentData &instance, QObject *parent )
        : QObject( parent ),
        KXMLGUIClient()
{
    d = new Private;
    d->core = static_cast<Koncrete::ICore*>(parent);
    setComponentData( instance );
}

IPlugin::~IPlugin()
{
}

Qt::DockWidgetArea IPlugin::dockWidgetAreaHint() const
{
    return Qt::NoDockWidgetArea;
}

bool IPlugin::isCentralPlugin() const
{
    return false;
}

void IPlugin::prepareForUnload()
{
    emit readyToUnload( this );
}

KIconLoader *IPlugin::iconLoader() const
{
    if ( d->iconLoader == 0 ) {
        d->iconLoader = new KIconLoader( componentData().componentName(), componentData().dirs() );
        d->iconLoader->addAppDir( "kdevelop" );
        connect(KGlobalSettings::self(), SIGNAL(iconChanged(int)),
                this, SLOT(newIconLoader()));
    }

    return d->iconLoader;
}

void IPlugin::newIconLoader() const
{
    if (d->iconLoader) {
        d->iconLoader->reconfigure( componentData().componentName(), componentData().dirs() );
    }
}

ICore *IPlugin::core() const
{
    return d->core;
}

QExtensionManager* IPlugin::extensionManager()
{
    return core()->pluginController()->extensionManager();
}

}

#include "iplugin.moc"

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
