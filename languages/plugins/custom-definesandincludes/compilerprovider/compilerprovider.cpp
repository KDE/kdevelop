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

#include "../debugarea.h"

#include "clangfactory.h"
#include "gccfactory.h"
#include "msvcfactory.h"

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <project/projectmodel.h>

#include <KPluginFactory>
#include <KAboutData>
#include <KStandardDirs>

using namespace KDevelop;
using KDevelop::Path;

namespace
{
class DummyCompiler : public ICompiler
{
    virtual QHash< QString, QString > defines(const QString&) const override
    {
        return {};
    }

    virtual Path::List includes(const QString&) const override
    {
        return {};
    }
};

class DummyFactory : public ICompilerFactory
{
    virtual Compiler createCompiler(const QString&, const QString&, bool = true) {
        static Compiler compiler = {CompilerPointer(new DummyCompiler()), i18n("None"), "", false};
        return compiler;
    }

    virtual QString name() const {
        return "";
    }
};
}

Compiler CompilerProvider::compilerForItem(ProjectBaseItem* item) const
{
    auto project = item ? item->project() : nullptr;
    Q_ASSERT(m_projects.contains(project));
    auto info = m_projects[project];
    Q_ASSERT(info.compiler);
    return info;
}

QHash<QString, QString> CompilerProvider::defines( ProjectBaseItem* item ) const
{
    auto info = compilerForItem(item);
    return info.compiler->defines(info.path);
}

Path::List CompilerProvider::includes( ProjectBaseItem* item ) const
{
    auto info = compilerForItem(item);
    return info.compiler->includes(info.path);
}

IDefinesAndIncludesManager::Type CompilerProvider::type() const
{
    return IDefinesAndIncludesManager::CompilerSpecific;
}

void CompilerProvider::addPoject( IProject* project, Compiler compiler )
{
    Q_ASSERT(compiler.compiler);
    //cache includes/defines
    compiler.compiler->includes(compiler.path);
    compiler.compiler->defines(compiler.path);
    m_projects[project] = compiler;
}

void CompilerProvider::removePoject( IProject* project )
{
    m_projects.remove( project );
}

CompilerProvider::~CompilerProvider()
{
    IDefinesAndIncludesManager::manager()->unregisterProvider( this );
}

Compiler CompilerProvider::CheckCompiler( Compiler compiler ) const
{
    //This may happen for opened for the first time projects
    if ( !compiler.compiler ) {
        for ( auto& compiler : m_compilers ) {
            if ( KStandardDirs::findExe( compiler.path ).isEmpty() ) {
                continue;
            }
            definesAndIncludesDebug() << "Selected compiler: " << compiler.name;
            return compiler;
        }
        kWarning() << "No compiler found. Standard includes/defines won't be provided to the project parser!";
    }else{
        for ( auto it = m_compilers.constBegin(); it != m_compilers.constEnd(); it++ ) {
            if ( it->name == compiler.name ) {
                return *it;
            }
        }
    }

    return m_factories.last()->createCompiler("", "");
}

bool CompilerProvider::setCompiler( IProject* project, Compiler compiler )
{
    auto c = CheckCompiler( compiler );
    Q_ASSERT(c.compiler);

    addPoject( project, c );

    return true;
}

void CompilerProvider::projectOpened( KDevelop::IProject* project )
{
    definesAndIncludesDebug() << "Adding project: " << project->name();
    auto settings = static_cast<DefinesAndIncludesManager*>( IDefinesAndIncludesManager::manager() );
    auto projectConfig =  project->projectConfiguration().data();

    auto compiler = settings->currentCompiler( projectConfig );
    auto name = compiler.name;
    compiler = CheckCompiler( compiler );

    if ( compiler.compiler && ( compiler.name != name ) ) {
        settings->writeCurrentCompiler(projectConfig, compiler);
    }
    definesAndIncludesDebug() << " compiler is: " << compiler.name;

    addPoject( project, compiler );
}

void CompilerProvider::projectClosed( KDevelop::IProject* project )
{
    removePoject( project );
    definesAndIncludesDebug() << "Removed project: " << project->name();
}

K_PLUGIN_FACTORY( CompilerProviderFactory, registerPlugin<CompilerProvider>(); )
K_EXPORT_PLUGIN( CompilerProviderFactory( KAboutData( "kdevcompilerprovider",
            "kdevcompilerprovider", ki18n( "Compiler Provider" ), "0.1", ki18n( "" ),
            KAboutData::License_GPL ) ) )

CompilerProvider::CompilerProvider( QObject* parent, const QVariantList& )
    : IPlugin( CompilerProviderFactory::componentData(), parent )
{
    KDEV_USE_EXTENSION_INTERFACE( ICompilerProvider );

    m_factories.append(CompilerFactoryPointer(new GccFactory()));
    m_factories.append(CompilerFactoryPointer(new ClangFactory()));
#ifdef _WIN32
    m_factories.append(CompilerFactoryPointer(new MsvcFactory()));
#endif
    //NOTE: keep it the last in the list
    m_factories.append(CompilerFactoryPointer(new DummyFactory()));

    if (!KStandardDirs::findExe( "gcc" ).isEmpty()) {
        registerCompiler( m_factories[0]->createCompiler("GCC", "gcc", false) );
    }
    if (!KStandardDirs::findExe( "clang" ).isEmpty()) {
        registerCompiler( m_factories[1]->createCompiler("Clang", "clang", false) );
    }
#ifdef _WIN32
    if (!KStandardDirs::findExe("cl.exe").isEmpty()) {
        registerCompiler(m_factories[2]->createCompiler("MSVC", "cl.exe", false));
    }
#endif

    registerCompiler(m_factories.last()->createCompiler("", ""));

    IDefinesAndIncludesManager::manager()->registerProvider( this );

    connect( ICore::self()->projectController(), SIGNAL( projectAboutToBeOpened( KDevelop::IProject* ) ), SLOT( projectOpened( KDevelop::IProject* ) ) );
    connect( ICore::self()->projectController(), SIGNAL( projectClosed( KDevelop::IProject* ) ), SLOT( projectClosed( KDevelop::IProject* ) ) );

    //Add a provider for files without project
    addPoject( nullptr, CheckCompiler({}));
}

QVector< Compiler > CompilerProvider::compilers() const
{
    return m_compilers;
}

Compiler CompilerProvider::currentCompiler(IProject* project) const
{
    Q_ASSERT(m_projects.contains(project));
    return m_projects[project];
}

bool CompilerProvider::registerCompiler(const Compiler& compiler)
{
    for(const auto& c: m_compilers){
        if (c.name == compiler.name) {
            return false;
        }
    }
    m_compilers.append(compiler);
    return true;
}

void CompilerProvider::unregisterCompiler(const Compiler& compiler)
{
    for (int i = 0; i < m_compilers.count(); i++) {
        if (m_compilers[i].name == compiler.name) {
            m_compilers.remove(i);
            break;
        }
    }
}

QVector< CompilerFactoryPointer > CompilerProvider::compilerFactories() const
{
    return m_factories;
}

#include "compilerprovider.moc"
