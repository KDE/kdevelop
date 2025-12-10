/*
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "gcclikecompiler.h"

#include <debug.h>

#include <interfaces/iruntime.h>
#include <interfaces/iruntimecontroller.h>

#include <QFileInfo>
#include <QProcess>
#include <QRegExp>
#include <QRegularExpression>
#include <QScopeGuard>

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
    // NOTE: we put the greedy .* in front to find the last occurrence
    const QRegularExpression regexp(QStringLiteral(".*(-std=\\S+)"));
    auto result = regexp.match(arguments);
    if (result.hasMatch()) {
        return result.captured(1);
    }

    switch (type) {
        case Utils::C:
        case Utils::ObjC:
            return QStringLiteral("-std=c17");
        case Utils::Cpp:
        case Utils::ObjCpp:
        case Utils::Cuda:
            return QStringLiteral("-std=c++20");
        case Utils::OpenCl:
            return QStringLiteral("-cl-std=CL1.1");
        default:
            Q_UNREACHABLE();
    }
}

}

Defines GccLikeCompiler::defines(Utils::LanguageType type, const QString& arguments) const
{
    // first do a lookup by type and arguments
    auto& data = m_definesIncludes[type][arguments];
    if (data.definedMacros.wasCached) {
        return data.definedMacros.data;
    }

    // TODO: what about -mXXX or -target= flags, some of these change search paths/defines
    const QStringList compilerArguments{
        languageOption(type),
        languageStandard(arguments, type),
        QStringLiteral("-dM"),
        QStringLiteral("-E"),
        QStringLiteral("-"),
    };

    // if that fails, do a lookup based on the actual compiler arguments
    // often these are much less variable than the arguments passed per TU
    // so here we can better exploit the cache by doing this two-phase lookup
    auto& cachedData = m_defines[compilerArguments];
    auto updateDataOnExit = qScopeGuard([&] {
        // we don't want to run the below code more than once
        // even if it errors out
        cachedData.wasCached = true;
        data.definedMacros = cachedData;
    });

    if (cachedData.wasCached) {
        return cachedData.data;
    }

    const auto rt = ICore::self()->runtimeController()->currentRuntime();
    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    proc.setStandardInputFile(QProcess::nullDevice());
    proc.setProgram(path());
    proc.setArguments(compilerArguments);
    rt->startProcess(&proc);

    if ( !proc.waitForStarted( 2000 ) || !proc.waitForFinished( 2000 ) ) {
        qCDebug(DEFINESANDINCLUDES) <<  "Unable to read standard macro definitions from "<< path() << compilerArguments;
        return {};
    }

    if (proc.exitCode() != 0) {
        qCWarning(DEFINESANDINCLUDES) <<  "error while fetching defines for the compiler:" << path() << compilerArguments << proc.readAll();
        return {};
    }

    // #define a 1
    // #define a
    QRegExp defineExpression(QStringLiteral("#define\\s+(\\S+)(?:\\s+(.*)\\s*)?"));

    while ( proc.canReadLine() ) {
        auto line = proc.readLine();

        if ( defineExpression.indexIn(QString::fromUtf8(line)) != -1 ) {
            cachedData.data[defineExpression.cap(1)] = defineExpression.cap(2).trimmed();
        }
    }

    return cachedData.data;
}

Path::List GccLikeCompiler::includes(Utils::LanguageType type, const QString& arguments) const
{
    // first do a lookup by type and arguments
    auto& data = m_definesIncludes[type][arguments];
    if (data.includePaths.wasCached) {
        return data.includePaths.data;
    }

    const QStringList compilerArguments {
        languageOption(type), languageStandard(arguments, type), QStringLiteral("-E"), QStringLiteral("-v"),
        QStringLiteral("-"),
    };

    // if that fails, do a lookup based on the actual compiler arguments
    // often these are much less variable than the arguments passed per TU
    // so here we can better exploit the cache by doing this two-phase lookup
    auto& cachedData = m_includes[compilerArguments];
    auto updateDataOnExit = qScopeGuard([&] {
        // we don't want to run the below code more than once
        // even if it errors out
        cachedData.wasCached = true;
        data.includePaths = cachedData;
    });

    if (cachedData.wasCached) {
        return cachedData.data;
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
    const auto lines = QStringView{output}.split(QLatin1Char('\n'));
    for (const auto line : lines) {
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
                    auto hostPath = rt->pathInHost(Path(QFileInfo(line.trimmed().toString()).canonicalFilePath()));
                    // but skip folders with compiler builtins, we cannot parse these with clang
                    if (!QFile::exists(hostPath.toLocalFile() + QLatin1String("/cpuid.h"))) {
                        cachedData.data << Path(QFileInfo(hostPath.toLocalFile()).canonicalFilePath());
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

    return cachedData.data;
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

#include "moc_gcclikecompiler.cpp"
