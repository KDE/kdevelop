/*
* This file is part of KDevelop
*
* Copyright 2010 Patrick Spendrin <ps_ml@gmx.de>
* Copyright 2013 Kevin Funk <kevin@kfunk.org>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU Library General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include "setuphelpers.h"

#include <QString>
#include <QStringList>
#include <QProcessEnvironment>
#include <QDir>
#include <QTemporaryFile>

#include <kprocess.h>
#include <kdebug.h>
#include <ktemporaryfile.h>
#include <parser/rpp/chartools.h>
#include <parser/rpp/macrorepository.h>

using namespace KDevelop;

namespace CppTools {

QStringList msvcSetupStandardIncludePaths()
{
    QStringList _includePaths = QProcessEnvironment::systemEnvironment().value("INCLUDE").split(";", QString::SkipEmptyParts);
    QStringList includePaths;
    foreach(const QString& include, _includePaths) {
        includePaths.append(QDir::fromNativeSeparators(include));
    }
    return includePaths;
}

QVector<rpp::pp_macro*> computeMsvcStandardMacros()
{
    QVector<rpp::pp_macro*> ret;
    //Get standard macros from kdevmsvcdefinehelpers
    KProcess proc;
    proc.setOutputChannelMode(KProcess::MergedChannels);
    proc.setTextModeEnabled(true);
    
    // we want to use kdevmsvcdefinehelper as a pseudo compiler backend which
    // returns the defines used in msvc. there is no such thing as -dM with cl.exe
    proc <<"cl.exe" <<"/nologo" <<"/Bxkdevmsvcdefinehelper" <<"empty.cpp";

    // this will fail, so check on that as well
    if (proc.execute(5000) == 2) {
        QString line;
        proc.readLine(); // read the filename

        while (proc.canReadLine()) {
            QByteArray buff = proc.readLine();
            kDebug(9007) << "msvcstandardmacros:" << buff;
            if (!buff.isEmpty()) {
                line = buff;
                if (line.startsWith("#define ")) {
                    line = line.right(line.length() - 8).trimmed();
                    int pos = line.indexOf(' ');
                    
                    ret.append(new rpp::pp_macro);
                    
                    rpp::pp_macro& macro(*ret.back());
                    if (pos != -1) {
                        macro.name = IndexedString( line.left(pos) );
                        macro.setDefinitionText( line.right(line.length() - pos - 1).toUtf8() );
                    } else {
                        macro.name = IndexedString( line );
                    }
                }
            } else {
                kDebug(9007) << "buff is empty!";
            }
        }
    } else {
        kDebug(9007) <<"Unable to read standard c++ macro definitions from cl.exe;";
        while (proc.canReadLine()) kDebug() << proc.readLine();
        kDebug() << proc.exitCode();
    }
    return ret;
}

QVector<rpp::pp_macro*> computeMsvcBuiltinMacros()
{
    QVector<rpp::pp_macro*> macros;

    {
        // MSVC builtin attributes
        macros.append(new rpp::pp_macro("__cdecl"));
        macros.append(new rpp::pp_macro("__fastcall"));
        macros.append(new rpp::pp_macro("__stdcall"));
        macros.append(new rpp::pp_macro("__thiscall"));
    }

    // MSVC builtin types
    // see http://msdn.microsoft.com/en-us/library/cc953fe1.aspx
    {
        rpp::pp_macro* m = new rpp::pp_macro("__int8");
        m->setDefinitionText("char");
        macros.append(m);

        m = new rpp::pp_macro("__int16");
        m->setDefinitionText("short");
        macros.append(m);

        m = new rpp::pp_macro("__int32");
        m->setDefinitionText("int");
        macros.append(m);

        m = new rpp::pp_macro("__int64");
        m->setDefinitionText("long long");
        macros.append(m);

        macros.append(new rpp::pp_macro("__ptr32"));
        macros.append(new rpp::pp_macro("__ptr64"));
    }

    // MSVC specific modifiers
    // see http://msdn.microsoft.com/en-us/library/vstudio/s04b5w00.aspx
    {
        rpp::pp_macro* m = new rpp::pp_macro("__declspec");
        m->function_like = true;
        m->fixed = true;
        m->formalsList().append(IndexedString("param"));
        macros.append(m);

        macros.append(new rpp::pp_macro("__sptr"));
        macros.append(new rpp::pp_macro("__uptr"));
        macros.append(new rpp::pp_macro("__unaligned"));
        macros.append(new rpp::pp_macro("__w64"));
    }

    // MSVC function specifiers
    // see http://msdn.microsoft.com/de-de/library/z8y1yy88.aspx
    {
        macros.append(new rpp::pp_macro("__inline"));
        macros.append(new rpp::pp_macro("__forceinline"));
    }

    return macros;
}

const QVector<rpp::pp_macro*>& msvcStandardMacros()
{
    static const QVector<rpp::pp_macro*> macros =
        computeMsvcStandardMacros() + computeMsvcBuiltinMacros();
    return macros;
}

}

