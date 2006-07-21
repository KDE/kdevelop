/* This file is part of the KDE project
   Copyright (C) 2000-2001 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (C) 2004 Alexander Dymo <adymo@kdevelop.org>

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

#include "kdevapi.h"

#include <kdevenv.h>

///////////////////////////////////////////////////////////////////////////////
// class KDevApiPrivate
///////////////////////////////////////////////////////////////////////////////

class KDevApiPrivate
{
public:
    KDevApiPrivate()
        : mainWindow(0)
        , documentController(0)
        , pluginController(0)
        , core(0)
        , environment(0)
        , projectDom(0)
        , project(0)
        , languageSupport(0)
    {}

    KDevMainWindow *mainWindow;
    KDevDocumentController *documentController;
    KDevPluginController *pluginController;
    KDevCore *core;
    KDevEnv *environment;
    QDomDocument *projectDom;
    KDevProject  *project;
    KDevLanguageSupport *languageSupport;
};

///////////////////////////////////////////////////////////////////////////////
// class KDevApi
///////////////////////////////////////////////////////////////////////////////

KDevApi* KDevApi::s_self = 0;

KDevApi* KDevApi::self()
{
    if ( s_self == 0 )
        s_self = new KDevApi();

    return s_self;
}

KDevApi::KDevApi()
    : d(new KDevApiPrivate())
{
}

KDevApi::~KDevApi()
{
    delete d;
}

KDevProject *KDevApi::project() const
{
  return d->project;
}

void KDevApi::setProject(KDevProject *project)
{
  d->project = project;
}

KDevLanguageSupport *KDevApi::languageSupport() const
{
  return d->languageSupport;
}

void KDevApi::setLanguageSupport(KDevLanguageSupport *languageSupport)
{
  d->languageSupport = languageSupport;
}

void KDevApi::setMainWindow( KDevMainWindow * mainWindow )
{
    d->mainWindow = mainWindow;
}

KDevMainWindow * KDevApi::mainWindow( ) const
{
    return d->mainWindow;
}

void KDevApi::setDocumentController( KDevDocumentController * documentController )
{
    d->documentController = documentController;
}

KDevDocumentController * KDevApi::documentController( ) const
{
    return d->documentController;
}

void KDevApi::setPluginController( KDevPluginController * pluginController )
{
    d->pluginController = pluginController;
}

KDevPluginController * KDevApi::pluginController( ) const
{
    return d->pluginController;
}

void KDevApi::setCore( KDevCore * core )
{
    d->core = core;

    new KDevEnv;
}

KDevCore * KDevApi::core( ) const
{
    return d->core;
}

void KDevApi::setEnvironment( KDevEnv * env )
{
    d->environment = env;
}

KDevEnv * KDevApi::environment( ) const
{
    return d->environment;
}

//kate: indent-mode cstyle; indent-width 4; replace-tabs on; space-indent on;
