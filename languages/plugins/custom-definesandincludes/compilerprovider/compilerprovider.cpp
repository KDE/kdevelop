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
#include "../definesandincludesmanager.h"

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

class IADCompilerProvider : public IDefinesAndIncludesManager::Provider
{
public:
    virtual QHash<QString, QString> defines( ProjectBaseItem* item ) const override
    {
        if ( !item ) {
            if ( !m_providers[nullptr] ) {
                return {};
            }
            return m_providers[nullptr]->defines();
        }

        if ( !m_providers.contains( item->project() ) || !m_providers[item->project()] ) {
            return {};
        }
        return m_providers[item->project()]->defines();
    }

    virtual Path::List includes( ProjectBaseItem* item ) const override
    {
        if ( !item ) {
            if ( !m_providers[nullptr] ) {
                return {};
            }
            return m_providers[nullptr]->includes();
        }

        if ( !m_providers.contains( item->project() ) || !m_providers[item->project()] ) {
            return {};
        }
        return m_providers[item->project()]->includes();
    }

    virtual IDefinesAndIncludesManager::Type type() const override
    {
        return IDefinesAndIncludesManager::CompilerSpecific;
    }

    void addPoject( IProject* project, ProviderPointer provider )
    {
        m_providers[project] = provider;
        //cache includes/defines
        if ( m_providers[project] ) {
            m_providers[project]->includes();
            m_providers[project]->defines();
        }
    }

    void removePoject( IProject* project )
    {
        m_providers.remove( project );
    }

private:
    //list of providers for each projects
    QHash<IProject*, ProviderPointer> m_providers;
};

CompilerProvider::~CompilerProvider() {
    IDefinesAndIncludesManager::manager()->unregisterProvider( m_provider.data() );
}

QString CompilerProvider::selectCompiler() const
{
    //Note: keep in sync with .kcfg file.
    QStringList compilers = {"clang", "gcc", "msvc"};
    for ( auto& compiler : compilers ) {
        if ( KStandardDirs::findExe( compiler ).isEmpty() ) {
            continue;
        }
        definesAndIncludesDebug() << "Selected compiler: " << compiler;
        return compiler;
    }

    return "none";
}

bool CompilerProvider::setCompiler( KDevelop::IProject* project, const QString& name, const QString& path )
{
    ProviderPointer provider;
    if ( name == "gcc" ) {
        provider = ProviderPointer( new GccLikeProvider() );
    }else if ( name == "clang" ) {
        provider = ProviderPointer( new GccLikeProvider() );
    }else if ( name == "msvc" ) {
        provider = ProviderPointer( new MsvcProvider() );
    }else if ( !name.isEmpty() || path.isEmpty() ) {
        definesAndIncludesDebug() << "Invalid compiler: " << name << " " << path;
        return false;
    }
    if ( provider ) {
        provider->setPath( path.trimmed() );
    }

    m_provider->addPoject( project, provider );

    return true;
}

void CompilerProvider::projectOpened( KDevelop::IProject* project )
{
    definesAndIncludesDebug() << "Adding project: " << project->name();
    auto settings = static_cast<KDevelop::DefinesAndIncludesManager*>( KDevelop::IDefinesAndIncludesManager::manager() );
    auto compiler = settings->currentCompiler( project->projectConfiguration().data() );

    definesAndIncludesDebug() << " compiler is: " << compiler;
    if ( compiler.isEmpty() || compiler == "none" ) {
        compiler = selectCompiler();
        settings->writeCompiler( project->projectConfiguration().data(), compiler );
    }
    auto path = settings->pathToCompiler( project->projectConfiguration().data() );
    setCompiler( project, compiler, path );
}

void CompilerProvider::projectClosed( KDevelop::IProject* project )
{
    m_provider->removePoject( project );
    definesAndIncludesDebug() << "Removed project: " << project->name();
}

K_PLUGIN_FACTORY( CompilerProviderFactory, registerPlugin<CompilerProvider>(); )
K_EXPORT_PLUGIN( CompilerProviderFactory( KAboutData( "kdevcompilerprovider",
            "kdevcompilerprovider", ki18n( "Compiler Provider" ), "0.1", ki18n( "" ),
            KAboutData::License_GPL ) ) )

CompilerProvider::CompilerProvider( QObject* parent, const QVariantList& )
    : IPlugin( CompilerProviderFactory::componentData(), parent ),
    m_provider( new IADCompilerProvider )
{
    KDEV_USE_EXTENSION_INTERFACE( ICompilerProvider );

    IDefinesAndIncludesManager::manager()->registerProvider( m_provider.data() );

    connect( ICore::self()->projectController(), SIGNAL( projectAboutToBeOpened( KDevelop::IProject* ) ), SLOT( projectOpened( KDevelop::IProject* ) ) );
    connect( ICore::self()->projectController(), SIGNAL( projectClosed( KDevelop::IProject* ) ), SLOT( projectClosed( KDevelop::IProject* ) ) );

    //Add a provider for files without project
    setCompiler( nullptr, selectCompiler(), selectCompiler() );
}

#include "compilerprovider.moc"