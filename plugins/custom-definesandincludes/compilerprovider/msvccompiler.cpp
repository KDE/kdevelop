/*
    SPDX-FileCopyrightText: 2010 Patrick Spendrin <ps_ml@gmx.de>
    SPDX-FileCopyrightText: 2013 Kevin Funk <kfunk@kde.org>
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "msvccompiler.h"

#include <QDir>
#include <QProcessEnvironment>

#include <KProcess>

#include <debug.h>

using namespace KDevelop;

Defines MsvcCompiler::defines(Utils::LanguageType, const QString&) const
{
    Defines ret;
    //Get standard macros from kdevmsvcdefinehelpers
    KProcess proc;
    proc.setOutputChannelMode( KProcess::MergedChannels );
    proc.setTextModeEnabled( true );

    // we want to use kdevmsvcdefinehelper as a pseudo compiler backend which
    // returns the defines used in msvc. there is no such thing as -dM with cl.exe
    proc << path() << QStringLiteral("/nologo") << QStringLiteral("/Bxkdevmsvcdefinehelper") << QStringLiteral("empty.cpp");

    // this will fail, so check on that as well
    if ( proc.execute( 5000 ) == 2 ) {
        QString line;
        proc.readLine(); // read the filename

        while ( proc.canReadLine() ) {
            QByteArray buff = proc.readLine();
            qCDebug(DEFINESANDINCLUDES) << "msvcstandardmacros:" << buff;
            if ( !buff.isEmpty() ) {
                line = QString::fromUtf8(buff);
                if ( line.startsWith( QLatin1String("#define ") ) ) {
                    line = QStringView{line}.sliced(8).trimmed().toString();
                    int pos = line.indexOf(QLatin1Char(' '));

                    if ( pos != -1 ) {
                        ret[line.first(pos)] = line.sliced(pos + 1);
                    } else {
                        ret[line] = QLatin1String("");
                    }
                }
            }
        }
    } else {
        qCDebug(DEFINESANDINCLUDES) << QLatin1String("Unable to read standard c++ macro definitions from ") + path();
        while ( proc.canReadLine() ){
            qCDebug(DEFINESANDINCLUDES)  << proc.readLine();
        }
        qCDebug(DEFINESANDINCLUDES)  << proc.exitCode();
    }

    // MSVC builtin attributes
    {
        ret[QStringLiteral("__cdecl")] = QLatin1String("");
        ret[QStringLiteral("__fastcall")] = QLatin1String("");
        ret[QStringLiteral("__stdcall")] = QLatin1String("");
        ret[QStringLiteral("__thiscall")] = QLatin1String("");
    }

    // MSVC builtin types
    // see http://msdn.microsoft.com/en-us/library/cc953fe1.aspx
    {
        ret[QStringLiteral("__int8")] = QStringLiteral("char");
        ret[QStringLiteral("__int16")] = QStringLiteral("short");
        ret[QStringLiteral("__int32")] = QStringLiteral("int");
        ret[QStringLiteral("__int64")] = QStringLiteral("long long");
        ret[QStringLiteral("__int16")] = QStringLiteral("short");
        ret[QStringLiteral("__ptr32")] = QLatin1String("");
        ret[QStringLiteral("__ptr64")] = QLatin1String("");
    }

    // MSVC specific modifiers
    // see http://msdn.microsoft.com/en-us/library/vstudio/s04b5w00.aspx
    {
        ret[QStringLiteral("__sptr")] = QLatin1String("");
        ret[QStringLiteral("__uptr")] = QLatin1String("");
        ret[QStringLiteral("__unaligned")] = QLatin1String("");
        ret[QStringLiteral("__w64")] = QLatin1String("");
    }

    // MSVC function specifiers
    // see http://msdn.microsoft.com/de-de/library/z8y1yy88.aspx
    {
        ret[QStringLiteral("__inline")] = QLatin1String("");
        ret[QStringLiteral("__forceinline")] = QLatin1String("");
    }

    return ret;
}

Path::List MsvcCompiler::includes(Utils::LanguageType, const QString&) const
{
    const QStringList _includePaths = QProcessEnvironment::systemEnvironment()
                                          .value(QStringLiteral("INCLUDE"))
                                          .split(QLatin1Char(';'), Qt::SkipEmptyParts);
    Path::List includePaths;
    includePaths.reserve(_includePaths.size());
    for (const QString& include : _includePaths) {
        includePaths.append( Path( QDir::fromNativeSeparators( include ) ) );
    }
    return includePaths;
}

MsvcCompiler::MsvcCompiler(const QString& name, const QString& path, bool editable, const QString& factoryName):
    ICompiler(name, path, factoryName, editable)
{}
