/* This file is part of the KDevelop project
   Copyright (C) 2006 Matt Rogers <mattr@kde.org>

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

#include "kdevreadwritepart.h"

#include <cassert>
#include <kglobal.h>
#include <kiconloader.h>
#include "kdevapi.h"
#include "kdevcore.h"
#include "kdevdocumentcontroller.h"
#include "kdevplugincontroller.h"
#include "kdevplugininfo.h"

struct KDevReadWritePart::Private
{
public:
    KDevApi* api;
};

KDevReadWritePart::KDevReadWritePart( QObject* parent )
    : KParts::ReadWritePart( parent ), d( new Private )
{
    d->api = 0;
    KGlobal::iconLoader()->addAppDir("kdevelop");
}

KDevReadWritePart::~KDevReadWritePart()
{
    delete d;
}

void KDevReadWritePart::setApiInstance( KDevApi* api )
{
    d->api = api;
}

KDevMainWindow *KDevReadWritePart::mainWindow()
{
    return d->api->mainWindow();
}

KDevCore *KDevReadWritePart::core() const
{
    return d->api->core();
}

KDevProject *KDevReadWritePart::project() const
{
    return d->api->project();
}

QDomDocument *KDevReadWritePart::projectDom() const
{
    return d->api->projectDom();
}

KDevLanguageSupport *KDevReadWritePart::languageSupport() const
{
    return d->api->languageSupport();
}

KDevDocumentController *KDevReadWritePart::documentController() const
{
    return d->api->documentController();
}

KDevPluginController *KDevReadWritePart::pluginController() const
{
    return d->api->pluginController();
}

//kate: indent-mode cstyle; indent-width 4; space-indent on; replace-tabs on;
