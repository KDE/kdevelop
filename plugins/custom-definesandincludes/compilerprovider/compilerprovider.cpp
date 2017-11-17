/*
 * This file is part of KDevelop
 *
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "compilerprovider.h"

#include "debug.h"
#include "qtcompat_p.h"
#include "compilerfactories.h"
#include "settingsmanager.h"

#include <interfaces/iruntime.h>
#include <interfaces/iruntimecontroller.h>
#include <interfaces/iproject.h>
#include <project/projectmodel.h>

#include <KLocalizedString>
#include <QStandardPaths>
#include <QDir>

using namespace KDevelop;

namespace
{
class NoCompiler : public ICompiler
{
public:
    NoCompiler():
        ICompiler(i18n("None"), QString(), QString(), false)
    {}

    QHash< QString, QString > defines(const QString&) const override
    {
        return {};
    }

    Path::List includes(const QString&) const override
    {
        return {};
    }
};

static CompilerPointer createDummyCompiler()
{
    static CompilerPointer compiler(new NoCompiler());
    return compiler;
}

ConfigEntry configForItem(KDevelop::ProjectBaseItem* item)
{
    if(!item){
        return ConfigEntry();
    }

    const Path itemPath = item->path();
    const Path rootDirectory = item->project()->path();

    auto paths = SettingsManager::globalInstance()->readPaths(item->project()->projectConfiguration().data());
    ConfigEntry config;
    Path closestPath;

    // find config entry closest to the requested item
    for (const auto& entry : paths) {
        auto configEntry = entry;
        Path targetDirectory = rootDirectory;

        targetDirectory.addPath(entry.path);

        if (targetDirectory == itemPath) {
            return configEntry;
        }

        if (targetDirectory.isParentOf(itemPath)) {
            if (config.path.isEmpty() || targetDirectory.segments().size() > closestPath.segments().size()) {
                config = configEntry;
                closestPath = targetDirectory;
            }
        }
    }

    return config;
}
}

CompilerProvider::CompilerProvider( SettingsManager* settings, QObject* parent )
    : QObject( parent )
    , m_settings(settings)
{
    m_factories.append(CompilerFactoryPointer(new GccFactory()));
    m_factories.append(CompilerFactoryPointer(new ClangFactory()));
#ifdef _WIN32
    m_factories.append(CompilerFactoryPointer(new MsvcFactory()));
#endif

    if (!QStandardPaths::findExecutable( QStringLiteral("clang") ).isEmpty()) {
        m_factories[1]->registerDefaultCompilers(this);
    }
    if (!QStandardPaths::findExecutable( QStringLiteral("gcc") ).isEmpty()) {
        m_factories[0]->registerDefaultCompilers(this);
    }
#ifdef _WIN32
    if (!QStandardPaths::findExecutable("cl.exe").isEmpty()) {
        m_factories[2]->registerDefaultCompilers(this);
    }
#endif

    registerCompiler(createDummyCompiler());
    retrieveUserDefinedCompilers();

    connect(ICore::self()->runtimeController(), &IRuntimeController::currentRuntimeChanged, this, [this]() { m_defaultProvider.clear(); });
}

CompilerProvider::~CompilerProvider() = default;

QHash<QString, QString> CompilerProvider::defines( ProjectBaseItem* item ) const
{
    auto config = configForItem(item);
    auto languageType = Utils::Cpp;
    if (item) {
        languageType = Utils::languageType(item->path(), config.parserArguments.parseAmbiguousAsCPP);
    }
    // If called on files that we can't compile, return an empty set of defines.
    if (languageType == Utils::Other) {
        return {};
    }

    return config.compiler->defines(languageType == Utils::C ? config.parserArguments.cArguments : config.parserArguments.cppArguments);
}

Path::List CompilerProvider::includes( ProjectBaseItem* item ) const
{
    auto config = configForItem(item);
    auto languageType = Utils::Cpp;
    if (item) {
        languageType = Utils::languageType(item->path(), config.parserArguments.parseAmbiguousAsCPP);
    }
    // If called on files that we can't compile, return an empty set of includes.
    if (languageType == Utils::Other) {
        return {};
    }

    return config.compiler->includes(languageType == Utils::C ? config.parserArguments.cArguments : config.parserArguments.cppArguments);
}

Path::List CompilerProvider::frameworkDirectories( ProjectBaseItem* /* item */ ) const
{
    return {};
}

IDefinesAndIncludesManager::Type CompilerProvider::type() const
{
    return IDefinesAndIncludesManager::CompilerSpecific;
}

CompilerPointer CompilerProvider::defaultCompiler() const
{
    if (m_defaultProvider)
        return m_defaultProvider;

    auto rt = ICore::self()->runtimeController()->currentRuntime();
    const auto path = QFile::decodeName(rt->getenv("PATH")).split(QtCompat::listSeparator());

    for ( const CompilerPointer& compiler : m_compilers ) {
        const bool absolutePath = QDir::isAbsolutePath(compiler->path());
        if ((absolutePath && QFileInfo::exists(rt->pathInHost(Path(compiler->path())).toLocalFile()))
            || QStandardPaths::findExecutable( compiler->path(), path).isEmpty() ) {
            continue;
        }

        m_defaultProvider = compiler;
        break;
    }
    if (!m_defaultProvider)
        m_defaultProvider = createDummyCompiler();

    qCDebug(DEFINESANDINCLUDES) << "new default compiler" << rt->name() << m_defaultProvider->name() << m_defaultProvider->path();
    return m_defaultProvider;
}

QVector< CompilerPointer > CompilerProvider::compilers() const
{
    return m_compilers;
}

CompilerPointer CompilerProvider::compilerForItem( KDevelop::ProjectBaseItem* item ) const
{
    auto compiler = configForItem(item).compiler;
    Q_ASSERT(compiler);
    return compiler;
}

bool CompilerProvider::registerCompiler(const CompilerPointer& compiler)
{
    if (!compiler) {
        return false;
    }

    for(auto c: m_compilers){
        if (c->name() == compiler->name()) {
            return false;
        }
    }
    m_compilers.append(compiler);
    return true;
}

void CompilerProvider::unregisterCompiler(const CompilerPointer& compiler)
{
    if (!compiler->editable()) {
        return;
    }

    for (int i = 0; i < m_compilers.count(); i++) {
        if (m_compilers[i]->name() == compiler->name()) {
            m_compilers.remove(i);
            break;
        }
    }
}

QVector< CompilerFactoryPointer > CompilerProvider::compilerFactories() const
{
    return m_factories;
}

void CompilerProvider::retrieveUserDefinedCompilers()
{
    auto compilers = m_settings->userDefinedCompilers();
    for (auto c : compilers) {
        registerCompiler(c);
    }
}
