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

#include "compilersprovider.h"

#include <language/interfaces/idefinesandincludesmanager.h>

#include <QDir>
#include <QHash>
#include <QProcess>
#include <QRegExp>
#include <QSharedPointer>

using namespace KDevelop;

#ifdef _WIN32
#define NULL_DEVICE "NUL"
#else
#define NULL_DEVICE "/dev/null"
#endif

class BaseProvider : public IDefinesAndIncludesManager::Provider
{
public:
    virtual QHash<QString, QString> defines( ProjectBaseItem* ) const override
    {
        return {};
    }

    virtual Path::List includes( ProjectBaseItem* ) const override
    {
        return {};
    }

    virtual IDefinesAndIncludesManager::Type type() const override
    {
        return IDefinesAndIncludesManager::CompilerSpecific;
    }

protected:
    QString m_compilerName;
};

class GccLikeProvider : public BaseProvider
{
public:
    virtual QHash<QString, QString> defines( ProjectBaseItem* ) const override
    {
        static QHash<QString, QString> definedMacros;
        if ( !definedMacros.isEmpty() ) {
            return definedMacros;
        }

        // #define a 1
        // #define a
        QRegExp defineExpression( "#define\\s+(\\S+)(\\s+(.*))?" );

        QProcess proc;
        proc.setProcessChannelMode( QProcess::MergedChannels );

        proc.start( m_compilerName, {"-std=c++11", "-xc++", "-dM", "-E", NULL_DEVICE} );
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

    virtual Path::List includes( ProjectBaseItem* ) const override
    {
        static Path::List includePaths;

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
        proc.start( m_compilerName, {"-std=c++11", "-xc++", "-E", "-v", NULL_DEVICE} );
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

class ClangProvider : public GccLikeProvider
{
public:
    ClangProvider()
    {
        m_compilerName = "clang++";
    }
};

class GccProvider : public GccLikeProvider
{
public:
    GccProvider()
    {
        m_compilerName = "gcc";
    }
};

class MsvcProvider : public BaseProvider
{
public:
    MsvcProvider()
    {
        m_compilerName = "msvc";
    }

    virtual QHash<QString, QString> defines( ProjectBaseItem* ) const override
    {
        //FIXME:
        return {};
    }

    virtual Path::List includes( ProjectBaseItem* ) const override
    {
        //FIXME:
        return {};
    }
};

class CompilersProvider::CompilersProviderPrivate
{
public:
    CompilersProviderPrivate()
    {
        m_providers["clang"] = QSharedPointer<BaseProvider>( new ClangProvider() );
        m_providers["gcc"] = QSharedPointer<BaseProvider>( new GccProvider() );
        m_providers["msvc"] = QSharedPointer<BaseProvider>( new MsvcProvider() );
    }

    /// Reads config, if compiler already set uses it, otherwise goes through all and checks if it available on the system.

    //FIXME: check if already chosen, if not go through all available and choose an appropriate.
    void selectCompiler() {
        IDefinesAndIncludesManager::manager()->registerProvider( new GccProvider );
    }

    bool setCompiler( const QString& name ) {
        if ( !m_providers.contains( name ) ) {
            return false;
        }

        for ( auto& p : m_providers ) {
            IDefinesAndIncludesManager::manager()->unregisterProvider( p.data() );
        }

        IDefinesAndIncludesManager::manager()->registerProvider( m_providers[name].data() );
        return true;
    }

    QHash<QString, QSharedPointer<BaseProvider> > m_providers;
};

CompilersProvider::CompilersProvider()
    : d( new CompilersProviderPrivate )
{
    d->selectCompiler();
}

bool CompilersProvider::setCompiler( const QString& name )
{
    return d->setCompiler( name );
}

QString CompilersProvider::currentCompiler()
{
    //FIXME:
    return {};
}

