/* This file is part of the KDE project
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002 Harald Fernengel <harry@kdevelop.org>
   Copyright (C) 2002 F@lk Brettschneider <falkbr@kdevelop.org>
   Copyright (C) 2003 Julian Rockey <linux@jrockey.com>
   Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
   Copyright (C) 2003 Mario Scalas <mario.scalas@libero.it>
   Copyright (C) 2003 Alexander Dymo <cloudtemple@mksat.net>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kdevplugin.h"
#include "kdevpartcontroller.h"
#include "kdevapi.h"
#include "kdevcore.h"
#include "kdevversioncontrol.h"

#include <kaction.h>

#include <dcopclient.h>
#include <qdom.h>
#include <qmap.h>

#include <assert.h>

///////////////////////////////////////////////////////////////////////////////
// struct KDevPlugin::Private
///////////////////////////////////////////////////////////////////////////////

struct KDevPlugin::Private
{
    DCOPClient *dcopClient;
    QCString name;
    QString pluginName;
    QString icon;
};

///////////////////////////////////////////////////////////////////////////////
// class KDevPlugin
///////////////////////////////////////////////////////////////////////////////

KDevPlugin::KDevPlugin( const QString& pluginName, const QString& icon, QObject *parent, const char *name)
    : QObject( parent, name ), d( new Private )
{
    assert( parent->inherits( "KDevApi" ) );
    m_api = static_cast<KDevApi *>( parent );

    actionCollection()->setHighlightingEnabled( true );

    d->name = name;
    d->icon = icon;
    d->pluginName = pluginName;
    d->dcopClient = 0L;
}

///////////////////////////////////////////////////////////////////////////////

KDevPlugin::~KDevPlugin()
{
   delete( d->dcopClient );
   delete( d );
}

///////////////////////////////////////////////////////////////////////////////

QString KDevPlugin::pluginName() const
{
    return d->pluginName;
}

///////////////////////////////////////////////////////////////////////////////

QString KDevPlugin::icon() const
{
    return d->icon;
}

///////////////////////////////////////////////////////////////////////////////

QString KDevPlugin::shortDescription() const
{
    return QString::null;
}

///////////////////////////////////////////////////////////////////////////////

QString KDevPlugin::description() const
{
    return QString::null;
}

///////////////////////////////////////////////////////////////////////////////

DCOPClient* KDevPlugin::dcopClient() const
{
    if (!d->dcopClient)
    {
        d->dcopClient = new DCOPClient();
        d->dcopClient->registerAs(d->name, false);
    }

    return d->dcopClient;
}

///////////////////////////////////////////////////////////////////////////////

KDevMainWindow *KDevPlugin::mainWindow()
{
    return m_api->mainWindow();
}

///////////////////////////////////////////////////////////////////////////////

bool KDevPlugin::mainWindowValid()
{
    return m_api->mainWindowValid();
}

///////////////////////////////////////////////////////////////////////////////

KDevCore *KDevPlugin::core() const
{
    return m_api->core();
}

///////////////////////////////////////////////////////////////////////////////

KDevProject *KDevPlugin::project() const
{
    return m_api->project();
}

///////////////////////////////////////////////////////////////////////////////

CodeModel *KDevPlugin::codeModel() const
{
    return m_api->codeModel();
}

///////////////////////////////////////////////////////////////////////////////

QDomDocument *KDevPlugin::projectDom() const
{
    return m_api->projectDom();
}

///////////////////////////////////////////////////////////////////////////////

KDevLanguageSupport *KDevPlugin::languageSupport() const
{
    return m_api->languageSupport();
}

///////////////////////////////////////////////////////////////////////////////

KDevMakeFrontend *KDevPlugin::makeFrontend() const
{
    return m_api->makeFrontend();
}

///////////////////////////////////////////////////////////////////////////////

KDevDiffFrontend *KDevPlugin::diffFrontend() const
{
    return m_api->diffFrontend();
}

///////////////////////////////////////////////////////////////////////////////

KDevAppFrontend *KDevPlugin::appFrontend() const
{
    return m_api->appFrontend();
}

///////////////////////////////////////////////////////////////////////////////

KDevPartController *KDevPlugin::partController() const
{
    return m_api->partController();
}

///////////////////////////////////////////////////////////////////////////////

KDevDebugger *KDevPlugin::debugger() const
{
    return m_api->debugger();
}

///////////////////////////////////////////////////////////////////////////////

KDevCreateFile *KDevPlugin::createFileSupport() const
{
    return m_api->createFile();
}

///////////////////////////////////////////////////////////////////////////////

KDevVersionControl *KDevPlugin::versionControl() const
{
    return m_api->versionControl();
}

///////////////////////////////////////////////////////////////////////////////

void KDevPlugin::restorePartialProjectSession(const QDomElement* /*el*/)
{
    // there's still nothing to do in the base class
}

///////////////////////////////////////////////////////////////////////////////

void KDevPlugin::savePartialProjectSession(QDomElement* /*el*/)
{
    // there's still nothing to do in the base class
}

///////////////////////////////////////////////////////////////////////////////

void KDevPlugin::showPart()
{
    if( part() )
        partController()->showPart( part(), d->pluginName, shortDescription() );
}

///////////////////////////////////////////////////////////////////////////////

KDevSourceFormatter * KDevPlugin::sourceFormatter( ) const
{
    return m_api->sourceFormatter();
}

///////////////////////////////////////////////////////////////////////////////

KDevCodeRepository * KDevPlugin::codeRepository( ) const
{
    return m_api->codeRepository();
}

#include "kdevplugin.moc"
