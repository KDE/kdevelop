/* This file is part of KDevelop
    Copyright 2012 Aleix Pol Gonzalez <aleixpol@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kdevninjabuilderplugin.h"
#include "ninjajob.h"
#include <KAboutData>
#include <KPluginFactory>
#include <KStandardDirs>
#include <KDebug>
#include <project/projectmodel.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <interfaces/iproject.h>
#include <QFile>

K_PLUGIN_FACTORY(NinjaBuilderFactory, registerPlugin<KDevNinjaBuilderPlugin>(); )
K_EXPORT_PLUGIN(NinjaBuilderFactory(KAboutData("kdevninja", "kdevninja", ki18n("Ninja Builder"), "0.1", ki18n("Support for building Ninja projects"), KAboutData::License_GPL)))

KDevNinjaBuilderPlugin::KDevNinjaBuilderPlugin(QObject* parent, const QVariantList& )
    : KDevelop::IPlugin(NinjaBuilderFactory::componentData(), parent)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IProjectBuilder )
    if(hasError())
        kWarning() << "Ninja plugin installed but ninja is not installed.";
}

bool KDevNinjaBuilderPlugin::hasError() const
{
    return KStandardDirs::findExe("ninja").isEmpty();
}

static QStringList targetsInFolder(KDevelop::ProjectFolderItem* item)
{
    QStringList ret;
    foreach(KDevelop::ProjectTargetItem* target, item->targetList()) {
        ret += target->text();
    }
    foreach(KDevelop::ProjectFolderItem* folder, item->folderList()) {
        ret += targetsInFolder(folder);
    }
    return ret;
}

static QStringList argumentsForItem(KDevelop::ProjectBaseItem* item)
{
    if(!item->parent() &&
        QFile::exists(item->project()->buildSystemManager()->buildDirectory(item->project()->projectItem()).toLocalFile()))
      return QStringList();

    switch(item->type()) {
        case KDevelop::ProjectBaseItem::File:
          return QStringList(item->url().toLocalFile()+'^');
        case KDevelop::ProjectBaseItem::Target:
        case KDevelop::ProjectBaseItem::ExecutableTarget:
        case KDevelop::ProjectBaseItem::LibraryTarget:
          return QStringList(item->target()->text());
        case KDevelop::ProjectBaseItem::Folder:
        case KDevelop::ProjectBaseItem::BuildFolder:
          return targetsInFolder(item->folder());
    }
    return QStringList();
}

KUrl KDevNinjaBuilderPlugin::findNinjaFile(KDevelop::IProject* p)
{
    KUrl ret = p->buildSystemManager()->buildDirectory(p->projectItem());
    while(!QFile::exists(ret.toLocalFile(KUrl::AddTrailingSlash)+"build.ninja") && ret!=ret.upUrl()) {
        ret = ret.upUrl();
    }
    
    if(ret.isEmpty()) {
        ret = p->buildSystemManager()->buildDirectory(p->projectItem());
    }
    
    return ret;
}

NinjaJob* KDevNinjaBuilderPlugin::runNinja(KDevelop::ProjectBaseItem* item, const QStringList& args, const QByteArray& signal)
{
    NinjaJob* job = new NinjaJob(findNinjaFile(item->project()), args, this);
    job->signalWhenFinished(signal, item);
    return job;
}

KJob* KDevNinjaBuilderPlugin::build(KDevelop::ProjectBaseItem* item)
{
    return runNinja(item, argumentsForItem(item), "built");
}

KJob* KDevNinjaBuilderPlugin::clean(KDevelop::ProjectBaseItem* item)
{
    return runNinja(item, QStringList("-t") << "clean", "cleaned");
}

KJob* KDevNinjaBuilderPlugin::install(KDevelop::ProjectBaseItem* item)
{
    return runNinja(item, QStringList("install"), "installed");
}
