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

#include "../debugarea.h"

QHash<QString, QString> MsvcCompiler::defines( const QString& path ) const
{
    QHash<QString, QString> ret;
    //Get standard macros from kdevmsvcdefinehelpers
    KProcess proc;
    proc.setOutputChannelMode( KProcess::MergedChannels );
    proc.setTextModeEnabled( true );

    // we want to use kdevmsvcdefinehelper as a pseudo compiler backend which
    // returns the defines used in msvc. there is no such thing as -dM with cl.exe
    proc << path << "/nologo" << "/Bxkdevmsvcdefinehelper" << "empty.cpp";

    // this will fail, so check on that as well
    if ( proc.execute( 5000 ) == 2 ) {
        QString line;
        proc.readLine(); // read the filename

        while ( proc.canReadLine() ) {
            QByteArray buff = proc.readLine();
            definesAndIncludesDebug() << "msvcstandardmacros:" << buff;
            if ( !buff.isEmpty() ) {
                line = buff;
                if ( line.startsWith( "#define " ) ) {
                    line = line.right( line.length() - 8 ).trimmed();
                    int pos = line.indexOf( ' ' );

                    if ( pos != -1 ) {
                        ret[line.left( pos )] = line.right( line.length() - pos - 1 ).toUtf8();
                    } else {
                        ret[line] = "";
                    }
                }
            }
        }
    } else {
        definesAndIncludesDebug() << "Unable to read standard c++ macro definitions from " + path;
        while ( proc.canReadLine() ){
            definesAndIncludesDebug()  << proc.readLine();
        }
        definesAndIncludesDebug()  << proc.exitCode();
    }

    // MSVC builtin attributes
    {
        ret["__cdecl"] = "";
        ret["__fastcall"] = "";
        ret["__stdcall"] = "";
        ret["__thiscall"] = "";
    }

    // MSVC builtin types
    // see http://msdn.microsoft.com/en-us/library/cc953fe1.aspx
    {
        ret["__int8"] = "char";
        ret["__int16"] = "short";
        ret["__int32"] = "int";
        ret["__int64"] = "long long";
        ret["__int16"] = "short";
        ret["__ptr32"] = "";
        ret["__ptr64"] = "";
    }

    // MSVC specific modifiers
    // see http://msdn.microsoft.com/en-us/library/vstudio/s04b5w00.aspx
    {
        ret["__sptr"] = "";
        ret["__uptr"] = "";
        ret["__unaligned"] = "";
        ret["__w64"] = "";
    }

    // MSVC function specifiers
    // see http://msdn.microsoft.com/de-de/library/z8y1yy88.aspx
    {
        ret["__inline"] = "";
        ret["__forceinline"] = "";
    }

    return ret;
}

Path::List MsvcCompiler::includes( const QString& ) const
{
    QStringList _includePaths = QProcessEnvironment::systemEnvironment().value( "INCLUDE" ).split( ";", QString::SkipEmptyParts );
    QStringList includePaths;
    foreach( const QString &include, _includePaths ) {
        includePaths.append( QDir::fromNativeSeparators( include ) );
    }
    return KDevelop::toPathList( includePaths );
}

QString MsvcCompiler::name() const
{
    return "MSVC";
}

QString MsvcCompiler::defaultPath() const
{
    return "cl.exe";
}
