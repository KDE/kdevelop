/*
 * This file is part of KDevelop
 *
 * Copyright 2013 Kevin Funk <kfunk@kde.org>
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

#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <project/projectmodel.h>

#include <QDir>
#include <QHash>
#include <QProcess>
#include <QRegExp>
#include <QSharedPointer>

#include <KPluginFactory>
#include <KAboutData>
#include <KStandardDirs>

using namespace KDevelop;

#ifdef _WIN32
#define NULL_DEVICE "NUL"
#else
#define NULL_DEVICE "/dev/null"
#endif

class BaseProvider
{
public:
    virtual QHash<QString, QString> defines() {return {}; }
    virtual Path::List includes() {return {}; }

    virtual ~BaseProvider() = default;
    void setPath( const QString& path )
    {
        m_pathToCompiler = path;
    }
protected:
    QString m_pathToCompiler;
    QHash<QString, QString> definedMacros;
    Path::List includePaths;
};

typedef QSharedPointer<BaseProvider> ProviderPointer;

class GccLikeProvider : public BaseProvider
{
public:
    virtual QHash<QString, QString> defines() override
    {
        if ( !definedMacros.isEmpty() ) {
            return definedMacros;
        }

        // #define a 1
        // #define a
        QRegExp defineExpression( "#define\\s+(\\S+)(\\s+(.*))?" );

        QProcess proc;
        proc.setProcessChannelMode( QProcess::MergedChannels );

        proc.start( m_pathToCompiler, {"-std=c++11", "-xc++", "-dM", "-E", NULL_DEVICE} );
        if ( !proc.waitForStarted( 1000 ) || !proc.waitForFinished( 1000 ) ) {
            return {};
        }

        while ( proc.canReadLine() ) {
            auto line = proc.readLine();

            if ( defineExpression.indexIn( line ) != -1 ) {
                definedMacros[defineExpression.cap( 1 )] = defineExpression.cap( 3 );
            }
        }

        return definedMacros;
    }

    virtual Path::List includes() override
    {
        if ( !includePaths.isEmpty() ) {
            return includePaths;
        }

        QProcess proc;
        proc.setProcessChannelMode( QProcess::MergedChannels );

        // The following command will spit out a bunch of information we don't care
        // about before spitting out the include paths.  The parts we care about
        // look like this:
        // #include "..." search starts here:
        // #include <...> search starts here:
        //  /usr/lib/gcc/i486-linux-gnu/4.1.2/../../../../include/c++/4.1.2
        //  /usr/lib/gcc/i486-linux-gnu/4.1.2/../../../../include/c++/4.1.2/i486-linux-gnu
        //  /usr/lib/gcc/i486-linux-gnu/4.1.2/../../../../include/c++/4.1.2/backward
        //  /usr/local/include
        //  /usr/lib/gcc/i486-linux-gnu/4.1.2/include
        //  /usr/include
        // End of search list.
        proc.start( m_pathToCompiler, {"-std=c++11", "-xc++", "-E", "-v", NULL_DEVICE} );
        if ( !proc.waitForStarted( 1000 ) || !proc.waitForFinished( 1000 ) ) {
            return {};
        }

        // We'll use the following constants to know what we're currently parsing.
        enum Status {
            Initial,
            FirstSearch,
            Includes,
            Finished
        };
        Status mode = Initial;

        foreach( const QString &line, QString::fromLocal8Bit( proc.readAllStandardOutput() ).split( '\n' ) ) {
            switch ( mode ) {
                case Initial:
                    if ( line.indexOf( "#include \"...\"" ) != -1 ) {
                        mode = FirstSearch;
                    }
                    break;
                case FirstSearch:
                    if ( line.indexOf( "#include <...>" ) != -1 ) {
                        mode = Includes;
                        break;
                    }
                case Includes:
                    //Detect the include-paths by the first space that is prepended. Reason: The list may contain relative paths like "."
                    if ( !line.startsWith( ' ' ) ) {
                        // We've reached the end of the list.
                        mode = Finished;
                    } else {
                        // This is an include path, add it to the list.
                        includePaths << Path( QDir::cleanPath( line.trimmed() ) );
                    }
                    break;
                default:
                    break;
            }
            if ( mode == Finished ) {
                break;
            }
        }

        return includePaths;
    }
};

class MsvcProvider : public BaseProvider
{
public:
    virtual QHash<QString, QString> defines() override
    {
        //FIXME:
        return {};
    }

    virtual Path::List includes() override
    {
        //FIXME:
        return {};
    }
};

class IADCompilerProvider : public IDefinesAndIncludesManager::Provider
{
public:
    virtual QHash<QString, QString> defines( ProjectBaseItem* item ) const override
    {
        if ( !m_providers.contains( item->project() ) || !m_providers[item->project()] ) {
            return {};
        }
        return m_providers[item->project()]->defines();
    }

    virtual Path::List includes( ProjectBaseItem* item ) const override
    {
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
    }

    void removePoject( IProject* project )
    {
        m_providers.remove( project );
    }

private:
    //list of providers for each projects
    QHash<IProject*, ProviderPointer> m_providers;
};

class CompilerProvider::CompilerProviderPrivate
{
public:
    CompilerProviderPrivate() : m_provider( new IADCompilerProvider )
    {
        IDefinesAndIncludesManager::manager()->registerProvider( m_provider.data() );
    }

    ~CompilerProviderPrivate()
    {
        IDefinesAndIncludesManager::manager()->unregisterProvider( m_provider.data() );
    }

    QString selectCompiler()
    {
        //Note: keep in sync with .kcfg file.
        QStringList compilers = {"clang", "gcc", "msvc"};
        for ( auto& compiler : compilers ) {
            if ( KStandardDirs::findExe( compiler ).isEmpty() ) {
                continue;
            }
            return compiler;
        }

        return "none";
    }

    bool setCompiler( KDevelop::IProject* project, const QString& name, const QString& path )
    {
        ProviderPointer provider;
        if ( name == "gcc" ) {
            provider = ProviderPointer( new GccLikeProvider() );
        }else if ( name == "clang" ) {
            provider = ProviderPointer( new GccLikeProvider() );
        }else if ( name == "msvc" ) {
            provider = ProviderPointer( new MsvcProvider() );
        }else if ( name != "none" || path.isEmpty() ) {
            definesAndIncludesDebug() << "Invalid compiler: " << name << " " << path;
            return false;
        }
        if ( provider ) {
            provider->setPath( path );
        }

        m_provider->addPoject( project, provider );
        return true;
    }

    void projectOpened( KDevelop::IProject* project )
    {
        definesAndIncludesDebug() << "Adding project: " << project->name();
        auto settings = static_cast<KDevelop::DefinesAndIncludesManager*>( KDevelop::IDefinesAndIncludesManager::manager() );
        auto path = settings->pathToCompiler(project->projectConfiguration().data());
        auto compiler = settings->currentCompiler(project->projectConfiguration().data());

        if (compiler.isEmpty()) {
            compiler = selectCompiler();
            settings->writeCompiler(project->projectConfiguration().data(), compiler);
        }
        setCompiler(project, compiler, path);
    }

    void projectClosed( KDevelop::IProject* project )
    {
        m_provider->removePoject(project);
        definesAndIncludesDebug() << "Removed project: " << project->name();
    }

private:
    QScopedPointer<IADCompilerProvider> m_provider;
};

K_PLUGIN_FACTORY( CompilerProviderFactory, registerPlugin<CompilerProvider>(); )
K_EXPORT_PLUGIN( CompilerProviderFactory( KAboutData( "kdevcompilerprovider",
            "kdevcompilerprovider", ki18n( "Compiler Provider" ), "0.1", ki18n( "" ),
            KAboutData::License_GPL ) ) )

CompilerProvider::CompilerProvider( QObject* parent, const QVariantList& )
    : IPlugin( CompilerProviderFactory::componentData(), parent ),
    d( new CompilerProviderPrivate )
{
    KDEV_USE_EXTENSION_INTERFACE( ICompilerProvider );

    connect( ICore::self()->projectController(), SIGNAL( projectAboutToBeOpened( KDevelop::IProject* ) ), SLOT( projectOpened( KDevelop::IProject* ) ) );
    connect( ICore::self()->projectController(), SIGNAL( projectClosed( KDevelop::IProject* ) ), SLOT( projectClosed( KDevelop::IProject* ) ) );

    d->selectCompiler();
}

bool CompilerProvider::setCompiler( KDevelop::IProject* project, const QString& name, const QString& path )
{
    return d->setCompiler( project, name, path );
}

#include "compilerprovider.moc"