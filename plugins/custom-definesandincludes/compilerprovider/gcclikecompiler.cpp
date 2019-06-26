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

QString languageOption(Utils::LanguageType type)
{
    switch (type) {
        case Utils::C:
            return QStringLiteral("-xc");
        case Utils::Cpp:
            return QStringLiteral("-xc++");
        case Utils::OpenCl:
            return QStringLiteral("-xcl");
        case Utils::Cuda:
            return QStringLiteral("-xcuda");
        case Utils::ObjC:
            return QStringLiteral("-xobjective-c");
        case Utils::ObjCpp:
            return QStringLiteral("-xobjective-c++");
        default:
            Q_UNREACHABLE();
    }
}

QString languageStandard(const QString& arguments, Utils::LanguageType type)
{
    // TODO: handle -ansi flag: In C mode, this is equivalent to -std=c90. In C++ mode, it is equivalent to -std=c++98.
    const QRegularExpression regexp(QStringLiteral("-std=(\\S+)"));
    auto result = regexp.match(arguments);
    if (result.hasMatch())
        return result.captured(0);

    switch (type) {
        case Utils::C:
        case Utils::ObjC:
            return QStringLiteral("-std=c99");
        case Utils::Cpp:
        case Utils::ObjCpp:
        case Utils::Cuda:
            return QStringLiteral("-std=c++11");
        case Utils::OpenCl:
            return QStringLiteral("-cl-std=CL1.1");
        default:
            Q_UNREACHABLE();
    }
}

}

Defines GccLikeCompiler::defines(Utils::LanguageType type, const QString& arguments) const
{
    auto& data = m_definesIncludes[arguments];
    if (!data.definedMacros.isEmpty() ) {
        return data.definedMacros;
    }

    // #define a 1
    // #define a
    QRegExp defineExpression(QStringLiteral("#define\\s+(\\S+)(?:\\s+(.*)\\s*)?"));

    const auto rt = ICore::self()->runtimeController()->currentRuntime();
    QProcess proc;
    proc.setProcessChannelMode( QProcess::MergedChannels );

    // TODO: what about -mXXX or -target= flags, some of these change search paths/defines
    const QStringList compilerArguments{
        languageOption(type),
        languageStandard(arguments, type),
        QStringLiteral("-dM"),
        QStringLiteral("-E"),
        QStringLiteral("-"),
    };
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

        if ( defineExpression.indexIn(QString::fromUtf8(line)) != -1 ) {
            data.definedMacros[defineExpression.cap( 1 )] = defineExpression.cap( 2 ).trimmed();
        }
    }

    return data.definedMacros;
}

Path::List GccLikeCompiler::includes(Utils::LanguageType type, const QString& arguments) const
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

    const QStringList compilerArguments{
        languageOption(type),
        languageStandard(arguments, type),
        QStringLiteral("-E"),
        QStringLiteral("-v"),
        QStringLiteral("-"),
    };

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
    const auto lines = output.splitRef(QLatin1Char('\n'));
    for (const auto& line : lines) {
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
                if (!line.startsWith(QLatin1Char(' '))) {
                    // We've reached the end of the list.
                    mode = Finished;
                } else {
                    // This is an include path, add it to the list.
                    auto hostPath = rt->pathInHost(Path(line.trimmed().toString()));
                    // but skip folders with compiler builtins, we cannot parse these with clang
                    if (!QFile::exists(hostPath.toLocalFile() + QLatin1String("/cpuid.h"))) {
                        data.includePaths << Path(QFileInfo(hostPath.toLocalFile()).canonicalFilePath());
                    }
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
