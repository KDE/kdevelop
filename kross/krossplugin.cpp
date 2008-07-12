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

#include <KUrl>
#include <KProcess>
#include <KDebug>
#include <KStandardDirs>

#include <icore.h>
#include <iproject.h>
#include <iplugincontroller.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <projectmodel.h>

#include <contextmenuextension.h>

K_PLUGIN_FACTORY(KrossSupportFactory, registerPlugin<KrossPlugin>(); )
K_EXPORT_PLUGIN(KrossSupportFactory("kdevkrossmanager"))

extern "C"
{
    KDE_EXPORT QObject* krossmodule()
    {
        return new KrossPlugin();
    }
}

KrossPlugin::KrossPlugin( QObject* parent, const QVariantList& args )
    : KDevelop::IPlugin( KrossSupportFactory::componentData(), parent ), KrossDistributedVersionControl(this),
    action(0)
{
    kDebug() << "Krossing the krossed paths of this krossed world" << args;
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectFileManager )
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IBuildSystemManager )
    
    Q_ASSERT(args.count() >= 2);
    QStringList interfaces = args[0].toStringList();
    QString name = args[1].toString();

    KUrl file=KStandardDirs::installPath("kdedir");
    file.addPath("lib");
    file.addPath("kde4");
    file.addPath(name);
    file.addPath(name+".py");

    action = new Kross::Action(this, file);
    action->setFile(file.toLocalFile());
    
    action->addObject(KDevelop::ICore::self(), "ICore");
    setAction(action); //should add it here and only if it is necessary (when more ifaces)
    setActionDistributed(action); //should add it here and only if it is necessary (when more ifaces)

    action->trigger();
}

KDevelop::ContextMenuExtension KrossPlugin::contextMenuExtension(KDevelop::Context* context)
{
    return distributedMenuExtension(context);
}

#include "krossplugin.moc"
