/*
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "compilerprovider.h"

#include "debug.h"
#include "compilerfactories.h"
#include "settingsmanager.h"

#include <interfaces/iruntime.h>
#include <interfaces/iruntimecontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/ibuildsystemmanager.h>
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
        ICompiler(i18nc("@item no compiler", "None"), QString(), QString(), false)
    {}

    QHash< QString, QString > defines(Utils::LanguageType, const QString&) const override
    {
        return {};
    }

    Path::List includes(Utils::LanguageType, const QString&) const override
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

    const auto paths = SettingsManager::globalInstance()->readPaths(item->project()->projectConfiguration().data());
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

QString parserArguments( const ConfigEntry& config, Utils::LanguageType languageType, ProjectBaseItem* item )
{
    auto args = config.parserArguments[languageType];
    if (item && item->project()->buildSystemManager()) {
        args += QLatin1Char(' ');
        args += item->project()->buildSystemManager()->extraArguments(item);
    }
    return args;
}
}

ProjectTargetItem* findCompiledTarget(ProjectBaseItem* item)
{
    const auto targets = item->targetList();
    for (auto* item : targets) {
        if (item->type() == ProjectBaseItem::ExecutableTarget || item->type() == ProjectBaseItem::LibraryTarget) {
            return item;
        }
    }

    const auto folders = item->folderList();
    for (auto* folder: folders) {
        auto target = findCompiledTarget(folder);
        if (target)
            return target;
    }
    return nullptr;
}

CompilerProvider::CompilerProvider( SettingsManager* settings, QObject* parent )
    : QObject( parent )
    , m_settings(settings)
{
    m_factories = {
        CompilerFactoryPointer(new GccFactory()),
        CompilerFactoryPointer(new ClangFactory()),
#ifdef _WIN32
        CompilerFactoryPointer(new MsvcFactory()),
#endif
    };

    if (!QStandardPaths::findExecutable( QStringLiteral("clang") ).isEmpty()) {
        m_factories[1]->registerDefaultCompilers(this);
    }
    if (!QStandardPaths::findExecutable( QStringLiteral("gcc") ).isEmpty()) {
        m_factories[0]->registerDefaultCompilers(this);
    }
#ifdef _WIN32
    if (!QStandardPaths::findExecutable(QStringLiteral("cl.exe")).isEmpty()) {
        m_factories[2]->registerDefaultCompilers(this);
    }
#endif

    registerCompiler(createDummyCompiler());
    retrieveUserDefinedCompilers();

    connect(ICore::self()->runtimeController(), &IRuntimeController::currentRuntimeChanged, this, [this]() { m_defaultProvider.clear(); });
    connect(ICore::self()->projectController(), &IProjectController::projectConfigurationChanged, this, &CompilerProvider::projectChanged);
    connect(ICore::self()->projectController(), &IProjectController::projectOpened, this, &CompilerProvider::projectChanged);
}

CompilerProvider::~CompilerProvider() = default;

void CompilerProvider::projectChanged(KDevelop::IProject* p)
{
    const auto target = findCompiledTarget(p->projectItem());
    if (!target)
        return;

    auto path = p->buildSystemManager()->compiler(target);
    qCDebug(DEFINESANDINCLUDES) << "found compiler" << path;
    if (path.isEmpty())
        return;

    Q_ASSERT(QDir::isAbsolutePath(path.toLocalFile()));
    const auto pathString = path.toLocalFile();
    auto it = std::find_if(m_compilers.begin(), m_compilers.end(),
        [&pathString](const CompilerPointer& compiler) { return compiler->path() == pathString; });
    if (it != m_compilers.end()) {
        m_defaultProvider = *it;
        return;
    }

    //we need to search, sdk compiler names are weird: arm-linux-androideabi-g++
    for (auto& factory : std::as_const(m_factories)) {
        if (factory->isSupported(path)) {
            auto compiler = factory->createCompiler(path.lastPathSegment(), pathString);
            registerCompiler(compiler);
            m_defaultProvider = compiler;
        }
    }

    qCDebug(DEFINESANDINCLUDES) << "using compiler" << m_defaultProvider << path;
}

QHash<QString, QString> CompilerProvider::defines( const QString& path ) const
{
    auto config = configForItem(nullptr);
    auto languageType = Utils::languageType(path, config.parserArguments.parseAmbiguousAsCPP);
    // If called on files that we can't compile, return an empty set of defines.
    if (languageType == Utils::Other) {
        return {};
    }

    return config.compiler->defines(languageType, parserArguments(config, languageType, nullptr));
}

QHash<QString, QString> CompilerProvider::defines( ProjectBaseItem* item ) const
{
    auto config = configForItem(item);
    auto languageType = Utils::Cpp;
    if (item) {
        languageType = Utils::languageType(item->path().path(), config.parserArguments.parseAmbiguousAsCPP);
    }
    // If called on files that we can't compile, return an empty set of defines.
    if (languageType == Utils::Other) {
        return {};
    }

    return config.compiler->defines(languageType, parserArguments(config, languageType, item));
}

Path::List CompilerProvider::includes( const QString& path ) const
{
    auto config = configForItem(nullptr);
    auto languageType = Utils::languageType(path, config.parserArguments.parseAmbiguousAsCPP);
    // If called on files that we can't compile, return an empty set of includes.
    if (languageType == Utils::Other) {
        return {};
    }

    return config.compiler->includes(languageType, parserArguments(config, languageType, nullptr));
}

Path::List CompilerProvider::includes( ProjectBaseItem* item ) const
{
    auto config = configForItem(item);
    auto languageType = Utils::Cpp;
    if (item) {
        languageType = Utils::languageType(item->path().path(), config.parserArguments.parseAmbiguousAsCPP);
    }
    // If called on files that we can't compile, return an empty set of includes.
    if (languageType == Utils::Other) {
        return {};
    }

    return config.compiler->includes(languageType, parserArguments(config, languageType, item));
}

Path::List CompilerProvider::frameworkDirectories( const QString& /* path */ ) const
{
    return {};
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

    for ( const CompilerPointer& compiler : m_compilers ) {
        if (rt->findExecutable(compiler->path()).isEmpty())
            continue;
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

    for (auto& c : std::as_const(m_compilers)) {
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
    const auto compilers = m_settings->userDefinedCompilers();
    for (auto& c : compilers) {
        registerCompiler(c);
    }
}

#include "moc_compilerprovider.cpp"
