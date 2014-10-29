/* This file is part of the KDE project
 Copyright 2002 Simon Hausmann <hausmann@kde.org>
 Copyright 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 Copyright 2002 Harald Fernengel <harry@kdevelop.org>
 Copyright 2002 Falk Brettschneider <falkbr@kdevelop.org>
 Copyright 2003 Julian Rockey <linux@jrockey.com>
 Copyright 2003 Roberto Raggi <roberto@kdevelop.org>
 Copyright 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
 Copyright 2003 Mario Scalas <mario.scalas@libero.it>
 Copyright 2003-2004,2007 Alexander Dymo <adymo@kdevelop.org>
 Copyright 2006 Adam Treat <treat@kde.org>
 Copyright 2007 Andreas Pakulat <apaku@gmx.de>

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
#include "iplugin.h"

#include <kiconloader.h>
#include <kmainwindow.h>
#include <kxmlguiwindow.h>
#include <kxmlguifactory.h>
#include <kactioncollection.h>
#include "icore.h"
#include "iplugincontroller.h"
#include "iprojectcontroller.h"
#include "contextmenuextension.h"

namespace KDevelop
{

class IPluginPrivate
{
public:
    IPluginPrivate(IPlugin *q)
        : q(q), iconLoader(0)
    {}

    ~IPluginPrivate()
    {
        delete iconLoader;
    }

    void _k_guiClientAdded(KXMLGUIClient *client)
    {
	if (client != q)
	    return;

	q->initializeGuiState();
	_k_updateState();
    }

    void _k_updateState()
    {
	const int projectCount =
	    ICore::self()->projectController()->projectCount();

	IPlugin::ReverseStateChange reverse = IPlugin::StateNoReverse;
	if (! projectCount)
	    reverse = IPlugin::StateReverse;

	q->stateChanged(QLatin1String("has_project"), reverse);
    }

    IPlugin *q;
    ICore *core;
    KIconLoader* iconLoader;
    QStringList m_extensions;
};

IPlugin::IPlugin( const QString &componentName, QObject *parent )
        : QObject( parent ),
	  KXMLGUIClient(), d( new IPluginPrivate(this) )
{
    // The following cast is safe, there's no component in KDevPlatform that
    // creates plugins except the plugincontroller. The controller passes
    // Core::self() as parent to KServiceTypeTrader::createInstanceFromQuery
    // so we know the parent is always a Core* pointer.
    // This is the only way to pass the Core pointer to the plugin during its
    // creation so plugins have access to ICore during their creation.
    d->core = static_cast<KDevelop::ICore*>(parent);

    setComponentName(componentName, componentName);

    foreach (KMainWindow* mw, KMainWindow::memberList()) {
        KXmlGuiWindow* guiWindow = qobject_cast<KXmlGuiWindow*>(mw);
        if (! guiWindow)
            continue;

        connect(guiWindow->guiFactory(), SIGNAL(clientAdded(KXMLGUIClient*)),
		this, SLOT(_k_guiClientAdded(KXMLGUIClient*)));
    }

    connect(ICore::self()->projectController(),
	    SIGNAL(projectOpened(KDevelop::IProject*)),
	    this, SLOT(_k_updateState()));

    connect(ICore::self()->projectController(),
	    SIGNAL(projectClosed(KDevelop::IProject*)),
	    this, SLOT(_k_updateState()));
}

IPlugin::~IPlugin()
{
    delete d;
}

void IPlugin::unload()
{
}

KIconLoader *IPlugin::iconLoader() const
{
    if ( d->iconLoader == 0 ) {
        d->iconLoader = new KIconLoader(componentName());
        d->iconLoader->addAppDir( "kdevelop" );
        connect(d->iconLoader, SIGNAL(iconChanged(int)),
                this, SLOT(newIconLoader()));
    }

    return d->iconLoader;
}

void IPlugin::newIconLoader() const
{
    if (d->iconLoader) {
        d->iconLoader->reconfigure(componentName());
    }
}

ICore *IPlugin::core() const
{
    return d->core;
}

}

QStringList KDevelop::IPlugin::extensions( ) const
{
  return d->m_extensions;
}

void KDevelop::IPlugin::addExtension( const QString& ext )
{
    d->m_extensions << ext;
}

KDevelop::ContextMenuExtension KDevelop::IPlugin::contextMenuExtension(
        KDevelop::Context* )
{
    return KDevelop::ContextMenuExtension();
}

void KDevelop::IPlugin::initializeGuiState()
{ }

class CustomXmlGUIClient : public KXMLGUIClient {
    public:
        CustomXmlGUIClient(const QString& componentName) {
            // TODO KF5: Get rid off this
            setComponentName(componentName, componentName);
        }
        void setXmlFile(QString file) {
            KXMLGUIClient::setXMLFile(file);
        }
};

KXMLGUIClient* KDevelop::IPlugin::createGUIForMainWindow(Sublime::MainWindow* window)
{
    CustomXmlGUIClient* ret = new CustomXmlGUIClient(componentName());
    QString file;
    createActionsForMainWindow(window, file, *ret->actionCollection());

    if(!ret->actionCollection()->isEmpty()) {
        Q_ASSERT(!file.isEmpty()); //A file must have been set
        ret->setXmlFile(file);
    }else{
        delete ret;
        ret = 0;
    }
    return ret;
}

void KDevelop::IPlugin::createActionsForMainWindow( Sublime::MainWindow* /*window*/, QString& /*xmlFile*/, KActionCollection& /*actions*/ )
{
}

bool KDevelop::IPlugin::hasError() const
{
    return false;
}

QString KDevelop::IPlugin::errorDescription() const
{
    return QString();
}

#include "moc_iplugin.cpp"
