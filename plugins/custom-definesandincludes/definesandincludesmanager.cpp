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

#include "kcm_widget/definesandincludesconfigpage.h"
#include "compilerprovider/compilerprovider.h"
#include "compilerprovider/widget/compilerswidget.h"
#include "noprojectincludesanddefines/noprojectincludepathsmanager.h"

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>

#include <KPluginFactory>

#include <QThread>
#include <QCoreApplication>

#include <algorithm>

using namespace KDevelop;

namespace
{
///@return: The ConfigEntry, with includes/defines from @p paths for all parent folders of @p item.
static ConfigEntry findConfigForItem(QVector<ConfigEntry> paths, const KDevelop::ProjectBaseItem* item)
{
    ConfigEntry ret;

    const Path itemPath = item->path();
    const Path rootDirectory = item->project()->path();
    Path closestPath;

    std::sort(paths.begin(), paths.end(), [] (const ConfigEntry& lhs, const ConfigEntry& rhs) {
        // sort in reverse order to do a bottom-up search
        return lhs.path > rhs.path;
    });

    for (const ConfigEntry & entry : paths) {
        Path targetDirectory = rootDirectory;
        // note: a dot represents the project root
        if (entry.path != QLatin1String(".")) {
            targetDirectory.addPath(entry.path);
        }

        if (targetDirectory == itemPath || targetDirectory.isParentOf(itemPath)) {
            ret.includes += entry.includes;

            for (auto it = entry.defines.constBegin(); it != entry.defines.constEnd(); it++) {
                if (!ret.defines.contains(it.key())) {
                    ret.defines[it.key()] = it.value();
                }
            }

            if (targetDirectory.segments().size() > closestPath.segments().size()) {
                ret.parserArguments = entry.parserArguments;
                closestPath = targetDirectory;
            }
        }
    }
    ret.includes.removeDuplicates();

    Q_ASSERT(!ret.parserArguments.isAnyEmpty());

    return ret;
}

void merge(Defines* target, const Defines& source)
{
    if (target->isEmpty()) {
        *target = source;
        return;
    }

    for (auto it = source.constBegin(); it != source.constEnd(); ++it) {
        target->insert(it.key(), it.value());
    }
}

QString argumentsForPath(const QString& path, const ParserArguments& arguments)
{
    auto languageType = Utils::languageType(path, arguments.parseAmbiguousAsCPP);
    if (languageType != Utils::Other)
        return arguments[languageType];
    else
        return {};
}

}

K_PLUGIN_FACTORY_WITH_JSON(DefinesAndIncludesManagerFactory, "kdevdefinesandincludesmanager.json", registerPlugin<DefinesAndIncludesManager>(); )

DefinesAndIncludesManager::DefinesAndIncludesManager( QObject* parent, const QVariantList& )
    : IPlugin(QStringLiteral("kdevdefinesandincludesmanager"), parent )
    , m_settings(SettingsManager::globalInstance())
    , m_noProjectIPM(new NoProjectIncludePathsManager())
{
    registerProvider(m_settings->provider());
#ifdef Q_OS_OSX
    m_defaultFrameworkDirectories += Path(QStringLiteral("/Library/Frameworks"));
    m_defaultFrameworkDirectories += Path(QStringLiteral("/System/Library/Frameworks"));
#endif
}

DefinesAndIncludesManager::~DefinesAndIncludesManager() = default;

Defines DefinesAndIncludesManager::defines( ProjectBaseItem* item, Type type  ) const
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    if (!item) {
        return m_settings->provider()->defines(nullptr);
    }

    Defines defines;

    for (auto provider : m_providers) {
        if (provider->type() & type) {
            merge(&defines, provider->defines(item));
        }
    }

    if ( type & ProjectSpecific ) {
        auto buildManager = item->project()->buildSystemManager();
        if ( buildManager ) {
            merge(&defines, buildManager->defines(item));
        }
    }

    // Manually set defines have the highest priority and overwrite values of all other types of defines.
    if (type & UserDefined) {
        auto cfg = item->project()->projectConfiguration().data();

        merge(&defines, findConfigForItem(m_settings->readPaths(cfg), item).defines);
    }

    merge(&defines, m_noProjectIPM->includesAndDefines(item->path().path()).second);

    return defines;
}

Path::List DefinesAndIncludesManager::includes( ProjectBaseItem* item, Type type ) const
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    if (!item) {
        return m_settings->provider()->includes(nullptr);
    }

    Path::List includes;

    if (type & UserDefined) {
        auto cfg = item->project()->projectConfiguration().data();

        includes += KDevelop::toPathList(findConfigForItem(m_settings->readPaths(cfg), item).includes);
    }

    if ( type & ProjectSpecific ) {
        auto buildManager = item->project()->buildSystemManager();
        if ( buildManager ) {
            includes += buildManager->includeDirectories(item);
        }
    }

    for (auto provider : m_providers) {
        if ( !(provider->type() & type) ) {
            continue;
        }
        const auto newItems = provider->includes(item);
        if ( provider->type() & DefinesAndIncludesManager::CompilerSpecific ) {
            // If an item occurs in the "compiler specific" list, but was previously supplied
            // in the user include path list already, remove it from there.
            // Re-ordering the system include paths causes confusion in some cases.
            for (const auto& x : newItems) {
                includes.removeAll(x);
            }
        }
        includes += newItems;
    }

    includes += m_noProjectIPM->includesAndDefines(item->path().path()).first;

    return includes;
}

