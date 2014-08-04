/*
 * This file is part of KDevelop
 *
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include "definesandincludesmanager.h"

#include "settingsmanager.h"

#include "noprojectincludesanddefines/noprojectincludepathsmanager.h"
#include "makefileresolver/makefileresolver.h"

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>

#include <KPluginFactory>
#include <KAboutData>

#include <QThread>
#include <QCoreApplication>

using KDevelop::ConfigEntry;

namespace
{
///@return: The ConfigEntry, with includes/defines from @p paths for all parent folders of @p item.
static ConfigEntry findConfigForItem(const QList<ConfigEntry>& paths, const KDevelop::ProjectBaseItem* item)
{
    ConfigEntry ret;

    const Path itemPath = item->path();
    const Path rootDirectory = item->project()->path();

    for (const ConfigEntry & entry : paths) {
        Path targetDirectory = rootDirectory;
        // note: a dot represents the project root
        if (entry.path != ".") {
            targetDirectory.addPath(entry.path);
        }

        if (targetDirectory == itemPath || targetDirectory.isParentOf(itemPath)) {
            ret.includes += entry.includes;

            for (auto it = entry.defines.constBegin(); it != entry.defines.constEnd(); it++) {
                if (!ret.defines.contains(it.key())) {
                    ret.defines[it.key()] = it.value();
                }
            }
        }
    }
    ret.includes.removeDuplicates();
    return ret;
}

KDevelop::IDefinesAndIncludesManager::Provider* compilerProvider(QVector<KDevelop::IDefinesAndIncludesManager::Provider*> providers)
{
    for (auto provider : providers) {
        if (provider->type() & KDevelop::IDefinesAndIncludesManager::CompilerSpecific) {
            return provider;
        }
    }
    return {};
}
}

namespace KDevelop
{

K_PLUGIN_FACTORY(DefinesAndIncludesManagerFactory, registerPlugin<DefinesAndIncludesManager>(); )
K_EXPORT_PLUGIN(DefinesAndIncludesManagerFactory(KAboutData("kdevdefinesandincludesmanager",
"kdevdefinesandincludesmanager", ki18n("Custom Defines and Includes Manager"), "0.1", ki18n(""),
KAboutData::License_GPL)))

DefinesAndIncludesManager::DefinesAndIncludesManager( QObject* parent, const QVariantList& )
    : IPlugin( DefinesAndIncludesManagerFactory::componentData(), parent )
    , m_noProjectIPM(new NoProjectIncludePathsManager())
    , m_makeFileResolver(new MakeFileResolver())
{
    KDEV_USE_EXTENSION_INTERFACE(IDefinesAndIncludesManager);
}

QHash<QString, QString> DefinesAndIncludesManager::defines( ProjectBaseItem* item, Type type  ) const
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    if (!item) {
        return defines(QString());
    }

    QHash<QString, QString> defines;

    for (auto provider : m_providers) {
        if (provider->type() & type) {
            auto result = provider->defines(item);
            for (auto it = result.constBegin(); it != result.constEnd(); it++) {
                defines[it.key()] = it.value();
            }
        }
    }

    if ( type & ProjectSpecific ) {
        auto buildManager = item->project()->buildSystemManager();
        if ( buildManager ) {
            auto def = buildManager->defines(item);
            for ( auto it = def.constBegin(); it != def.constEnd(); it++ ) {
                defines[it.key()] = it.value();
            }
        }
    }

    // Manually set defines have the highest priority and overwrite values of all other types of defines.
    if (type & UserDefined) {
        auto cfg = item->project()->projectConfiguration().data();

        const auto result = findConfigForItem(readPaths(cfg), item).defines;
        for (auto it = result.constBegin(); it != result.constEnd(); it++) {
            defines[it.key()] = it.value().toString();
        }
    }

    return defines;
}

Path::List DefinesAndIncludesManager::includes( ProjectBaseItem* item, Type type ) const
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    if (!item) {
        return includes(QString());;
    }

    Path::List includes;

    if (type & UserDefined) {
        auto cfg = item->project()->projectConfiguration().data();

        includes += KDevelop::toPathList(findConfigForItem(readPaths(cfg), item).includes);
    }

    if ( type & ProjectSpecific ) {
        auto buildManager = item->project()->buildSystemManager();
        if ( buildManager ) {
            includes += buildManager->includeDirectories(item);
        }
    }

    for (auto provider : m_providers) {
        if (provider->type() & type) {
            includes += provider->includes(item);
        }
    }

    return includes;
}

bool DefinesAndIncludesManager::unregisterProvider(IDefinesAndIncludesManager::Provider* provider)
{
    int idx = m_providers.indexOf(provider);
    if (idx != -1) {
        m_providers.remove(idx);
        return true;
    }

    return false;
}

void DefinesAndIncludesManager::registerProvider(IDefinesAndIncludesManager::Provider* provider)
{
    Q_ASSERT(provider);
    if (m_providers.contains(provider)) {
        return;
    }

    m_providers.push_back(provider);
}

QHash< QString, QString > DefinesAndIncludesManager::defines(const QString&) const
{
    auto cp = compilerProvider(m_providers);
    return cp ? cp->defines(nullptr) : QHash<QString, QString>();
}

Path::List DefinesAndIncludesManager::includes(const QString& path) const
{
    Path::List includes;
    auto cp = compilerProvider(m_providers);
    if (cp) {
        includes += cp->includes(nullptr);
    }
    includes += m_noProjectIPM->includes(path);
    return includes;
}

void DefinesAndIncludesManager::openConfigurationDialog(const QString& pathToFile)
{
    if (auto project = KDevelop::ICore::self()->projectController()->findProjectForUrl(pathToFile)) {
        KDevelop::ICore::self()->projectController()->configureProject(project);
    } else {
        m_noProjectIPM->openConfigurationDialog(pathToFile);
    }
}

Path::List DefinesAndIncludesManager::includesInBackground(const QString& path) const
{
    return toPathList(m_makeFileResolver->resolveIncludePath(path).paths);
}

// NOTE: Part of a fix for build failures on <GCC-4.7
DefinesAndIncludesManager::~DefinesAndIncludesManager() noexcept = default;

}

#include "definesandincludesmanager.moc"
