/* KDevPlatform Kross Support
 *
 * Copyright 2008 Aleix Pol <aleixpol@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "krossplugin.h"

#include "krossbuildsystemmanager.h"
#include "krosstoolviewfactory.h"
#include "wrappers/krosscontext.h"
#include "wrappers/krosscontextmenuextension.h"

#include <kross/core/manager.h>
#include <KUrl>
#include <KProcess>
#include <kaboutdata.h>
#include <KDebug>
#include <KStandardDirs>
#include <KToolBar>

#include <language/duchain/duchain.h>
#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iplugincontroller.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <project/projectmodel.h>

#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>

KrossPlugin::KrossPlugin( const QString& scriptfile, const KAboutData& about, QObject* parent )
    : KDevelop::IPlugin( KComponentData( about ), parent ), KrossDistributedVersionControl(this),
    action(0)
{
    kDebug() << "Krossing the krossed paths of this krossed world" << scriptfile;
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectFileManager )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IBuildSystemManager )
    
    Kross::Manager::self().setStrictTypesEnabled(false);
    action = new Kross::Action(this, scriptfile);
    action->setFile(scriptfile);
    
    action->addObject(KDevelop::ICore::self(), "ICore");
    action->addObject(KDevelop::DUChain::self(), "DUChain");
    action->addObject(this, "IPlugin");
    setAction(action); //should add it here and only if it is necessary (when more ifaces)
    setActionDistributed(action); //should add it here and only if it is necessary (when more ifaces)

    action->trigger();
}

KrossPlugin::~KrossPlugin()
{
    foreach(KrossToolViewFactory* toolview, m_toolFactories)
        core()->uiController()->removeToolView(toolview);
}

KDevelop::ContextMenuExtension KrossPlugin::contextMenuExtension(KDevelop::Context* context)
{
    KDevelop::ContextMenuExtension cme;
    QVariant result=action->callFunction("contextMenuExtension",
                                         QVariantList() << Handlers::kDevelopContextHandler(context)
                                                        << Handlers::kDevelopContextMenuExtensionHandler(&cme));
    kDebug() << "retrieving name" << result.toString() << cme.actions(cme.RunGroup);
//     return result.toString();
    return cme;
}

void KrossPlugin::createToolViewFactory(const QString& method, const QString& id, Qt::DockWidgetArea pos)
{
    Q_UNUSED(pos);
    kDebug() << "creating kross tool view" << method << id;
    //KrossToolViewFactory* toolFactory=new KrossToolViewFactory(this, action, method, id, pos);
    KrossToolViewFactory* toolFactory=new KrossToolViewFactory(this, action, method, id, Qt::BottomDockWidgetArea);
    core()->uiController()->addToolView(id, toolFactory);
    m_toolFactories.append(toolFactory);
}

QToolBar* KrossPlugin::createToolBar(const QString& name, QMainWindow* parent)
{
    KToolBar* t=new KToolBar(name, parent, Qt::AllToolBarAreas);
    t->show();
    return t;
}

#include "krossplugin.moc"
