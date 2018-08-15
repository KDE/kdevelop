/*
 * This file is part of KDevelop
 *
 * Copyright 2010 Patrick Spendrin <ps_ml@gmx.de>
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
                line = buff;
                if ( line.startsWith( QLatin1String("#define ") ) ) {
                    line = line.right( line.length() - 8 ).trimmed();
                    int pos = line.indexOf( ' ' );

                    if ( pos != -1 ) {
                        ret[line.left( pos )] = line.right( line.length() - pos - 1 ).toUtf8();
                    } else {
                        ret[line] = QLatin1String("");
                    }
                }
            }
        }
    } else {
        qCDebug(DEFINESANDINCLUDES) << "Unable to read standard c++ macro definitions from " + path();
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
    QStringList _includePaths = QProcessEnvironment::systemEnvironment().value(QStringLiteral("INCLUDE")).split(QLatin1Char(';'), QString::SkipEmptyParts);
    Path::List includePaths;
    includePaths.reserve(_includePaths.size());
    foreach( const QString &include, _includePaths ) {
        includePaths.append( Path( QDir::fromNativeSeparators( include ) ) );
    }
    return includePaths;
}

MsvcCompiler::MsvcCompiler(const QString& name, const QString& path, bool editable, const QString& factoryName):
    ICompiler(name, path, factoryName, editable)
{}
