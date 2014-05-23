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

#include "daicompilerprovider.h"

#include "../debugarea.h"

#include "gcclikeprovider.h"
#include "msvcprovider.h"

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <project/projectmodel.h>

#include <KPluginFactory>
#include <KAboutData>
#include <KStandardDirs>

using namespace KDevelop;

QHash<QString, QString> DAICompilerProvider::defines( ProjectBaseItem* item ) const
{
    if ( !item ) {
        Q_ASSERT( m_projects.contains( nullptr ) );
        if ( !m_projects[nullptr].compiler ) {
            return {};
        }
        return m_projects[nullptr].compiler->defines( m_projects[nullptr].path );
    }

    auto project = item->project();
    if ( !m_projects.contains( project ) || !m_projects[project].compiler ) {
        return {};
    }
    return m_projects[project].compiler->defines( m_projects[project].path );
}

Path::List DAICompilerProvider::includes( ProjectBaseItem* item ) const
{
    if ( !item ) {
        Q_ASSERT( m_projects.contains( nullptr ) );
        if ( !m_projects[nullptr].compiler ) {
            return {};
        }
        return m_projects[nullptr].compiler->includes( m_projects[nullptr].path );
    }

    auto project = item->project();
    if ( !m_projects.contains( project ) || !m_projects[project].compiler ) {
        return {};
    }
    return m_projects[project].compiler->includes( m_projects[project].path );
}

IDefinesAndIncludesManager::Type DAICompilerProvider::type() const
{
    return IDefinesAndIncludesManager::CompilerSpecific;
}

void DAICompilerProvider::addPoject( IProject* project, Compiler compiler )
{
    m_projects[project] = compiler;
    //cache includes/defines
    if ( m_projects[project].compiler ) {
        m_projects[project].compiler->includes( compiler.path );
        m_projects[project].compiler->defines( compiler.path );
    }
}

void DAICompilerProvider::removePoject( IProject* project )
{
    m_projects.remove( project );
}

DAICompilerProvider::~DAICompilerProvider() {
    IDefinesAndIncludesManager::manager()->unregisterProvider( this );
}

DAICompilerProvider::Compiler DAICompilerProvider::selectCompiler( const QString& compilerName, const QString& path ) const
{
    if ( compilerName.isEmpty() ) {
        for ( auto& compiler : m_providers ) {
            if ( KStandardDirs::findExe( compiler->name() ).isEmpty() ) {
                continue;
            }
            definesAndIncludesDebug() << "Selected compiler: " << compiler->name();
            return {compiler, !path.isEmpty() ? path : compiler->name()};
        }
        kWarning() << "No compiler found. Standard includes/defines won't be provided to the project parser!";
    }else{
        for ( auto it = m_providers.constBegin(); it != m_providers.constEnd(); it++ ) {
            if ( ( *it )->name() == compilerName ) {
                return {*it, !path.isEmpty() ? path : ( *it )->name()};
            }
        }
    }
    return {};
}

bool DAICompilerProvider::setCompiler( KDevelop::IProject* project, const QString& name, const QString& path )
{
    auto compiler = selectCompiler( name, path );

    if ( !compiler.compiler && name != "none" ) {
        definesAndIncludesDebug() << "Invalid compiler: " << name << " " << path;
        return false;
    }

    addPoject( project, compiler );

    return true;
}

void DAICompilerProvider::projectOpened( KDevelop::IProject* project )
{
    definesAndIncludesDebug() << "Adding project: " << project->name();
    auto settings = static_cast<KDevelop::DefinesAndIncludesManager*>( KDevelop::IDefinesAndIncludesManager::manager() );
    auto projectConfig =  project->projectConfiguration().data();

    auto path = settings->pathToCompiler( projectConfig );
    auto name = settings->currentCompiler( projectConfig );
    auto compiler = selectCompiler( name, path );

    if ( compiler.compiler && ( compiler.compiler->name() != name ) ) {
        settings->writeCompiler( projectConfig, compiler.compiler->name() );
    }
    definesAndIncludesDebug() << " compiler is: " << ( compiler.compiler ? compiler.compiler->name() : nullptr );

    addPoject( project, compiler );
}

void DAICompilerProvider::projectClosed( KDevelop::IProject* project )
{
    removePoject( project );
    definesAndIncludesDebug() << "Removed project: " << project->name();
}

K_PLUGIN_FACTORY( CompilerProviderFactory, registerPlugin<DAICompilerProvider>(); )
K_EXPORT_PLUGIN( CompilerProviderFactory( KAboutData( "kdevcompilerprovider",
            "kdevcompilerprovider", ki18n( "Compiler Provider" ), "0.1", ki18n( "" ),
            KAboutData::License_GPL ) ) )

DAICompilerProvider::DAICompilerProvider( QObject* parent, const QVariantList& )
    : IPlugin( CompilerProviderFactory::componentData(), parent )
{
    KDEV_USE_EXTENSION_INTERFACE( IDAICompilerProvider );

    m_providers.append( ProviderPointer( new ClangProvider() ) );
    m_providers.append( ProviderPointer( new GccProvider() ) );
    m_providers.append( ProviderPointer( new MsvcProvider() ) );

    IDefinesAndIncludesManager::manager()->registerProvider( this );

    connect( ICore::self()->projectController(), SIGNAL( projectAboutToBeOpened( KDevelop::IProject* ) ), SLOT( projectOpened( KDevelop::IProject* ) ) );
    connect( ICore::self()->projectController(), SIGNAL( projectClosed( KDevelop::IProject* ) ), SLOT( projectClosed( KDevelop::IProject* ) ) );

    //Add a provider for files without project
    addPoject( nullptr, selectCompiler({}, {}));
}

#include "daicompilerprovider.moc"
