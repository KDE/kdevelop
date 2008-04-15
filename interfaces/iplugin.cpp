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
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kcomponentdata.h>
#include <kiconloader.h>
#include <kmainwindow.h>
#include <kxmlguiwindow.h>
#include <kxmlguifactory.h>
#include <kdebug.h>
#include "icore.h"
#include "iplugincontroller.h"
#include "contextmenuextension.h"
#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>

namespace KDevelop
{

class PluginExtensionFactory : public QExtensionFactory {
public:
    PluginExtensionFactory( const QStringList& extensions, QExtensionManager *parent = 0 )
        :QExtensionFactory( parent ), m_extensions( extensions )
    {
    }
    protected:
    virtual QObject *createExtension(QObject* object, const QString& iid, QObject* parent ) const
    {
        Q_UNUSED(parent)
        if( !m_extensions.contains( iid ) )
            return 0;
        IPlugin* p = qobject_cast<IPlugin *>(object);
        if( !p )
            return 0;
        return object;
    }
    private:
        QStringList m_extensions;
};

class IPluginPrivate
{
public:
    IPluginPrivate()
        : iconLoader(0), m_factory(0)
    {}

    ~IPluginPrivate()
    {
        delete iconLoader;
    }

    ICore *core;
    KIconLoader* iconLoader;
    PluginExtensionFactory* m_factory;
    QStringList m_extensions;
};

IPlugin::IPlugin( const KComponentData &instance, QObject *parent )
        : QObject( parent ),
        KXMLGUIClient(), d( new IPluginPrivate )
{
    // The following cast is safe, there's no component in KDevPlatform that
    // creates plugins except the plugincontroller. The controller passes
    // Core::self() as parent to KServiceTypeTrader::createInstanceFromQuery
    // so we know the parent is always a Core* pointer.
    // This is the only way to pass the Core pointer to the plugin during its
    // creation so plugins have access to ICore during their creation.
    d->core = static_cast<KDevelop::ICore*>(parent);
    setComponentData( instance );

    foreach (KMainWindow* mw, KMainWindow::memberList()) {
	KXmlGuiWindow* guiWindow = qobject_cast<KXmlGuiWindow*>(mw);
	if (! guiWindow)
	    continue;

	connect(guiWindow->guiFactory(), SIGNAL(clientAdded(KXMLGUIClient *)),
		this, SLOT(guiClientAdded(KXMLGUIClient *)));
    }
}

IPlugin::~IPlugin()
{
    delete d;
}

void IPlugin::guiClientAdded(KXMLGUIClient *)
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

void IPlugin::unload()
{
    ICore::self()->pluginController()->pluginUnloading(this);
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

QStringList KDevelop::IPlugin::extensions( ) const
{
  return d->m_extensions;
}

void KDevelop::IPlugin::registerExtensions( )
{
    if( extensions().isEmpty() )
        return;
    if( !d->m_factory )
        d->m_factory = new PluginExtensionFactory( extensions(), extensionManager() );
    Q_FOREACH( QString ext, extensions() )
    {
        extensionManager()->registerExtensions( d->m_factory, ext );
    }
}

void KDevelop::IPlugin::unregisterExtensions( )
{
    if( extensions().isEmpty() )
        return;
    if( !d->m_factory )
        return;
    Q_FOREACH( QString ext, extensions() )
    {
        extensionManager()->unregisterExtensions( d->m_factory, ext );
    }

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

#include "iplugin.moc"


