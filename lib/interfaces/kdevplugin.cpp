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

#include "kdevapi.h"
#include "kdevcore.h"
#include "kdevplugininfo.h"
#include "kdevdocumentcontroller.h"
#include "kdevplugincontroller.h"

#include <kaction.h>
#include <kglobal.h>
#include <kiconloader.h>

#include <qdom.h>
#include <qmap.h>

#include <assert.h>

///////////////////////////////////////////////////////////////////////////////
// struct KDevPlugin::Private
///////////////////////////////////////////////////////////////////////////////

struct KDevPlugin::Private
{
    const KDevPluginInfo *info;
};

///////////////////////////////////////////////////////////////////////////////
// class KDevPlugin
///////////////////////////////////////////////////////////////////////////////

KDevPlugin::KDevPlugin(const KDevPluginInfo *info, QObject *parent)
    :QObject(parent), d(new Private)
{
    assert(parent->inherits( "KDevApi" ));
    m_api = static_cast<KDevApi *>( parent );

    d->info = info;
    KGlobal::iconLoader()->addAppDir("kdevelop");
}

KDevPlugin::~KDevPlugin()
{
   delete( d );
}

KDevMainWindow *KDevPlugin::mainWindow()
{
    return m_api->mainWindow();
}

KDevCore *KDevPlugin::core() const
{
    return m_api->core();
}

KDevProject *KDevPlugin::project() const
{
    return m_api->project();
}

QDomDocument *KDevPlugin::projectDom() const
{
    return m_api->projectDom();
}

KDevLanguageSupport *KDevPlugin::languageSupport() const
{
    return m_api->languageSupport();
}

KDevDocumentController *KDevPlugin::documentController() const
{
    return m_api->documentController();
}

KDevPluginController *KDevPlugin::pluginController() const
{
    return m_api->pluginController();
}

void KDevPlugin::restorePartialProjectSession(const QDomElement* /*el*/)
{
    // there's still nothing to do in the base class
}

void KDevPlugin::savePartialProjectSession(QDomElement* /*el*/)
{
    // there's still nothing to do in the base class
}

KDevPlugin * KDevPlugin::extension_internal(const QString &serviceType, const QString &constraint)
{
    return m_api->pluginController()->extension(serviceType, constraint);
}

const KDevPluginInfo *KDevPlugin::info()
{
    return d->info;
}

#include "kdevplugin.moc"
