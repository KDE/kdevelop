/* This file is part of the KDE project
 Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
 Copyright (C) 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 Copyright (C) 2002 Harald Fernengel <harry@kdevelop.org>
 Copyright (C) 2002 F@lk Brettschneider <falkbr@kdevelop.org>
 Copyright (C) 2003 Julian Rockey <linux@jrockey.com>
 Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>
 Copyright (C) 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
 Copyright (C) 2003 Mario Scalas <mario.scalas@libero.it>
 Copyright (C) 2003-2004 Alexander Dymo <adymo@kdevelop.org>
 Copyright (C) 2006 Adam Treat <treat@kde.org>

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
#include "kdevplugin.h"

#include <kaction.h>
#include <kglobal.h>
#include <kinstance.h>
#include <kiconloader.h>

#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevplugincontroller.h"

KDevPlugin::KDevPlugin( KInstance *instance, QObject *parent )
        : QObject( parent ),
        KXMLGUIClient()
{
    setInstance( instance );
    KGlobal::iconLoader() ->addAppDir( "kdevelop" );
}

KDevPlugin::~KDevPlugin()
{
}

QWidget *KDevPlugin::pluginView() const
{
    return 0;
}

Qt::DockWidgetArea KDevPlugin::dockWidgetAreaHint() const
{
    return Qt::NoDockWidgetArea;
}

bool KDevPlugin::isCentralPlugin() const
{
    return false;
}

KDevPlugin * KDevPlugin::extension_internal( const QString &serviceType, const QString &constraint )
{
    return KDevCore::pluginController() ->extension( serviceType, constraint );
}

#include "kdevplugin.moc"
