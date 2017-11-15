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

#include "gcclikecompiler.h"

#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QMap>
#include <interfaces/iruntime.h>
#include <interfaces/iruntimecontroller.h>

#include <debug.h>

using namespace KDevelop;

namespace
{
// compilers don't deduplicate QStringLiteral
QString minusXC() { return QStringLiteral("-xc"); }
QString minusXCPlusPlus() { return QStringLiteral("-xc++"); }

QStringList languageOptions(const QString& arguments)
{

    // TODO: handle -ansi flag: In C mode, this is equivalent to -std=c90. In C++ mode, it is equivalent to -std=c++98.
    // TODO: check for -x flag on command line
    const QRegularExpression regexp(QStringLiteral("-std=(\\S+)"));
    // see gcc manpage or llvm/tools/clang/include/clang/Frontend/LangStandards.def for list of valid language options
    auto result = regexp.match(arguments);
    if(result.hasMatch()){
        const auto standard = result.captured(0);
        const auto mode = result.capturedRef(1);
        QString language;
        if (mode.startsWith(QLatin1String("c++")) || mode.startsWith(QLatin1String("gnu++"))) {
            language = minusXCPlusPlus();
        } else if (mode.startsWith(QLatin1String("iso9899:"))) {
            // all iso9899:xxxxx modes are C standards
            language = minusXC();
        } else {
            // check for c11, gnu99, etc: all of them have a digit after the c/gnu
            const QRegularExpression cRegexp(QStringLiteral("(c|gnu)\\d.*"));
            if (cRegexp.match(mode).hasMatch()) {
                language = minusXC();
            }
        }
        if (language.isEmpty()) {
            qCWarning(DEFINESANDINCLUDES) << "Failed to determine language from -std= flag:" << arguments;
            language = minusXCPlusPlus();
        }
        return {standard, language};

    }
    // no -std= flag passed -> assume c++11
    return {QStringLiteral("-std=c++11"), minusXCPlusPlus()};
}

}

Defines GccLikeCompiler::defines(const QString& arguments) const
{
    auto& data = m_definesIncludes[arguments];
    if (!data.definedMacros.isEmpty() ) {
        return data.definedMacros;
    }

    // #define a 1
    // #define a
    QRegExp defineExpression( "#define\\s+(\\S+)(?:\\s+(.*)\\s*)?");

    const auto rt = ICore::self()->runtimeController()->currentRuntime();
    QProcess proc;
    proc.setProcessChannelMode( QProcess::MergedChannels );

    // TODO: what about -mXXX or -target= flags, some of these change search paths/defines
    auto compilerArguments = languageOptions(arguments);
    compilerArguments.append(QStringLiteral("-dM"));
    compilerArguments.append(QStringLiteral("-E"));
    compilerArguments.append(QStringLiteral("-"));

    proc.setStandardInputFile(QProcess::nullDevice());
    proc.setProgram(path());
    proc.setArguments(compilerArguments);
    rt->startProcess(&proc);

    if ( !proc.waitForStarted( 2000 ) || !proc.waitForFinished( 2000 ) ) {
        qCDebug(DEFINESANDINCLUDES) <<  "Unable to read standard macro definitions from "<< path();
        return {};
    }

    if (proc.exitCode() != 0) {
        qCWarning(DEFINESANDINCLUDES) <<  "error while fetching defines for the compiler:" << path() << proc.readAll();
        return {};
    }

    while ( proc.canReadLine() ) {
        auto line = proc.readLine();

        if ( defineExpression.indexIn( line ) != -1 ) {
            data.definedMacros[defineExpression.cap( 1 )] = defineExpression.cap( 2 ).trimmed();
        }
    }

    return data.definedMacros;
}

Path::List GccLikeCompiler::includes(const QString& arguments) const
{
    auto& data = m_definesIncludes[arguments];
    if ( !data.includePaths.isEmpty() ) {
        return data.includePaths;
    }

    const auto rt = ICore::self()->runtimeController()->currentRuntime();
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

    auto compilerArguments = languageOptions(arguments);
    compilerArguments.append(QStringLiteral("-E"));
    compilerArguments.append(QStringLiteral("-v"));
    compilerArguments.append(QStringLiteral("-"));

    proc.setStandardInputFile(QProcess::nullDevice());
    proc.setProgram(path());
    proc.setArguments(compilerArguments);
    rt->startProcess(&proc);

    if ( !proc.waitForStarted( 2000 ) || !proc.waitForFinished( 2000 ) ) {
        qCDebug(DEFINESANDINCLUDES) <<  "Unable to read standard include paths from " << path();
        return {};
    }

    if (proc.exitCode() != 0) {
        qCWarning(DEFINESANDINCLUDES) <<  "error while fetching includes for the compiler:" << path() << proc.readAll();
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

    const auto output = QString::fromLocal8Bit( proc.readAllStandardOutput() );
    foreach( const QString &line, output.split( '\n' ) ) {
        switch ( mode ) {
            case Initial:
                if ( line.indexOf( QLatin1String("#include \"...\"") ) != -1 ) {
                    mode = FirstSearch;
                }
                break;
            case FirstSearch:
                if ( line.indexOf( QLatin1String("#include <...>") ) != -1 ) {
                    mode = Includes;
                }
                break;
            case Includes:
                //Detect the include-paths by the first space that is prepended. Reason: The list may contain relative paths like "."
                if ( !line.startsWith( ' ' ) ) {
                    // We've reached the end of the list.
                    mode = Finished;
                } else {
                    // This is an include path, add it to the list.
                    auto hostPath = rt->pathInHost(Path(line.trimmed()));
                    data.includePaths << Path(QFileInfo(hostPath.toLocalFile()).canonicalFilePath());
                }
                break;
            default:
                break;
        }
        if ( mode == Finished ) {
            break;
        }
    }

    return data.includePaths;
}

void GccLikeCompiler::invalidateCache()
{
    m_definesIncludes.clear();
}

GccLikeCompiler::GccLikeCompiler(const QString& name, const QString& path, bool editable, const QString& factoryName):
    ICompiler(name, path, factoryName, editable)
{
    connect(ICore::self()->runtimeController(), &IRuntimeController::currentRuntimeChanged, this, &GccLikeCompiler::invalidateCache);
}
