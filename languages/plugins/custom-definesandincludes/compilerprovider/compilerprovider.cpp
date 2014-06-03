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

#include "gcclikecompiler.h"
#include "msvccompiler.h"

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

    virtual QString name() const override
    {
        return i18n("None");
    }

    virtual QString defaultPath() const override
    {
        return {};
    }
};
}

CompilerProvider::Compiler CompilerProvider::compilerForItem(ProjectBaseItem* item) const
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

CompilerProvider::Compiler CompilerProvider::selectCompiler( const QString& compilerName, const QString& path ) const
{
    //This may happen for opened for the first time projects
    if ( compilerName.isEmpty() ) {
        for ( auto& compiler : m_compilers ) {
            if ( KStandardDirs::findExe( compiler->defaultPath() ).isEmpty() ) {
                continue;
            }
            definesAndIncludesDebug() << "Selected compiler: " << compiler->name();
            return {compiler, !path.isEmpty() ? path : compiler->defaultPath()};
        }
        kWarning() << "No compiler found. Standard includes/defines won't be provided to the project parser!";
    }else{
        for ( auto it = m_compilers.constBegin(); it != m_compilers.constEnd(); it++ ) {
            if ( ( *it )->name() == compilerName ) {
                return {*it, !path.isEmpty() ? path : ( *it )->defaultPath()};
            }
        }
    }

    return {CompilerPointer(new DummyCompiler()), QString()};
}

bool CompilerProvider::setCompiler( IProject* project, const QString& name, const QString& path )
{
    auto compiler = selectCompiler( name, path.trimmed() );
    Q_ASSERT(compiler.compiler);

    addPoject( project, compiler );

    return true;
}

void CompilerProvider::projectOpened( KDevelop::IProject* project )
{
    definesAndIncludesDebug() << "Adding project: " << project->name();
    auto settings = static_cast<DefinesAndIncludesManager*>( IDefinesAndIncludesManager::manager() );
    auto projectConfig =  project->projectConfiguration().data();

    auto path = settings->pathToCompiler( projectConfig );
    auto name = settings->currentCompiler( projectConfig );
    auto compiler = selectCompiler( name, path.trimmed() );

    if ( compiler.compiler && ( compiler.compiler->name() != name ) ) {
        settings->writeCompiler( projectConfig, compiler.compiler->name() );
        settings->writePathToCompiler( projectConfig, compiler.path );
    }
    definesAndIncludesDebug() << " compiler is: " << ( compiler.compiler ? compiler.compiler->name() : nullptr );

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

    m_compilers.append( CompilerPointer( new ClangCompiler() ) );
    m_compilers.append( CompilerPointer( new GccCompiler() ) );
    m_compilers.append( CompilerPointer( new MsvcCompiler() ) );
    m_compilers.append( CompilerPointer( new DummyCompiler() ) );

    IDefinesAndIncludesManager::manager()->registerProvider( this );

    connect( ICore::self()->projectController(), SIGNAL( projectAboutToBeOpened( KDevelop::IProject* ) ), SLOT( projectOpened( KDevelop::IProject* ) ) );
    connect( ICore::self()->projectController(), SIGNAL( projectClosed( KDevelop::IProject* ) ), SLOT( projectClosed( KDevelop::IProject* ) ) );

    //Add a provider for files without project
    addPoject( nullptr, selectCompiler({}, {}));
}

QVector< CompilerPointer > CompilerProvider::compilers() const
{
    return m_compilers;
}

CompilerPointer CompilerProvider::currentCompiler(IProject* project) const
{
    Q_ASSERT(m_projects.contains(project));
    return m_projects[project].compiler;
}

#include "compilerprovider.moc"