Path::List DefinesAndIncludesManager::frameworkDirectories( ProjectBaseItem* item, Type type ) const
{
    Q_ASSERT(QThread::currentThread() == qApp->thread());

    if (!item) {
        return m_settings->provider()->frameworkDirectories(nullptr);
    }

    Path::List frameworkDirectories = m_defaultFrameworkDirectories;

    if ( type & ProjectSpecific ) {
        auto buildManager = item->project()->buildSystemManager();
        if ( buildManager ) {
            frameworkDirectories += buildManager->frameworkDirectories(item);
        }
    }

    for (auto provider : m_providers) {
        if (provider->type() & type) {
            frameworkDirectories += provider->frameworkDirectories(item);
        }
    }

    return frameworkDirectories;
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

Defines DefinesAndIncludesManager::defines(const QString& path, Type type) const
{
    Defines ret;
    if ( type & CompilerSpecific ) {
        merge(&ret, m_settings->provider()->defines(path));
    }
    if ( type & ProjectSpecific ) {
        merge(&ret, m_noProjectIPM->includesAndDefines(path).second);
    }
    return ret;
}

Path::List DefinesAndIncludesManager::includes(const QString& path, Type type) const
{
    Path::List ret;
    if ( type & CompilerSpecific ) {
        ret += m_settings->provider()->includes(path);
    }
    if ( type & ProjectSpecific ) {
        ret += m_noProjectIPM->includesAndDefines(path).first;
    }
    return ret;
}

Path::List DefinesAndIncludesManager::frameworkDirectories(const QString& path, Type type) const
{
    return (type & CompilerSpecific) ? m_settings->provider()->frameworkDirectories(path) : Path::List();
}

void DefinesAndIncludesManager::openConfigurationDialog(const QString& pathToFile)
{
    if (auto project = KDevelop::ICore::self()->projectController()->findProjectForUrl(QUrl::fromLocalFile(pathToFile))) {
        KDevelop::ICore::self()->projectController()->configureProject(project);
    } else {
        m_noProjectIPM->openConfigurationDialog(pathToFile);
    }
}

Path::List DefinesAndIncludesManager::includesInBackground(const QString& path) const
{
    Path::List includes;

    for (auto provider: m_backgroundProviders) {
        includes += provider->includesInBackground(path);
    }

    return includes;
}

Path::List DefinesAndIncludesManager::frameworkDirectoriesInBackground(const QString& path) const
{
    Path::List fwDirs;

    for (auto provider: m_backgroundProviders) {
        fwDirs += provider->frameworkDirectoriesInBackground(path);
    }

    return fwDirs;
}

Defines DefinesAndIncludesManager::definesInBackground(const QString& path) const
{
    QHash<QString, QString> defines;

    for (auto provider: m_backgroundProviders) {
        auto result = provider->definesInBackground(path);
        for (auto it = result.constBegin(); it != result.constEnd(); it++) {
            defines[it.key()] = it.value();
        }
    }

    merge(&defines, m_noProjectIPM->includesAndDefines(path).second);

    return defines;
}

bool DefinesAndIncludesManager::unregisterBackgroundProvider(IDefinesAndIncludesManager::BackgroundProvider* provider)
{
    int idx = m_backgroundProviders.indexOf(provider);
    if (idx != -1) {
        m_backgroundProviders.remove(idx);
        return true;
    }

    return false;
}

void DefinesAndIncludesManager::registerBackgroundProvider(IDefinesAndIncludesManager::BackgroundProvider* provider)
{
    Q_ASSERT(provider);
    if (m_backgroundProviders.contains(provider)) {
        return;
    }

    m_backgroundProviders.push_back(provider);
}

QString DefinesAndIncludesManager::parserArguments(KDevelop::ProjectBaseItem* item) const
{
    Q_ASSERT(item);

    Q_ASSERT(QThread::currentThread() == qApp->thread());

    auto cfg = item->project()->projectConfiguration().data();
    const auto parserArguments = findConfigForItem(m_settings->readPaths(cfg), item).parserArguments;
    auto arguments = argumentsForPath(item->path().path(), parserArguments);

    auto buildManager = item->project()->buildSystemManager();
    if ( buildManager ) {
        const auto extraArguments = buildManager->extraArguments(item);
        if (!extraArguments.isEmpty())
            arguments += ' ' + extraArguments;
    }

    return arguments;
}

QString DefinesAndIncludesManager::parserArguments(const QString& path) const
{
    const auto args = m_settings->defaultParserArguments();
    return argumentsForPath(path, args);
}

int DefinesAndIncludesManager::perProjectConfigPages() const
{
    return 1;
}

ConfigPage* DefinesAndIncludesManager::perProjectConfigPage(int number, const ProjectConfigOptions& options, QWidget* parent)
{
    if (number == 0) {
        return new DefinesAndIncludesConfigPage(this, options, parent);
    }
    return nullptr;
}

KDevelop::ConfigPage* DefinesAndIncludesManager::configPage(int number, QWidget* parent)
{
    return number == 0 ? new CompilersWidget(parent) : nullptr;
}

int DefinesAndIncludesManager::configPages() const
{
    return 1;
}

#include "definesandincludesmanager.moc"
